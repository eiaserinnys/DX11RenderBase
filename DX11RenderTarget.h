#pragma once

#include "DX11DepthStencil.h"

class IDX11RenderTarget {
public:
	virtual ~IDX11RenderTarget();

	virtual ID3D11Texture2D* GetTexture(int index) = 0;

	virtual ID3D11ShaderResourceView* GetShaderResourceView(int index) = 0;

	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;

	virtual void SetRenderTarget(ID3D11DeviceContext* ctx) = 0;

	virtual void Clear(
		ID3D11DeviceContext* devCtx,
		const float* color,
		float depth,
		UINT8 stencil)
	{
		ClearRenderTarget(devCtx, 0, color);
		ClearDepthStencil(devCtx, depth, stencil);
	}

	virtual void ClearRenderTarget(
		ID3D11DeviceContext* devCtx,
		int index, 
		const float* color) = 0;

	virtual void ClearDepthStencil(
		ID3D11DeviceContext* devCtx,
		float depth,
		UINT8 stencil) = 0;

	// ±âº» ·»´õ Å¸°ÙÀÇ ·¡ÆÛ¸¦ »ý¼ºÇÑ´Ù
	static IDX11RenderTarget* Create_BackBuffer(
		ID3D11Device* d3dDev, 
		IDXGISwapChain* swapChain);

	// ·»´õ Å¸°Ù°ú µª½º ½ºÅÙ½ÇÀ» ¸ðµÎ »ý¼ºÇÑ´Ù
	static IDX11RenderTarget* Create_GenericRenderTarget(
		ID3D11Device* d3dDev, 
		DXGI_FORMAT fmt, 
		int width, 
		int height);

	// ¸ÖÆ¼ÇÃ ·»´õ Å¸°ÙÀ» »ý¼ºÇÑ´Ù
	static IDX11RenderTarget* Create_GenericRenderTarget(
		ID3D11Device* d3dDev,
		DXGI_FORMAT* formates,
		int formatCount, 
		int width,
		int height);

	// µª½º ·»´õ Å¸°ÙÀ» »ý¼ºÇÑ´Ù (i.e., ±×¸²ÀÚ ·»´õ¸µ)
	static IDX11RenderTarget* Create_DepthStencilTarget(
		ID3D11Device* d3dDev, 
		DXGI_FORMAT fmt, 
		DXGI_FORMAT dsvFmt, 
		DXGI_FORMAT srvFmt, 
		int width, 
		int height);
};
