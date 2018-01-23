#pragma once

#include "ComPtr.h"
#include "RenderTargetHelper.h"

////////////////////////////////////////////////////////////////////////////////
struct GenericRenderTarget
{
	GenericRenderTarget(
		ID3D11Device* d3dDev,
		DXGI_FORMAT fmt,
		UINT width,
		UINT height)
		: targetWidth(width)
		, targetHeight(height)
		, format(fmt)
	{
		DXGI_SAMPLE_DESC sampleDesc = 
		{
			1,		// UINT Count
			0		// UINT Quality
		};

		D3D11_TEXTURE2D_DESC dtd =
		{
			targetWidth,
			targetHeight,
			1,						//UINT MipLevels;
			1,						//UINT ArraySize;
			fmt,					//DXGI_FORMAT Format;
			sampleDesc,				//DXGI_SAMPLE_DESC SampleDesc;
			D3D11_USAGE_DEFAULT,	//D3D11_USAGE Usage;
			D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
			0,						//UINT CPUAccessFlags;
			0						//UINT MiscFlags;    
		};

		HRESULT hr;

		hr = d3dDev->CreateTexture2D(&dtd, nullptr, &texture);
		if (FAILED(hr)) { throw hr; }

		hr = RenderTargetHelper::CreateRenderTargetview(d3dDev, texture, format, &view);
		if (FAILED(hr)) { throw hr; }

		hr = RenderTargetHelper::CreateShaderResourceView(d3dDev, texture, fmt, &srv);
		if (FAILED(hr)) { throw hr; }
	}

	UINT targetWidth, targetHeight;
	DXGI_FORMAT format;
	ComPtrT<ID3D11Texture2D> texture;
	ComPtrT<ID3D11RenderTargetView> view;
	ComPtrT<ID3D11ShaderResourceView> srv;
};
