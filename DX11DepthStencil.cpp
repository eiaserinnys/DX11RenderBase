#include "pch.h"
#include "DX11DepthStencil.h"

using namespace std;

DX11DepthStencil::DX11DepthStencil(ID3D11Device* d3dDev, int width, int height)
	: targetWidth(width), targetHeight(height)
	, texture(NULL)
	, view(NULL)
{
	HRESULT hr;

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = targetWidth;
	descDepth.Height = targetHeight;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = d3dDev->CreateTexture2D(&descDepth, NULL, &texture);
	if (FAILED(hr)) { throw hr; }

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = d3dDev->CreateDepthStencilView(texture, &descDSV, &view);
	if (FAILED(hr)) { throw hr; }
}

DX11DepthStencil::~DX11DepthStencil()
{
	if (view != NULL) { view->Release(); view = NULL; }
	if (texture != NULL) { texture->Release(); texture = NULL; }
}

