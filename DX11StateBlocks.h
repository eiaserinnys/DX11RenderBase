#pragma once

#include <d3d11.h>

class IDX11SamplerState {
public:
	virtual ~IDX11SamplerState();
	virtual void Apply(ID3D11DeviceContext* devCtx, int slot) = 0;

	static IDX11SamplerState* Create(ID3D11Device* d3dDev, const D3D11_SAMPLER_DESC& desc);

	static void DefaultDesc(D3D11_SAMPLER_DESC& desc);

	static IDX11SamplerState* Create_LinearNoMipWrap(ID3D11Device* d3dDev);
	static IDX11SamplerState* Create_LinearNoMipClamp(ID3D11Device* d3dDev);
};

class IDX11RasterizerState {
public:
	virtual ~IDX11RasterizerState();
	virtual void Apply(ID3D11DeviceContext* devCtx) = 0;

	static IDX11RasterizerState* Create(ID3D11Device* d3dDev, const D3D11_RASTERIZER_DESC& desc);

	static IDX11RasterizerState* Create_Default(ID3D11Device* d3dDev);
	static IDX11RasterizerState* Create_CullNone(ID3D11Device* d3dDev);

	static void DefaultDesc(D3D11_RASTERIZER_DESC& desc);
};

class IDX11BlendState {
public:
	virtual ~IDX11BlendState();
	virtual void Apply(ID3D11DeviceContext* devCtx) = 0;
	static IDX11BlendState* Create(ID3D11Device* d3dDev, const D3D11_BLEND_DESC& desc);

	static IDX11BlendState* Create_AlphaBlend(ID3D11Device* d3dDev);
	static IDX11BlendState* Create_Preserve(ID3D11Device* d3dDev);

	static void DefaultDesc(D3D11_BLEND_DESC& blendDesc);
};

class IDX11DepthStencilState {
public:
	virtual ~IDX11DepthStencilState();
	virtual void Apply(ID3D11DeviceContext* devCtx) = 0;
	
	static IDX11DepthStencilState* Create(ID3D11Device* d3dDev, const D3D11_DEPTH_STENCIL_DESC& desc);

	static IDX11DepthStencilState* Create_Default(ID3D11Device* d3dDev);
	static IDX11DepthStencilState* Create_Disabled(ID3D11Device* d3dDev);
	static IDX11DepthStencilState* Create_Always(ID3D11Device* d3dDev);

	static void DefaultDesc(D3D11_DEPTH_STENCIL_DESC& depthStencilDesc);
};
