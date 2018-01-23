#pragma once

#include <memory>

#include "DX11InputLayout.h"
#include "ShaderManager.h"

//------------------------------------------------------------------------------
struct ShaderDefine
{
	ShaderDefine(
		ID3D11Device* dev,
		IVertexShaderManager* vs,
		IPixelShaderManager* ps,
		const std::wstring& fileName,
		D3D11_INPUT_ELEMENT_DESC* layout,
		UINT layoutCount);

	void Set(
		ID3D11DeviceContext* devCtx,
		IVertexShaderManager* vs,
		IPixelShaderManager* ps);

	std::unique_ptr<IDX11InputLayout> vertexLayout;

	std::wstring fxFileName;
};
