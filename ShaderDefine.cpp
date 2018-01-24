#include "pch.h"
#include "ShaderDefine.h"

#include "DX11Shader.h"

using namespace std;

ShaderDefine::ShaderDefine(
	const string& key, 
	ID3D11Device* dev,
	IVertexShaderManager* vs,
	IPixelShaderManager* ps,
	const std::wstring& fileName,
	D3D11_INPUT_ELEMENT_DESC* layout,
	UINT layoutCount,
	IShaderCompileLog* log)
	: key(key)
{
	vs->Load(key, fileName, "VS", false, log);
	ps->Load(key, fileName, "PS", false, log);
	
	if (layout != nullptr && layoutCount > 0)
	{
		auto vs_ = vs->Find(key);

		if (vs_ != nullptr)
		{
			vertexLayout.reset(IDX11InputLayout::Create(
				dev, layout, layoutCount, vs_->pVSBlob));
		}
	}
}

bool ShaderDefine::Set(
	ID3D11DeviceContext* devCtx,
	IVertexShaderManager* vs,
	IPixelShaderManager* ps)
{
	if (vertexLayout != nullptr)
	{
		vertexLayout->Apply(devCtx);
	}

	if (!vs->Set(key)) { return false; }
	if (!ps->Set(key)) { return false; }

	return true;
}

