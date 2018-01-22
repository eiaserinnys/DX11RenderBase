#pragma once

#include <windows.h>
#include <d3d11.h>

#include <memory>
#include <map>

#include "DX11DepthStencil.h"
#include "DX11RenderTarget.h"

struct DepthStencil;

class DX11Device {
public:
	struct RenderTarget
	{
		enum Type
		{
			Backbuffer,
			ForUnwrap,
			ForUpsample,
			ForWls,
		};
	};

	DX11Device(HWND hwnd);
	~DX11Device();

	void SetScreenshotMode(RenderTarget::Type rtType);

	void RestoreRenderTarget();
	void ClearRenderTarget();
	IDX11RenderTarget* GetRenderTarget();
	int GetRenderTargetWidth();
	int GetRenderTargetHeight();

	void SetTexture(int index, ID3D11ShaderResourceView* textureRSView);

	D3D_DRIVER_TYPE                     g_driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL                   g_featureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device*                       g_pd3dDevice = NULL;
	ID3D11DeviceContext*                immDevCtx = NULL;
	IDXGISwapChain*                     g_pSwapChain = NULL;

	RenderTarget::Type					rtType = RenderTarget::Backbuffer;

	std::unique_ptr<IDX11RenderTarget>	backBuffer;

	std::unique_ptr<IDX11RenderTarget>	unwrapRT;
	UINT unwrapWidth = 2048, unwrapHeight = 2048;

	std::unique_ptr<IDX11RenderTarget>	upsampleRT;
	UINT upsampleWidth = 1920, upsampleHeight = 1080;

	std::unique_ptr<IDX11RenderTarget>	wlsRT;
	UINT wlsWidth = 1024, wlsHeight = 1024;

	UINT width = 0, height = 0;

	HRESULT hr;
};

