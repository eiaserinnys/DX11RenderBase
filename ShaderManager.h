#pragma once

#include <string>

class DX11Device;
class DX11VertexShader;
class IShaderCompileLog;

class IVertexShaderManager {
public:
	virtual ~IVertexShaderManager();

	virtual bool Load(
		const std::string& key,
		const std::wstring& pathName,
		const std::string& entryName = "VS",
		IShaderCompileLog* log = nullptr, 
		bool force = false) = 0;

	virtual void Set(const std::string& key) = 0;

	virtual DX11VertexShader* Find(const std::string& key) = 0;

	virtual void Reload(IShaderCompileLog* log = nullptr) = 0;
	
	static IVertexShaderManager* Create(DX11Device* device);
};

class IPixelShaderManager {
public:
	virtual ~IPixelShaderManager();

	virtual bool Load(
		const std::string& key, 
		const std::wstring& pathName,
		const std::string& entryName = "PS",
		IShaderCompileLog* log = nullptr,
		bool force = false) = 0;

	virtual void Set(const std::string& key) = 0;

	virtual void Reload(IShaderCompileLog* log = nullptr) = 0;

	static IPixelShaderManager* Create(DX11Device* device);
};