#pragma once

#include "ComPtr.h"

struct DX11DepthStencil
{
	int targetWidth, targetHeight;
	ComPtrT<ID3D11Texture2D> texture;
	ComPtrT<ID3D11DepthStencilView> view;

	DX11DepthStencil(ID3D11Device* d3dDev, int width, int height);
};