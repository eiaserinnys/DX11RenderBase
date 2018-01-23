#pragma once

#include "ComPtr.h"
#include "RenderTargetHelper.h"

struct DepthStencilTarget
{
	DepthStencilTarget(
		ID3D11Device* d3dDev,
		DXGI_FORMAT fmt,		// DXGI_FORMAT_R32_TYPELESS
		DXGI_FORMAT dsvFmt,		// DXGI_FORMAT_D32_FLOAT
		DXGI_FORMAT srvFmt,		// DXGI_FORMAT_R32_FLOAT
		int width,
		int height)
		: targetWidth(width)
		, targetHeight(height)
		, format(fmt)
	{
		D3D11_TEXTURE2D_DESC dtd =
		{
			targetWidth,
			targetHeight,
			1,		//UINT MipLevels;
			1,		//UINT ArraySize;
			fmt,	//DXGI_FORMAT Format;
			1,		//DXGI_SAMPLE_DESC SampleDesc;
			0,
			D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
			D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
			0,		//UINT CPUAccessFlags;
			0		//UINT MiscFlags;    
		};

		HRESULT hr;

		if (FAILED(hr = d3dDev->CreateTexture2D(&dtd, nullptr, &texture))) { throw hr; }

		if (FAILED(hr = RenderTargetHelper::CreateDepthStencilView(d3dDev, texture, dsvFmt, &dsv))) { throw hr; }

		if (FAILED(hr = RenderTargetHelper::CreateShaderResourceView(d3dDev, texture, srvFmt, &srv))) { throw hr; }
	}

	int targetWidth, targetHeight;
	DXGI_FORMAT format;
	ComPtrT<ID3D11Texture2D> texture;
	ComPtrT<ID3D11DepthStencilView> dsv;
	ComPtrT<ID3D11ShaderResourceView> srv;
};
