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

	// Create a render target view
	backBuffer.reset(IDX11RenderTarget::Create_BackBuffer(g_pd3dDevice, g_pSwapChain));

	// 스크린샷용 렌더 타겟을 만들어둔다
	unwrapRT.reset(IDX11RenderTarget::Create_GenericRenderTarget(
		g_pd3dDevice, DXGI_FORMAT_B8G8R8A8_UNORM, unwrapWidth, unwrapHeight));
	assert(unwrapRT.get() != nullptr);

	upsampleRT.reset(IDX11RenderTarget::Create_GenericRenderTarget(
		g_pd3dDevice, DXGI_FORMAT_R32_FLOAT, upsampleWidth, upsampleHeight));
	assert(upsampleRT.get() != nullptr);

	wlsRT.reset(IDX11RenderTarget::Create_GenericRenderTarget(
		g_pd3dDevice, DXGI_FORMAT_R32_FLOAT, wlsWidth, wlsHeight));
	assert(wlsRT.get() != nullptr);

	RestoreRenderTarget();

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
void DX11Device::SetScreenshotMode(RenderTarget::Type rtType_)
{
	this->rtType = rtType_;
}

//------------------------------------------------------------------------------
void DX11Device::RestoreRenderTarget()
{
	D3D11_VIEWPORT vp;

	switch (rtType) {
	case RenderTarget::ForUnwrap:
		unwrapRT->SetRenderTarget(immDevCtx);
		vp.Width = (FLOAT)unwrapRT->GetWidth();
		vp.Height = (FLOAT)unwrapRT->GetHeight();
		break;

	case RenderTarget::ForUpsample:
		upsampleRT->SetRenderTarget(immDevCtx);
		vp.Width = (FLOAT)upsampleRT->GetWidth();
		vp.Height = (FLOAT)upsampleRT->GetHeight();
		break;

	case RenderTarget::ForWls:
		wlsRT->SetRenderTarget(immDevCtx);
		vp.Width = (FLOAT)wlsRT->GetWidth();
		vp.Height = (FLOAT)wlsRT->GetHeight();
		break;

	default:
		backBuffer->SetRenderTarget(immDevCtx);
		vp.Width = (FLOAT)backBuffer->GetWidth();
		vp.Height = (FLOAT)backBuffer->GetHeight();
		break;
	}

	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	immDevCtx->RSSetViewports(1, &vp);
}

//------------------------------------------------------------------------------
void DX11Device::ClearRenderTarget()
{
	//float ClearColor[4] = { 0.25f, 0.25f, 0.25f, 0.0f }; // red, green, blue, alpha
	float ClearColor[4] = { 1, 1, 1, 0.0f }; // red, green, blue, alpha

	switch (rtType) {
	case RenderTarget::ForUnwrap:
		unwrapRT->Clear(immDevCtx, ClearColor, 1.0f, 0);
		break;

	case RenderTarget::ForUpsample:
		upsampleRT->Clear(immDevCtx, ClearColor, 1.0f, 0);
		break;

	case RenderTarget::ForWls:
		wlsRT->Clear(immDevCtx, ClearColor, 1.0f, 0);
		break;

	default:
		backBuffer->Clear(immDevCtx, ClearColor, 1.0f, 0);
		break;
	}
}

//------------------------------------------------------------------------------
IDX11RenderTarget* DX11Device::GetRenderTarget()
{
	switch (rtType) {
	case RenderTarget::ForUnwrap:	return unwrapRT.get();
	case RenderTarget::ForUpsample:	return upsampleRT.get();
	case RenderTarget::ForWls:		return wlsRT.get();
	}
	return backBuffer.get();
}

//------------------------------------------------------------------------------
int DX11Device::GetRenderTargetWidth()
{
	switch (rtType) {
	case RenderTarget::ForUnwrap:	return unwrapRT->GetWidth();
	case RenderTarget::ForUpsample:	return upsampleRT->GetWidth();
	case RenderTarget::ForWls:		return wlsRT->GetWidth();
	default:						return backBuffer->GetWidth();
	}
}

//------------------------------------------------------------------------------
int DX11Device::GetRenderTargetHeight()
{
	switch (rtType) {
	case RenderTarget::ForUnwrap:	return unwrapRT->GetHeight();
	case RenderTarget::ForUpsample:	return upsampleRT->GetHeight();
	case RenderTarget::ForWls:		return wlsRT->GetHeight();
	default:						return backBuffer->GetHeight();
	}
}

//------------------------------------------------------------------------------
DX11Device::~DX11Device()
{
	unwrapRT.reset(nullptr);
	upsampleRT.reset(nullptr);
	wlsRT.reset(nullptr);
	backBuffer.reset(nullptr);

	if (g_pSwapChain) g_pSwapChain->Release();
	if (immDevCtx) immDevCtx->Release();
	if (g_pd3dDevice) g_pd3dDevice->Release();
}

