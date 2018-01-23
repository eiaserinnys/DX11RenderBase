#include "pch.h"
#include "DX11RenderTarget.h"

#include <assert.h>

#include "ComPtr.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
struct RenderTargetHelper
{
	static HRESULT CreateRenderTargetview(
		ID3D11Device* d3dDev,
		ID3D11Texture2D* texture, 
		DXGI_FORMAT fmt, 
		ID3D11RenderTargetView** view)
	{
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = 
		{ 
			fmt, 
			D3D11_RTV_DIMENSION_TEXTURE2D, 
			0 
		};
		return d3dDev->CreateRenderTargetView(texture, &rtvDesc, view);
	}

	static HRESULT CreateShaderResourceView(
		ID3D11Device* d3dDev, 
		ID3D11Texture2D* texture,
		DXGI_FORMAT srvFmt,
		ID3D11ShaderResourceView** srv)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC svDesc =
		{ 
			srvFmt, 
			D3D11_SRV_DIMENSION_TEXTURE2D, 
			0, 
			0 
		};
		svDesc.Texture2D.MipLevels = 1;

		return d3dDev->CreateShaderResourceView(texture, &svDesc, srv);
	}

	static HRESULT CreateDepthStencilView(
		ID3D11Device* d3dDev,
		ID3D11Texture2D* texture,
		DXGI_FORMAT dsvFmt,
		ID3D11DepthStencilView** dsv)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC  dsvd =
		{
			dsvFmt,
			D3D11_DSV_DIMENSION_TEXTURE2D,
			0
		};
		return d3dDev->CreateDepthStencilView(texture, &dsvd, dsv);
	}
};

////////////////////////////////////////////////////////////////////////////////
IDX11RenderTarget::~IDX11RenderTarget() {}

////////////////////////////////////////////////////////////////////////////////
class DX11GenericRenderTarget : public IDX11RenderTarget {
public:
	DX11GenericRenderTarget(
		ID3D11Device* d3dDev,
		DXGI_FORMAT fmt,
		UINT width,
		UINT height)
		: targetWidth(width), targetHeight(height)
		, format(fmt)
	{
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

		HRESULT hr;

		if (FAILED(hr = d3dDev->CreateTexture2D(&dtd, nullptr, &texture))) { throw hr; }

		if (FAILED(hr = RenderTargetHelper::CreateRenderTargetview(d3dDev, texture, format, &view)))  { throw hr; }

		if (FAILED(hr = RenderTargetHelper::CreateShaderResourceView(d3dDev, texture, fmt, &srv))) { throw hr; }

		depthStencil.reset(new DX11DepthStencil(d3dDev, width, height));
	}

	void Clear(ID3D11DeviceContext* devCtx, const float* color, float depth, UINT8 stencil)
	{
		devCtx->ClearRenderTargetView(view, color);
		devCtx->ClearDepthStencilView(depthStencil->view, D3D11_CLEAR_DEPTH, depth, stencil);
	}

	void SetRenderTarget(ID3D11DeviceContext* ctx)
	{ 
		ID3D11RenderTargetView* views[] = { view };
		ctx->OMSetRenderTargets(1, views, depthStencil->view);
	}

	void PSSetShaderResources(ID3D11DeviceContext* ctx, UINT slot)
	{ 
		ID3D11ShaderResourceView* views[] = { srv };
		ctx->PSSetShaderResources(slot, 1, views); 
	}

	ID3D11Texture2D* GetTexture() { return texture; }

	int GetWidth() { return targetWidth; }
	int GetHeight() { return targetHeight; }

	UINT targetWidth, targetHeight;
	DXGI_FORMAT format;
	ComPtrT<ID3D11Texture2D> texture;
	ComPtrT<ID3D11RenderTargetView> view;
	ComPtrT<ID3D11ShaderResourceView> srv;
	unique_ptr<DX11DepthStencil> depthStencil;
};

IDX11RenderTarget* IDX11RenderTarget::Create_GenericRenderTarget(
	ID3D11Device* d3dDev, DXGI_FORMAT fmt, int targetWidth, int targetHeight)
	{ return new DX11GenericRenderTarget(d3dDev, fmt, targetWidth, targetHeight); }

////////////////////////////////////////////////////////////////////////////////
class DX11DepthStencilTarget : public IDX11RenderTarget {
public:
	DX11DepthStencilTarget(
		ID3D11Device* d3dDev,
		DXGI_FORMAT fmt,		// DXGI_FORMAT_R32_TYPELESS
		DXGI_FORMAT dsvFmt,		// DXGI_FORMAT_D32_FLOAT
		DXGI_FORMAT srvFmt,		// DXGI_FORMAT_R32_FLOAT
		int width,
		int height)
		: targetWidth(width), targetHeight(height)
		, format(fmt)
	{
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

		HRESULT hr;

		if (FAILED(hr = d3dDev->CreateTexture2D(&dtd, nullptr, &texture))) { throw hr; }

		if (FAILED(hr = RenderTargetHelper::CreateDepthStencilView(d3dDev, texture, dsvFmt, &dsv))) { throw hr; }

		if (FAILED(hr = RenderTargetHelper::CreateShaderResourceView(d3dDev, texture, srvFmt, &srv))) { throw hr; }
	}

	void Clear(ID3D11DeviceContext* devCtx, const float* color, float depth, UINT8 stencil)
	{
		devCtx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, depth, stencil);
	}

	void SetRenderTarget(ID3D11DeviceContext* ctx)
	{
		ID3D11RenderTargetView* views[] = { nullptr };
		ctx->OMSetRenderTargets(1, views, dsv);
	}

	void PSSetShaderResources(ID3D11DeviceContext* ctx, UINT slot)
	{
		ctx->PSSetShaderResources(slot, 1, &srv);
	}

	virtual ID3D11Texture2D* GetTexture() { return texture; }

	int GetWidth() { return targetWidth; }
	int GetHeight() { return targetHeight; }

	int targetWidth, targetHeight;
	DXGI_FORMAT format;
	ComPtrT<ID3D11Texture2D> texture;
	ComPtrT<ID3D11DepthStencilView> dsv;
	ComPtrT<ID3D11ShaderResourceView> srv;
};

IDX11RenderTarget* IDX11RenderTarget::Create_DepthStencilTarget(
	ID3D11Device* d3dDev, 
	DXGI_FORMAT fmt, 
	DXGI_FORMAT dsvFmt, 
	DXGI_FORMAT srvFmt, 
	int targetWidth, 
	int targetHeight)
	{ return new DX11DepthStencilTarget(d3dDev, fmt, dsvFmt, srvFmt, targetWidth, targetHeight); }

////////////////////////////////////////////////////////////////////////////////
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
			HRESULT hr = d3dDev->CreateRenderTargetView(pBackBuffer, nullptr, &rtView);
			assert(rtView != nullptr);
			if (FAILED(hr)) { return; }
		}

		depthStencil.reset(new DX11DepthStencil(d3dDev, desc.Width, desc.Height));
		assert(depthStencil.get() != nullptr);
	}

	virtual ID3D11Texture2D* GetTexture() { return pBackBuffer; }

	virtual int GetWidth() { return desc.Width; }
	virtual int GetHeight() { return desc.Height; }

	virtual void SetRenderTarget(ID3D11DeviceContext* ctx)
	{
		ID3D11RenderTargetView* views[] = { rtView };
		ctx->OMSetRenderTargets(1, views, depthStencil->view);
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

	ComPtrT<ID3D11Texture2D>			pBackBuffer;
	D3D11_TEXTURE2D_DESC				desc;
	ComPtrT<ID3D11RenderTargetView>		rtView;
	unique_ptr<DX11DepthStencil>		depthStencil;
};

IDX11RenderTarget* IDX11RenderTarget::Create_BackBuffer(
	ID3D11Device* d3dDev,
	IDXGISwapChain* swapChain)
{ return new DX11BackBuffer(d3dDev, swapChain); }