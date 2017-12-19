#pragma once

#include <D3DX11.h>

struct DX11SaveTextureToPng
{
	static HRESULT Save(
		ID3D11Device* dev,
		ID3D11DeviceContext* devCtx,
		ID3D11Texture2D* texture,
		const char* name,
		int& width, int& height, 
		int& cx, int& cy, int& cw, int& ch, 
		bool crop);

private:
	static HRESULT SaveToPng(
		ID3D11DeviceContext* devCtx, 
		ID3D11Texture2D* staging,
		const char* name,
		int& width, int& height, 
		int& cx, int& cy, int& cw, int& ch, 
		bool crop);
};