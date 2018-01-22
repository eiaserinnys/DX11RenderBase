#pragma once

#include "DX11DepthStencil.h"

class IDX11RenderTarget {
public:
	virtual ~IDX11RenderTarget();

	virtual ID3D11Texture2D* GetTexture() = 0;

	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;

	virtual void SetRenderTarget(ID3D11DeviceContext* ctx) = 0;

	virtual void Clear(
		ID3D11DeviceContext* devCtx, 
		const float* color, 
		float depth, 
		UINT8 stencil) = 0;

	// ±âº» ·»´õ Å¸°ÙÀÇ ·¡ÆÛ¸¦ »ý¼ºÇÑ´Ù
	static IDX11RenderTarget* Create_BackBuffer(ID3D11Device* d3dDev, IDXGISwapChain* swapChain);

	// ·»´õ Å¸°Ù ºä, ½¦ÀÌ´õ ¸®¼Ò½º ºä, µª½º ½ºÅÙ½ÇÀ» ¸ðµÎ »ý¼ºÇÑ´Ù
	static IDX11RenderTarget* Create_GenericRenderTarget(ID3D11Device* d3dDev, DXGI_FORMAT fmt, int width, int height);

	// µª½º ½ºÅÙ½Ç ·»´õ Å¸°ÙÀ» »ý¼ºÇÑ´Ù
	static IDX11RenderTarget* Create_DepthStencilTarget(ID3D11Device* d3dDev, DXGI_FORMAT fmt, DXGI_FORMAT dsvFmt, DXGI_FORMAT srvFmt, int width, int height);
};
