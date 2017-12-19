#include "pch.h"
#include "Global.h"

using namespace std;

GlobalContext::GlobalContext(HWND hwnd)
	: hwnd(hwnd)
{
	d3d11.reset(new DX11Device(hwnd));
	if (FAILED(d3d11->hr))
	{
		d3d11.reset(NULL);
		throw E_FAIL;
	}

	dxr.reset(new DXResource);

	ReloadShader();
}

GlobalContext::~GlobalContext()
{
	if (d3d11->immDevCtx) d3d11->immDevCtx->ClearState();

	dxr.reset(NULL);

	d3d11.reset(NULL);
}

void GlobalContext::Reload()
{
	d3d11->ReloadTexture();
	ReloadShader();
}

void GlobalContext::ReloadShader()
{
	dxr->quadVS.reset(new DX11VertexShader(d3d11->g_pd3dDevice, L"Shaders/DX11FullScreenQuad.fx"));
	dxr->quadPS.reset(new DX11PixelShader(d3d11->g_pd3dDevice, L"Shaders/DX11FullScreenQuad.fx"));
}


