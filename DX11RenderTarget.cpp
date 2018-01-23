#include "pch.h"
#include "DX11RenderTarget.h"

#include <assert.h>

#include "ComPtr.h"
#include "GenericRenderTarget.h"
#include "DepthStencilTarget.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
IDX11RenderTarget::~IDX11RenderTarget() {}

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

	virtual ID3D11Texture2D* GetTexture(int index) 
	{ 
		assert(index == 0);
		return pBackBuffer; 
	}

	virtual ID3D11ShaderResourceView* GetShaderResourceView(int index) 
	{ 
		assert(index == 0);
		return nullptr; 
	}

	virtual int GetWidth() { return desc.Width; }
	virtual int GetHeight() { return desc.Height; }

	virtual void SetRenderTarget(ID3D11DeviceContext* ctx)
	{
		ID3D11RenderTargetView* views[] = { rtView };
		ctx->OMSetRenderTargets(1, views, depthStencil->view);
	}

	virtual void ClearRenderTarget(
		ID3D11DeviceContext* devCtx,
		int index,
		const float* color)
	{
		assert(index == 0);
		devCtx->ClearRenderTargetView(rtView, color);
	}

	virtual void ClearDepthStencil(
		ID3D11DeviceContext* devCtx,
		float depth,
		UINT8 stencil)
	{
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
{
	return new DX11BackBuffer(d3dDev, swapChain);
}

////////////////////////////////////////////////////////////////////////////////
class DX11GenericRenderTarget : public IDX11RenderTarget {
public:
	DX11GenericRenderTarget(
		ID3D11Device* d3dDev,
		DXGI_FORMAT* formats,
		int formatCount,
		UINT width,
		UINT height)
		: targetWidth(width), targetHeight(height)
	{
		rt.reserve(formatCount);
		for (int i = 0; i < formatCount; ++i)
		{
			rt.push_back(new GenericRenderTarget(d3dDev, formats[i], width, height));
		}
		depthStencil.reset(new DX11DepthStencil(d3dDev, width, height));
	}

	void ClearRenderTarget(
		ID3D11DeviceContext* devCtx,
		int index,
		const float* color)
	{
		assert(index < rt.size());
		devCtx->ClearRenderTargetView(rt[index]->view, color);
	}

	void ClearDepthStencil(
		ID3D11DeviceContext* devCtx,
		float depth,
		UINT8 stencil)
	{
		devCtx->ClearDepthStencilView(depthStencil->view, D3D11_CLEAR_DEPTH, depth, stencil);
	}

	void SetRenderTarget(ID3D11DeviceContext* ctx)
	{ 
		vector<ID3D11RenderTargetView*> views;
		views.reserve(rt.size());

		for (size_t i = 0; i < rt.size(); ++i)
		{
			views.push_back(rt[i]->view);
		}

		ctx->OMSetRenderTargets(views.size(), &views[0], depthStencil->view);
	}

	virtual ID3D11ShaderResourceView* GetShaderResourceView(int index) 
	{ 
		assert(index < rt.size());
		return rt[index]->srv; 
	}

	ID3D11Texture2D* GetTexture(int index) 
	{ 
		assert(index < rt.size());
		return rt[index]->texture; 
	}

	int GetWidth() { return targetWidth; }
	int GetHeight() { return targetHeight; }

	UINT targetWidth, targetHeight;
	vector<GenericRenderTarget*> rt;
	unique_ptr<DX11DepthStencil> depthStencil;
};

IDX11RenderTarget* IDX11RenderTarget::Create_GenericRenderTarget(
	ID3D11Device* d3dDev,
	DXGI_FORMAT fmt,
	int targetWidth,
	int targetHeight)
{
	DXGI_FORMAT formats[] = { fmt };
	return new DX11GenericRenderTarget(d3dDev, formats, 1, targetWidth, targetHeight);
}

IDX11RenderTarget* IDX11RenderTarget::Create_GenericRenderTarget(
	ID3D11Device* d3dDev, 
	DXGI_FORMAT* formats, 
	int formatCount, 
	int targetWidth, 
	int targetHeight)
	{ return new DX11GenericRenderTarget(d3dDev, formats, formatCount, targetWidth, targetHeight); }

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
	{
		dst.reset(new DepthStencilTarget(d3dDev, fmt, dsvFmt, srvFmt, width, height));
	}

	void ClearRenderTarget(
		ID3D11DeviceContext* devCtx, 
		int index, 
		const float* color)
	{
		assert(index == 0);
	}

	void ClearDepthStencil(
		ID3D11DeviceContext* devCtx,
		float depth,
		UINT8 stencil)
	{
		devCtx->ClearDepthStencilView(
			dst->dsv, D3D11_CLEAR_DEPTH, depth, stencil);
	}

	void SetRenderTarget(ID3D11DeviceContext* ctx)
	{
		ID3D11RenderTargetView* views[] = { nullptr };
		ctx->OMSetRenderTargets(1, views, dst->dsv);
	}

	virtual ID3D11ShaderResourceView* GetShaderResourceView(int index)
	{ 
		assert(index == 0);
		return dst->srv; 
	}

	virtual ID3D11Texture2D* GetTexture(int index) 
	{ 
		assert(index == 0);
		return dst->texture;
	}

	int GetWidth() { return dst->targetWidth; }
	int GetHeight() { return dst->targetHeight; }

	unique_ptr<DepthStencilTarget> dst;
};

IDX11RenderTarget* IDX11RenderTarget::Create_DepthStencilTarget(
	ID3D11Device* d3dDev, 
	DXGI_FORMAT fmt, 
	DXGI_FORMAT dsvFmt, 
	DXGI_FORMAT srvFmt, 
	int targetWidth, 
	int targetHeight)
	{ return new DX11DepthStencilTarget(d3dDev, fmt, dsvFmt, srvFmt, targetWidth, targetHeight); }
