#pragma once

#include "DX11DepthStencil.h"

class IDX11RenderTarget {
public:
	virtual ~IDX11RenderTarget();

	virtual HRESULT CreateRenderTargetview(ID3D11Device* d3dDev) = 0;
	virtual HRESULT CreateShaderResourceView(ID3D11Device* d3dDev, DXGI_FORMAT srvFmt) = 0;
	virtual HRESULT CreateDepthStencilView(ID3D11Device* d3dDev, DXGI_FORMAT dsvFmt) = 0;
	virtual void CreateDepthStencil(ID3D11Device* d3dDev) = 0;

	virtual ID3D11Texture2D* GetTexture() = 0;
	virtual ID3D11RenderTargetView* GetRenderTargetView() = 0;
	virtual DX11DepthStencil* GetDepthStencil() = 0;
	virtual ID3D11DepthStencilView* GetDepthStencilTargetView() = 0;
	virtual ID3D11ShaderResourceView* GetShaderResourceView() = 0;

	virtual void PSSetShaderResources(ID3D11DeviceContext* ctx, UINT slot) = 0;

	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;

	virtual void SetRenderTarget(ID3D11DeviceContext* ctx) = 0;

	// ·»´õ Å¸°Ù ºä, ½¦ÀÌ´õ ¸®¼Ò½º ºä, µª½º ½ºÅÙ½ÇÀ» ¸ðµÎ »ý¼ºÇÑ´Ù
	static IDX11RenderTarget* Create_GenericRenderTarget(ID3D11Device* d3dDev, DXGI_FORMAT fmt, int width, int height);

	// µª½º ½ºÅÙ½Ç ·»´õ Å¸°ÙÀ» »ý¼ºÇÑ´Ù
	static IDX11RenderTarget* Create_DepthStencilTarget(ID3D11Device* d3dDev, DXGI_FORMAT fmt, DXGI_FORMAT dsvFmt, DXGI_FORMAT srvFmt, int width, int height);
};
