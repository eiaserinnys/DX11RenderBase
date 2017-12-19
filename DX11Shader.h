#pragma once

#include <windows.h>
#include <d3d11.h>
#include <string>

class DX11VertexShader {
public:
	DX11VertexShader(ID3D11Device* d3ddev, const std::wstring& filename);
	DX11VertexShader(ID3D11Device* d3ddev, const std::wstring& filename, const std::string& entry);
	~DX11VertexShader();

	HRESULT hr;
	ID3DBlob* pVSBlob;
	ID3D11VertexShader* vs;

private:
	void Create(ID3D11Device* d3ddev, const std::wstring& filename, const std::string& entry);
};

class DX11PixelShader {
public:
	DX11PixelShader(ID3D11Device* d3ddev, const std::wstring& filename);
	DX11PixelShader(ID3D11Device* d3ddev, const std::wstring& filename, const std::string& entry);
	~DX11PixelShader();

	HRESULT hr;
	ID3D11PixelShader* ps;

private:
	void Create(ID3D11Device* d3ddev, const std::wstring& filename, const std::string& entry);
};

class DX11ComputeShader {
public:
	DX11ComputeShader(ID3D11Device* d3ddev, const std::wstring& filename);
	~DX11ComputeShader();

	HRESULT hr;
	ID3D11ComputeShader* cs;
};

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
	~DX11GeometryShader();

	HRESULT hr;
	ID3D11GeometryShader* gs;

private:
	void Create(
		ID3D11Device* d3ddev,
		const std::wstring& filename,
		const std::string& entryName,
		D3D11_SO_DECLARATION_ENTRY* entry,
		UINT entrySize);
};

class DX11SamplerState {
public:
	DX11SamplerState();
	~DX11SamplerState();
};