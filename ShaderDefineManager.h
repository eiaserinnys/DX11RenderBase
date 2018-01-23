#pragma once

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

	virtual void Set(const std::string& key) = 0;

	static IShaderDefineManager* Create(DX11Device* device);
};