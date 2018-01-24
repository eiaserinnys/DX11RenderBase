#include "pch.h"
#include "DX11StateBlocks.h"

class DX11SamplerState : public IDX11SamplerState {
public:
	DX11SamplerState(ID3D11Device* dev, const D3D11_SAMPLER_DESC& desc)
		: state(NULL)
	{
		HRESULT hr = dev->CreateSamplerState(&desc, &state);
		if (FAILED(hr)) { throw hr; }
	}

	~DX11SamplerState()
	{
		if (state != NULL) { state->Release(); state = NULL; }
	}

	void Apply(ID3D11DeviceContext* devCtx, int slot)
	{
		devCtx->PSSetSamplers(slot, 1, &state);
	}

private:
	ID3D11SamplerState* state;
};

IDX11SamplerState::~IDX11SamplerState() {}

IDX11SamplerState* IDX11SamplerState::Create(ID3D11Device* dev, const D3D11_SAMPLER_DESC& desc)
{
	return new DX11SamplerState(dev, desc);
}

IDX11SamplerState* IDX11SamplerState::Create_Default(ID3D11Device* d3dDev)
{
	D3D11_SAMPLER_DESC desc;
	DefaultDesc(desc);
	return IDX11SamplerState::Create(d3dDev, desc);
}

IDX11SamplerState* IDX11SamplerState::Create_LinearNoMipWrap(ID3D11Device* d3dDev)
{
	D3D11_SAMPLER_DESC desc;
	DefaultDesc(desc);
	desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.MinLOD = 0;
	desc.MaxLOD = 0;
	return IDX11SamplerState::Create(d3dDev, desc);
}

IDX11SamplerState* IDX11SamplerState::Create_LinearNoMipClamp(ID3D11Device* d3dDev)
{
	D3D11_SAMPLER_DESC desc;
	DefaultDesc(desc);
	desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	desc.MinLOD = 0;
	desc.MaxLOD = 0;
	return IDX11SamplerState::Create(d3dDev, desc);
}

void IDX11SamplerState::DefaultDesc(D3D11_SAMPLER_DESC& sampDesc)
{
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.MinLOD = -FLT_MAX;
	sampDesc.MaxLOD = +FLT_MAX;
	sampDesc.MipLODBias = 0.0f;
	sampDesc.MaxAnisotropy = 1;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.BorderColor[0] = 1.0f;
	sampDesc.BorderColor[1] = 1.0f;
	sampDesc.BorderColor[2] = 1.0f;
	sampDesc.BorderColor[3] = 1.0f;
}

////////////////////////////////////////////////////////////////////////////////

class DX11RasterizerState : public IDX11RasterizerState {
public:
	DX11RasterizerState(ID3D11Device* dev, const D3D11_RASTERIZER_DESC& desc)
		: state(NULL)
	{
		HRESULT hr = dev->CreateRasterizerState(&desc, &state);
		if (FAILED(hr)) { throw hr; }
	}

	~DX11RasterizerState()
	{
		if (state != NULL) { state->Release(); state = NULL; }
	}

	void Apply(ID3D11DeviceContext* devCtx)
	{
		devCtx->RSSetState(state);
	}

private:
	ID3D11RasterizerState* state;
};

IDX11RasterizerState::~IDX11RasterizerState() {}

IDX11RasterizerState* IDX11RasterizerState::Create(ID3D11Device* dev, const D3D11_RASTERIZER_DESC& desc)
{
	return new DX11RasterizerState(dev, desc);
}

IDX11RasterizerState* IDX11RasterizerState::Create_Default(ID3D11Device* d3dDev)
{
	D3D11_RASTERIZER_DESC desc;
	DefaultDesc(desc);
	return Create(d3dDev, desc);
}

IDX11RasterizerState* IDX11RasterizerState::Create_CullNone(ID3D11Device* d3dDev)
{
	D3D11_RASTERIZER_DESC desc;
	DefaultDesc(desc);
	desc.CullMode = D3D11_CULL_NONE;
	return Create(d3dDev, desc);
}

void IDX11RasterizerState::DefaultDesc(D3D11_RASTERIZER_DESC& rasterDesc)
{
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.FrontCounterClockwise = FALSE;
	rasterDesc.DepthBias = 0;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = TRUE;
	rasterDesc.ScissorEnable = FALSE;
	rasterDesc.MultisampleEnable = FALSE;
	rasterDesc.AntialiasedLineEnable = FALSE;
}

////////////////////////////////////////////////////////////////////////////////

class DX11BlendState : public IDX11BlendState {
public:
	DX11BlendState(ID3D11Device* dev, const D3D11_BLEND_DESC& desc)
		: state(NULL)
	{
		HRESULT hr = dev->CreateBlendState(&desc, &state);
		if (FAILED(hr)) { throw hr; }
	}

	~DX11BlendState()
	{
		if (state != NULL) { state->Release(); state = NULL; }
	}

	void Apply(ID3D11DeviceContext* devCtx)
	{
		devCtx->OMSetBlendState(state, NULL, 0xffffffff);
	}

private:
	ID3D11BlendState* state;
};

IDX11BlendState::~IDX11BlendState() {}

IDX11BlendState* IDX11BlendState::Create(ID3D11Device* dev, const D3D11_BLEND_DESC& desc)
{
	return new DX11BlendState(dev, desc);
}

IDX11BlendState* IDX11BlendState::Create_Default(ID3D11Device* d3dDev)
{
	D3D11_BLEND_DESC desc;
	DefaultDesc(desc);
	return IDX11BlendState::Create(d3dDev, desc);
}

IDX11BlendState* IDX11BlendState::Create_Preserve(ID3D11Device* d3dDev)
{
	D3D11_BLEND_DESC desc;
	DefaultDesc(desc);
	desc.RenderTarget[0].BlendEnable = TRUE;
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	return IDX11BlendState::Create(d3dDev, desc);
}

IDX11BlendState* IDX11BlendState::Create_AlphaBlend(ID3D11Device* d3dDev)
{
	D3D11_BLEND_DESC desc;
	DefaultDesc(desc);
	desc.RenderTarget[0].BlendEnable = TRUE;
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	return IDX11BlendState::Create(d3dDev, desc);
}

void IDX11BlendState::DefaultDesc(D3D11_BLEND_DESC& desc)
{
	ZeroMemory(&desc, sizeof(desc));
	desc.AlphaToCoverageEnable = FALSE;
	desc.IndependentBlendEnable = FALSE;
	desc.RenderTarget[0].BlendEnable = FALSE;
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
}

////////////////////////////////////////////////////////////////////////////////

class DX11DepthStencilState : public IDX11DepthStencilState {
public:
	DX11DepthStencilState(ID3D11Device* dev, const D3D11_DEPTH_STENCIL_DESC& desc)
		: state(NULL)
	{
		HRESULT hr = dev->CreateDepthStencilState(&desc, &state);
		if (FAILED(hr)) { throw hr; }
	}

	~DX11DepthStencilState()
	{
		if (state != NULL) { state->Release(); state = NULL; }
	}

	void Apply(ID3D11DeviceContext* devCtx)
	{
		devCtx->OMSetDepthStencilState(state, 0);
	}

private:
	ID3D11DepthStencilState* state;
};

IDX11DepthStencilState::~IDX11DepthStencilState() {}

IDX11DepthStencilState* IDX11DepthStencilState::Create(ID3D11Device* dev, const D3D11_DEPTH_STENCIL_DESC& desc)
{
	return new DX11DepthStencilState(dev, desc);
}

IDX11DepthStencilState* IDX11DepthStencilState::Create_Default(ID3D11Device* d3dDev)
{
	D3D11_DEPTH_STENCIL_DESC desc;
	DefaultDesc(desc);
	return Create(d3dDev, desc);
}

IDX11DepthStencilState* IDX11DepthStencilState::Create_Disabled(ID3D11Device* d3dDev)
{
	D3D11_DEPTH_STENCIL_DESC desc;
	DefaultDesc(desc);
	desc.DepthEnable = FALSE;
	return Create(d3dDev, desc);
}

IDX11DepthStencilState* IDX11DepthStencilState::Create_Always(ID3D11Device* d3dDev)
{
	D3D11_DEPTH_STENCIL_DESC desc;
	DefaultDesc(desc);
	desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	return Create(d3dDev, desc);
}

void IDX11DepthStencilState::DefaultDesc(D3D11_DEPTH_STENCIL_DESC& depthStencilDesc)
{
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
}

