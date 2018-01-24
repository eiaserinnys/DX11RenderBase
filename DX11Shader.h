#pragma once

#include <windows.h>
#include <d3d11.h>
#include <string>

#include "ComPtr.h"

//------------------------------------------------------------------------------
class IShaderCompileLog {
public:
	virtual ~IShaderCompileLog();

	virtual void Log(const char* msg) = 0;
};

//------------------------------------------------------------------------------
class DX11VertexShader {
public:
	DX11VertexShader(ID3D11Device* d3ddev, const std::wstring& filename, IShaderCompileLog* log = nullptr);
	DX11VertexShader(ID3D11Device* d3ddev, const std::wstring& filename, const std::string& entry, IShaderCompileLog* log = nullptr);

	ComPtrT<ID3DBlob> pVSBlob;
	ComPtrT<ID3D11VertexShader> vs;

private:
	void Create(ID3D11Device* d3ddev, const std::wstring& filename, const std::string& entry, IShaderCompileLog* log);
};

//------------------------------------------------------------------------------
class DX11PixelShader {
public:
	DX11PixelShader(ID3D11Device* d3ddev, const std::wstring& filename, IShaderCompileLog* log = nullptr);
	DX11PixelShader(ID3D11Device* d3ddev, const std::wstring& filename, const std::string& entry, IShaderCompileLog* log = nullptr);

	ComPtrT<ID3D11PixelShader> ps;

private:
	void Create(ID3D11Device* d3ddev, const std::wstring& filename, const std::string& entry, IShaderCompileLog* log);
};

//------------------------------------------------------------------------------
class DX11ComputeShader {
public:
	DX11ComputeShader(ID3D11Device* d3ddev, const std::wstring& filename);

	ComPtrT<ID3D11ComputeShader> cs;
};

//------------------------------------------------------------------------------
class DX11GeometryShader {
public:
	DX11GeometryShader(
		ID3D11Device* d3ddev,
		const std::wstring& filename,
		D3D11_SO_DECLARATION_ENTRY* entry,
		UINT entrySize);
	DX11GeometryShader(
		ID3D11Device* d3ddev,
		const std::wstring& filename,
		const std::string& entryName,
		D3D11_SO_DECLARATION_ENTRY* entry,
		UINT entrySize);

	ComPtrT<ID3D11GeometryShader> gs;

private:
	void Create(
		ID3D11Device* d3ddev,
		const std::wstring& filename,
		const std::string& entryName,
		D3D11_SO_DECLARATION_ENTRY* entry,
		UINT entrySize);
};
