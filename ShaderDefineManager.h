#pragma once

#include "DX11Shader.h"

class DX11Device;
class IVertexShaderManager;
class IPixelShaderManager;

class IShaderDefineManager {
public:
	virtual ~IShaderDefineManager();

	virtual void Load(
		const std::string& key,
		const std::wstring& fileName,
		D3D11_INPUT_ELEMENT_DESC* layout,
		UINT layoutCount) = 0; 

	virtual bool Set(const std::string& key) = 0;

	virtual void Reload() = 0;

	virtual void SetCompileLogger(IShaderCompileLog* log) = 0;

	static IShaderDefineManager* Create(DX11Device* device);
};