#pragma once

#include <string>

class DX11Device;
class DX11VertexShader;
class DX11PixelShader;
class IShaderCompileLog;

template <typename ShaderType>
class IShaderManagerT {
public:
	virtual ~IShaderManagerT() {}

	virtual bool Load(
		const std::string& key,
		const std::wstring& pathName,
		const std::string& entryName,
		bool force,
		IShaderCompileLog* log) = 0;

	virtual bool Set(const std::string& key) = 0;

	virtual ShaderType* Find(const std::string& key) = 0;

	virtual void Reload(IShaderCompileLog* log = nullptr) = 0;
};

class IVertexShaderManager : public IShaderManagerT<DX11VertexShader> {
public:	
	static IVertexShaderManager* Create(DX11Device* device);
};

class IPixelShaderManager : public IShaderManagerT<DX11PixelShader> {
public:
	static IPixelShaderManager* Create(DX11Device* device);
};