#include "pch.h"
#include "DX11Device.h"

#include <assert.h>

using namespace std;

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
DX11Device::DX11Device(HWND hwnd)
{
	hr = S_OK;

	RECT rc;
	int width, height;
	{
		GetClientRect(hwnd, &rc);
		width = rc.right - rc.left;
		height = rc.bottom - rc.top;
	}

	UINT createDeviceFlags =
		// 동영상 텍스처링을 위해서 추가
		//D3D11_CREATE_DEVICE_VIDEO_SUPPORT |
		D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	{
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hwnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;
	}

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(
			nullptr,
			g_driverType,
			nullptr,
			createDeviceFlags,
			featureLevels,
			numFeatureLevels,
			D3D11_SDK_VERSION,
			&sd,
			&g_pSwapChain,
			&g_pd3dDevice,
			&g_featureLevel,
			&immDevCtx);
		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr)) { return; }

	// 미디어 플레이어가 멀티 스레드 접근하기 때문에???
#if 0
	{
		ID3D10Multithread* spMultithread;
		if (FAILED(g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&spMultithread))))
		{
			return;
		}
		spMultithread->SetMultithreadProtected(TRUE);
		spMultithread->Release();
	}
#endif

	hr = S_OK;
}

//------------------------------------------------------------------------------
void DX11Device::SetTexture(
	int index, 
	ID3D11ShaderResourceView* textureRSView)
{
	immDevCtx->PSSetShaderResources(index, 1, &textureRSView);
}

//------------------------------------------------------------------------------
DX11Device::~DX11Device()
{
	if (g_pSwapChain) g_pSwapChain->Release();
	if (immDevCtx) immDevCtx->Release();
	if (g_pd3dDevice) g_pd3dDevice->Release();
}

