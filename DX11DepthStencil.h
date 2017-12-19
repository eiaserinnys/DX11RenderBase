#pragma once

struct DX11DepthStencil
{
	int targetWidth, targetHeight;
	ID3D11Texture2D* texture;
	ID3D11DepthStencilView* view;

	DX11DepthStencil(ID3D11Device* d3dDev, int width, int height);
	~DX11DepthStencil();
};