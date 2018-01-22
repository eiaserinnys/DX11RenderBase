#include "pch.h"
#include "DX11RenderTarget.h"

#include <assert.h>

using namespace std;

class DX11RenderTarget : public IDX11RenderTarget {
public:
	DX11RenderTarget(
		ID3D11Device* d3dDev,
		DXGI_FORMAT fmt,
		int width,
		int height,
		const D3D11_TEXTURE2D_DESC& desc)
		: targetWidth(width), targetHeight(height)
		, format(fmt)
		, texture(NULL)
		, view(NULL)
		, srv(NULL)
		, dsv(NULL)
	{
		HRESULT hr;
		if (FAILED(hr = d3dDev->CreateTexture2D(&desc, NULL, &texture))) { throw hr; }
	}

	~DX11RenderTarget()
	{
		if (srv != NULL) { srv->Release(); srv = NULL; }
		if (view != NULL) { view->Release(); view = NULL; }
		if (dsv != NULL) { dsv->Release(); dsv = NULL; }
		if (texture != NULL) { texture->Release(); texture = NULL; }
		depthStencil.reset(NULL);
	}

	void Clear(
		ID3D11DeviceContext* devCtx, 
		const float* color,
		float depth,
		UINT8 stencil)
	{
		devCtx->ClearRenderTargetView(GetRenderTargetView(), color);
		devCtx->ClearDepthStencilView(GetDepthStencil()->view, D3D11_CLEAR_DEPTH, depth, stencil);
	}

	HRESULT CreateRenderTargetview(ID3D11Device* d3dDev)
	{
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc =
		{
			format,
			D3D11_RTV_DIMENSION_TEXTURE2D,
			0
		};
		return d3dDev->CreateRenderTargetView(texture, &rtvDesc, &view);
	}

	HRESULT CreateShaderResourceView(ID3D11Device* d3dDev, DXGI_FORMAT srvFmt)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC svDesc =
		{
			srvFmt,
			D3D11_SRV_DIMENSION_TEXTURE2D,
			0,
			0
		};
		svDesc.Texture2D.MipLevels = 1;

		return d3dDev->CreateShaderResourceView(texture, &svDesc, &srv);
	}

	HRESULT CreateDepthStencilView(ID3D11Device* d3dDev, DXGI_FORMAT dsvFmt)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC  dsvd =
		{
			dsvFmt, //DXGI_FORMAT_D32_FLOAT,
			D3D11_DSV_DIMENSION_TEXTURE2D,
			0
		};
		return d3dDev->CreateDepthStencilView(texture, &dsvd, &dsv);
	}

	void CreateDepthStencil(ID3D11Device* d3dDev)
	{
		depthStencil.reset(new DX11DepthStencil(d3dDev, targetWidth, targetHeight));
	}

	void SetRenderTarget(ID3D11DeviceContext* ctx)
	{
		ctx->OMSetRenderTargets(1, &view, GetDepthStencil()->view);
	}

	void PSSetShaderResources(ID3D11DeviceContext* ctx, UINT slot)
	{
		ctx->PSSetShaderResources(slot, 1, &srv);
	}

	virtual ID3D11Texture2D* GetTexture() { return texture; }
	ID3D11RenderTargetView* GetRenderTargetView() { return view; }
	ID3D11DepthStencilView* GetDepthStencilTargetView() { return dsv; }
	DX11DepthStencil* GetDepthStencil() { return depthStencil.get(); }
	ID3D11ShaderResourceView* GetShaderResourceView() { return srv; }

	int GetWidth() { return targetWidth; }
	int GetHeight() { return targetHeight; }

	int targetWidth, targetHeight;
	DXGI_FORMAT format;
	ID3D11Texture2D* texture;
	ID3D11RenderTargetView* view;
	ID3D11DepthStencilView* dsv;
	ID3D11ShaderResourceView* srv;

	std::auto_ptr<DX11DepthStencil> depthStencil;
};

IDX11RenderTarget::~IDX11RenderTarget() {}

IDX11RenderTarget* IDX11RenderTarget::Create_GenericRenderTarget(
	ID3D11Device* d3dDev, DXGI_FORMAT fmt, int targetWidth, int targetHeight)
{
	HRESULT hr;

	D3D11_TEXTURE2D_DESC dtd =
	{
		targetWidth,
		targetHeight,
		1,		//UINT MipLevels;
		1,		//UINT ArraySize;
		fmt,	//DXGI_FORMAT Format;
		1,		//DXGI_SAMPLE_DESC SampleDesc;
		0,
		D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
		0,		//UINT CPUAccessFlags;
		0		//UINT MiscFlags;    
	};
	DX11RenderTarget* rt = new DX11RenderTarget(d3dDev, fmt, targetWidth, targetHeight, dtd);

	if (FAILED(hr = rt->CreateRenderTargetview(d3dDev))) { throw hr; }

	if (FAILED(hr = rt->CreateShaderResourceView(d3dDev, fmt))) { throw hr; }

	rt->CreateDepthStencil(d3dDev);

	return rt;
}

IDX11RenderTarget* IDX11RenderTarget::Create_DepthStencilTarget(
	ID3D11Device* d3dDev, DXGI_FORMAT fmt, DXGI_FORMAT dsvFmt, DXGI_FORMAT srvFmt, int targetWidth, int targetHeight)
{
	HRESULT hr;

	D3D11_TEXTURE2D_DESC dtd =
	{
		targetWidth,
		targetHeight,
		1,		//UINT MipLevels;
		1,		//UINT ArraySize;
		fmt,	//DXGI_FORMAT Format;
		1,		//DXGI_SAMPLE_DESC SampleDesc;
		0,
		D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
		D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
		0,		//UINT CPUAccessFlags;
		0		//UINT MiscFlags;    
	};
	DX11RenderTarget* rt = new DX11RenderTarget(d3dDev, fmt, targetWidth, targetHeight, dtd);

	if (FAILED(hr = rt->CreateDepthStencilView(d3dDev, dsvFmt))) { throw hr; }

	if (FAILED(hr = rt->CreateShaderResourceView(d3dDev, srvFmt))) { throw hr; }

	return rt;
}

class DX11BackBuffer : public IDX11RenderTarget {
public:
	DX11BackBuffer(
		ID3D11Device* d3dDev,
		IDXGISwapChain* swapChain)
	{
		{
			HRESULT hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
			assert(pBackBuffer != nullptr);
			if (FAILED(hr)) { return; }
		}

		pBackBuffer->GetDesc(&desc);

		{
			HRESULT hr = d3dDev->CreateRenderTargetView(pBackBuffer, NULL, &rtView);
			assert(rtView != nullptr);
			if (FAILED(hr)) { return; }
		}

		depthStencil.reset(new DX11DepthStencil(d3dDev, desc.Width, desc.Height));
		assert(depthStencil.get() != nullptr);
	}

	~DX11BackBuffer()
	{
		if (rtView != nullptr) { rtView->Release(); rtView = nullptr; }
		if (pBackBuffer != nullptr) { pBackBuffer->Release(); pBackBuffer = nullptr; }
	}

	virtual ID3D11Texture2D* GetTexture() { return pBackBuffer; }

	virtual int GetWidth() { return desc.Width; }
	virtual int GetHeight() { return desc.Height; }

	virtual void SetRenderTarget(ID3D11DeviceContext* ctx)
	{
		ctx->OMSetRenderTargets(1, &rtView, depthStencil->view);
	}

	virtual void Clear(
		ID3D11DeviceContext* devCtx,
		const float* color,
		float depth,
		UINT8 stencil)
	{
		devCtx->ClearRenderTargetView(rtView, color);
		devCtx->ClearDepthStencilView(depthStencil->view, D3D11_CLEAR_DEPTH, depth, stencil);
	}

	ID3D11Texture2D*					pBackBuffer = NULL;
	D3D11_TEXTURE2D_DESC				desc;

	ID3D11RenderTargetView*             rtView = NULL;
	std::unique_ptr<DX11DepthStencil>	depthStencil;
};

IDX11RenderTarget* IDX11RenderTarget::Create_BackBuffer(
	ID3D11Device* d3dDev,
	IDXGISwapChain* swapChain)
{
	return new DX11BackBuffer(d3dDev, swapChain);
}