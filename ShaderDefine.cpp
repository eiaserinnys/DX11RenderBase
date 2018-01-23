#include "pch.h"
#include "ShaderDefine.h"

#include "DX11Shader.h"

using namespace std;

ShaderDefine::ShaderDefine(
	ID3D11Device* dev,
	IVertexShaderManager* vs,
	IPixelShaderManager* ps,
	const std::wstring& fileName,
	D3D11_INPUT_ELEMENT_DESC* layout,
	UINT layoutCount)
	: fxFileName(fileName)
{
	vs->Load(fxFileName);
	ps->Load(fxFileName);
	
	if (layout != nullptr && layoutCount > 0)
	{
		auto vs_ = vs->Find(fxFileName);

		if (vs_ != nullptr)
		{
			vertexLayout.reset(IDX11InputLayout::Create(
				dev, layout, layoutCount, vs_->pVSBlob));
		}
	}
}

void ShaderDefine::Set(
	ID3D11DeviceContext* devCtx,
	IVertexShaderManager* vs,
	IPixelShaderManager* ps)
{
	if (vertexLayout != nullptr)
	{
		vertexLayout->Apply(devCtx);
	}

	vs->Set(fxFileName);
	ps->Set(fxFileName);
}

