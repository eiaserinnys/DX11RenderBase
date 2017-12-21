#include "pch.h"

#include <DDSTextureLoader.h>
#include <DirectXTex.h>

#include "DX11Device.h"

using namespace std;

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
DX11Device::DX11Device(HWND hwnd)
{
	hr = S_OK;

	RECT rc;
	GetClientRect(hwnd, &rc);
	width = rc.right - rc.left;
	height = rc.bottom - rc.top;

	UINT createDeviceFlags =
		// 동영상 텍스처링을 위해서 추가
		D3D11_CREATE_DEVICE_VIDEO_SUPPORT |
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

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(
			NULL,
			g_driverType,
			NULL,
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
	ID3D11Texture2D* pBackBuffer = NULL;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr)) { return; }

	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView);
	pBackBuffer->Release();
	assert(g_pRenderTargetView != nullptr);

	if (FAILED(hr)) { return; }

	depthStencil.reset(new DX11DepthStencil(g_pd3dDevice, width, height));
	assert(depthStencil.get() != nullptr);

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

wstring ToUnicode(const string& str)
{
	const size_t cSize = str.length() + 1;
	wstring wc(cSize, L'#');
	mbstowcs(&wc[0], str.c_str(), cSize);
	return wc;
}

void DX11Device::UnloadTexture(const string& fileName)
{
	auto tit = textures.find(fileName);
	if (tit != textures.end())
	{
		tit->second.first->Release();
		tit->second.second->Release();
		textures.erase(tit);
	}
}

pair<ID3D11Resource*, ID3D11ShaderResourceView*>
DX11Device::CreateVideoTexture(
	const string& name,
	int width,
	int height)
{
	UnloadTexture(name);

	ID3D11Texture2D* texture = nullptr;
	ID3D11ShaderResourceView* view = nullptr;

	if (SUCCEEDED(g_pd3dDevice->CreateTexture2D(
		&CD3D11_TEXTURE2D_DESC(
			DXGI_FORMAT_B8G8R8A8_UNORM,
			width,
			height,
			1,
			1,
			D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET),
		nullptr,
		&texture)))
	{
#if 0
		ID3D11Resource* resource;

		if (SUCCEEDED(texture->QueryInterface(
			IID_ID3D11Resource,
			(void**)&resource)))
		{
			hr = g_pd3dDevice->CreateShaderResourceView(resource, nullptr, &view);
			if (SUCCEEDED(hr))
			{
				auto ret = make_pair(resource, view);
				textures.insert(make_pair(name, ret));
				return ret;
			}
		}

		texture->Release();
#endif
	}

	return make_pair(nullptr, nullptr);
}

pair<ID3D11Resource*, ID3D11ShaderResourceView*>
DX11Device::GetTexture(const string& fileName)
{
	auto tit = textures.find(fileName);
	if (tit != textures.end())
	{
		return tit->second;
	}
	else
	{
		auto res = LoadTextureInternal(fileName);
		textures.insert(make_pair(fileName, res));
		return res;
	}
}

pair<ID3D11Resource*, ID3D11ShaderResourceView*>
DX11Device::LoadTextureInternal(const std::string& fileName)
{
	ID3D11Resource* texture;
	ID3D11ShaderResourceView* view;

	string extName;
	{
		string::size_type idx;

		idx = fileName.rfind('.');

		if (idx != std::string::npos)
		{
			extName = fileName.substr(idx + 1);
		}
		else
		{
			// No extension found
		}
	}

	if (_stricmp(extName.c_str(), "tga") == 0)
	{
		std::wstring ws(fileName.size(), L' '); // Overestimate number of code points.
		ws.resize(mbstowcs(&ws[0], fileName.c_str(), fileName.size())); // Shrink to fit.

		DirectX::ScratchImage iimage;
		if (SUCCEEDED(DirectX::LoadFromTGAFile(ws.c_str(), nullptr, iimage)))
		{
			if (SUCCEEDED(DirectX::CreateTexture(
				g_pd3dDevice,
				iimage.GetImages(),
				iimage.GetImageCount(),
				iimage.GetMetadata(),
				&texture)))
			{
				if (SUCCEEDED(g_pd3dDevice->CreateShaderResourceView(
					texture, nullptr, &view)))
				{
					return make_pair(texture, view);
				}

				// 여기에 올 수가 있나...싶지만?
				texture->Release();
			}
		}
	}
	else
	{
		if (SUCCEEDED(DirectX::CreateDDSTextureFromFile(
			g_pd3dDevice, ToUnicode(fileName).c_str(), &texture, &view)))
		{
			return make_pair(texture, view);
		}
	}

	return make_pair(nullptr, nullptr);
}

void DX11Device::ReloadTexture()
{
	for (auto tit = textures.begin(); tit != textures.end(); ++tit)
	{
		if (tit->second.first != nullptr) { tit->second.first->Release(); }
		tit->second.first = NULL;

		if (tit->second.second != nullptr) { tit->second.second->Release(); }
		tit->second.second = NULL;

		tit->second = LoadTextureInternal(tit->first);
	}
}

void DX11Device::SetTexture(int index, const string& fileName)
{
	auto res = GetTexture(fileName);
	immDevCtx->PSSetShaderResources(index, 1, &res.second);
}

void DX11Device::SetScreenshotMode(RenderTarget::Type rtType_)
{
	this->rtType = rtType_;
}

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
		immDevCtx->OMSetRenderTargets(1, &g_pRenderTargetView, depthStencil->view);
		vp.Width = (FLOAT)width;
		vp.Height = (FLOAT)height;
		break;
	}

	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	immDevCtx->RSSetViewports(1, &vp);
}

void DX11Device::ClearRenderTarget()
{
	//float ClearColor[4] = { 0.25f, 0.25f, 0.25f, 0.0f }; // red, green, blue, alpha
	float ClearColor[4] = { 1, 1, 1, 0.0f }; // red, green, blue, alpha

	switch (rtType) {
	case RenderTarget::ForUnwrap:
		immDevCtx->ClearRenderTargetView(unwrapRT->GetRenderTargetView(), ClearColor);
		immDevCtx->ClearDepthStencilView(unwrapRT->GetDepthStencil()->view, D3D11_CLEAR_DEPTH, 1.0f, 0);
		break;

	case RenderTarget::ForUpsample:
	{
		float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // red, green, blue, alpha
		immDevCtx->ClearRenderTargetView(upsampleRT->GetRenderTargetView(), ClearColor);
		immDevCtx->ClearDepthStencilView(upsampleRT->GetDepthStencil()->view, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	break;

	case RenderTarget::ForWls:
	{
		float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // red, green, blue, alpha
		immDevCtx->ClearRenderTargetView(wlsRT->GetRenderTargetView(), ClearColor);
		immDevCtx->ClearDepthStencilView(wlsRT->GetDepthStencil()->view, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	break;

	default:
		immDevCtx->ClearRenderTargetView(g_pRenderTargetView, ClearColor);
		immDevCtx->ClearDepthStencilView(depthStencil->view, D3D11_CLEAR_DEPTH, 1.0f, 0);
		break;
	}
}

IDX11RenderTarget* DX11Device::GetRenderTarget()
{
	switch (rtType) {
	case RenderTarget::ForUnwrap:	return unwrapRT.get();
	case RenderTarget::ForUpsample:	return upsampleRT.get();
	case RenderTarget::ForWls:		return wlsRT.get();
	}
	return NULL;
}

int DX11Device::GetRenderTargetWidth()
{
	switch (rtType) {
	case RenderTarget::ForUnwrap:	return unwrapRT->GetWidth();
	case RenderTarget::ForUpsample:	return upsampleRT->GetWidth();
	case RenderTarget::ForWls:		return wlsRT->GetWidth();
	default:						return width;
	}
}

int DX11Device::GetRenderTargetHeight()
{
	switch (rtType) {
	case RenderTarget::ForUnwrap:	return unwrapRT->GetHeight();
	case RenderTarget::ForUpsample:	return upsampleRT->GetHeight();
	case RenderTarget::ForWls:		return wlsRT->GetHeight();
	default:						return height;
	}
}

DX11Device::~DX11Device()
{
	for (auto tit = textures.begin(); tit != textures.end(); ++tit)
	{
		if (tit->second.first != NULL) { tit->second.first->Release(); }
		if (tit->second.second != NULL) { tit->second.second->Release(); }
	}

	unwrapRT.reset(NULL);
	upsampleRT.reset(NULL);
	wlsRT.reset(NULL);
	depthStencil.reset(NULL);

	if (g_pRenderTargetView) g_pRenderTargetView->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (immDevCtx) immDevCtx->Release();
	if (g_pd3dDevice) g_pd3dDevice->Release();
}

HRESULT DX11Device::CreateDynamicStructuredBuffer(
	ID3D11Device* d3dDev,
	ID3D11Buffer*& buffer,
	UINT stride,
	UINT width)
{
	D3D11_BUFFER_DESC sbDesc;
	sbDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	sbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	sbDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	sbDesc.StructureByteStride = stride;
	sbDesc.ByteWidth = width;
	sbDesc.Usage = D3D11_USAGE_DYNAMIC;
	return d3dDev->CreateBuffer(&sbDesc, NULL, &buffer);
}

