#pragma once

#include <memory>

#include <DX11Device.h>
#include <DX11Shader.h>

struct GlobalContext
{
	GlobalContext(HWND hwnd);
	~GlobalContext();

	void Reload();
	void ReloadShader();

	HWND hwnd;

	std::auto_ptr<DX11Device> d3d11;

	float scale;
	float offsetAngle;

	struct DXResource
	{
		std::auto_ptr<DX11VertexShader> quadVS;
		std::auto_ptr<DX11PixelShader> quadPS;
	};
	std::auto_ptr<DXResource> dxr;

	bool advance = true;
};
