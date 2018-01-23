#pragma once

struct RenderTargetHelper
{
	static HRESULT CreateRenderTargetview(
		ID3D11Device* d3dDev,
		ID3D11Texture2D* texture,
		DXGI_FORMAT fmt,
		ID3D11RenderTargetView** view);

	static HRESULT CreateShaderResourceView(
		ID3D11Device* d3dDev,
		ID3D11Texture2D* texture,
		DXGI_FORMAT srvFmt,
		ID3D11ShaderResourceView** srv);

	static HRESULT CreateDepthStencilView(
		ID3D11Device* d3dDev,
		ID3D11Texture2D* texture,
		DXGI_FORMAT dsvFmt,
		ID3D11DepthStencilView** dsv);
};
