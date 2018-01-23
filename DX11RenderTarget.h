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

	// �⺻ ���� Ÿ���� ���۸� �����Ѵ�
	static IDX11RenderTarget* Create_BackBuffer(
		ID3D11Device* d3dDev, 
		IDXGISwapChain* swapChain);

	// ���� Ÿ�ٰ� ���� ���ٽ��� ��� �����Ѵ�
	static IDX11RenderTarget* Create_GenericRenderTarget(
		ID3D11Device* d3dDev, 
		DXGI_FORMAT fmt, 
		int width, 
		int height);

	// ��Ƽ�� ���� Ÿ���� �����Ѵ�
	static IDX11RenderTarget* Create_GenericRenderTarget(
		ID3D11Device* d3dDev,
		DXGI_FORMAT* formates,
		int formatCount, 
		int width,
		int height);

	// ���� ���� Ÿ���� �����Ѵ� (i.e., �׸��� ������)
	static IDX11RenderTarget* Create_DepthStencilTarget(
		ID3D11Device* d3dDev, 
		DXGI_FORMAT fmt, 
		DXGI_FORMAT dsvFmt, 
		DXGI_FORMAT srvFmt, 
		int width, 
		int height);
};
