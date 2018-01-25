#pragma once

#include <windows.h>
#include <d3d11.h>

#include <memory>
#include <map>

#include "ComPtr.h"

#include "DX11DepthStencil.h"
#include "DX11RenderTarget.h"

struct DepthStencil;

class DX11Device {
public:
	DX11Device(HWND hwnd);

	void SetTexture(int index, ID3D11ShaderResourceView* textureRSView);

	D3D_DRIVER_TYPE                     g_driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL                   g_featureLevel = D3D_FEATURE_LEVEL_11_0;
	ComPtrT<ID3D11Device>               g_pd3dDevice;
	ComPtrT<ID3D11DeviceContext>        immDevCtx;
	ComPtrT<IDXGISwapChain>             g_pSwapChain;

	HRESULT hr;
};

