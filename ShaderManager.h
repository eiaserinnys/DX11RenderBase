#pragma once

#include <string>

class DX11Device;
class DX11VertexShader;

class IVertexShaderManager {
public:
	virtual ~IVertexShaderManager();

	virtual bool Load(
		const std::wstring& pathName,
		const std::string& entryName = "VS",
		bool force = false) = 0;

	virtual void Set(const std::wstring& pathName) = 0;

	virtual DX11VertexShader* Find(const std::wstring& pathName) = 0;
	
	static IVertexShaderManager* Create(DX11Device* device);
};

class IPixelShaderManager {
public:
	virtual ~IPixelShaderManager();

	virtual bool Load(
		const std::wstring& pathName,
		const std::string& entryName = "PS",
		bool force = false) = 0;

	virtual void Set(const std::wstring& pathName) = 0;

	static IPixelShaderManager* Create(DX11Device* device);
};