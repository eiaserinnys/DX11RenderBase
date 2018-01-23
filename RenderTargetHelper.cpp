#include "pch.h"
#include "RenderTargetHelper.h"

//------------------------------------------------------------------------------
HRESULT RenderTargetHelper::CreateRenderTargetview(
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

//------------------------------------------------------------------------------
HRESULT RenderTargetHelper::CreateShaderResourceView(
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

//------------------------------------------------------------------------------
HRESULT RenderTargetHelper::CreateDepthStencilView(
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
