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

	std::pair<ID3D11Resource*, ID3D11ShaderResourceView*> GetTexture(const std::string& fileName);
	void SetTexture(int index, const std::string& fileName);

	void UnloadTexture(const std::string& fileName);

	std::pair<ID3D11Resource*, ID3D11ShaderResourceView*> CreateVideoTexture(const std::string& name, int width, int height);

	static HRESULT CreateDynamicStructuredBuffer(
		ID3D11Device* d3dDev,
		ID3D11Buffer*& buffer,
		UINT stride,
		UINT width);

	void ReloadTexture();

	D3D_DRIVER_TYPE                     g_driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL                   g_featureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device*                       g_pd3dDevice = NULL;
	ID3D11DeviceContext*                immDevCtx = NULL;
	IDXGISwapChain*                     g_pSwapChain = NULL;
	ID3D11RenderTargetView*             g_pRenderTargetView = NULL;
	std::auto_ptr<DX11DepthStencil>		depthStencil;

	RenderTarget::Type					rtType = RenderTarget::Backbuffer;

	std::auto_ptr<IDX11RenderTarget>	unwrapRT;
	UINT unwrapWidth = 2048, unwrapHeight = 2048;

	std::auto_ptr<IDX11RenderTarget>	upsampleRT;
	UINT upsampleWidth = 1920, upsampleHeight = 1080;

	std::auto_ptr<IDX11RenderTarget>	wlsRT;
	UINT wlsWidth = 1024, wlsHeight = 1024;

	UINT width = 0, height = 0;

	std::map<
		std::string,
		std::pair<
		ID3D11Resource*,
		ID3D11ShaderResourceView*>> textures;

	HRESULT hr;

private:
	std::pair<ID3D11Resource*, ID3D11ShaderResourceView*>
		LoadTextureInternal(const std::string& fileName);
};

