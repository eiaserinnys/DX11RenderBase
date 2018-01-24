#include "pch.h"

#include "DX11Shader.h"
#include <d3dcompiler.h>

using namespace std;

//------------------------------------------------------------------------------
IShaderCompileLog ::~IShaderCompileLog()
{
}

//------------------------------------------------------------------------------
HRESULT CompileShaderFromFile(
	WCHAR* szFileName, 
	LPCSTR szEntryPoint, 
	LPCSTR szShaderModel, 
	ID3DBlob** ppBlobOut,
	IShaderCompileLog* log = nullptr)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ComPtrT<ID3DBlob> pErrorBlob;
	hr = D3DCompileFromFile(
		szFileName,
		NULL,
		NULL,
		szEntryPoint,
		szShaderModel,
		dwShaderFlags,
		0,
		ppBlobOut,
		&pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob != NULL)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

			if (log != nullptr)
			{
				log->Log((char*)pErrorBlob->GetBufferPointer());
			}
		}
		return hr;
	}

	return S_OK;
}

//------------------------------------------------------------------------------
DX11VertexShader::DX11VertexShader(
	ID3D11Device* d3ddev, 
	const wstring& filename,
	IShaderCompileLog* log)
	: pVSBlob(NULL)
	, vs(NULL)
{
	Create(d3ddev, filename, "VS", log);
}

//------------------------------------------------------------------------------
DX11VertexShader::DX11VertexShader(
	ID3D11Device* d3ddev, 
	const wstring& filename, 
	const string& entry,
	IShaderCompileLog* log)
	: pVSBlob(NULL)
	, vs(NULL)
{
	Create(d3ddev, filename, entry, log);
}

//------------------------------------------------------------------------------
void DX11VertexShader::Create(
	ID3D11Device* d3ddev, 
	const wstring& filename, 
	const string& entry,
	IShaderCompileLog* log)
{
	// Compile the vertex shader
	HRESULT hr = CompileShaderFromFile((WCHAR*)filename.c_str(), entry.c_str(), "vs_5_0", &pVSBlob, log);
	if (FAILED(hr)) { throw hr; }

	// Create the vertex shader
	hr = d3ddev->CreateVertexShader(
		pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(),
		NULL,
		&vs);
	if (FAILED(hr)) { throw hr; }
}

//------------------------------------------------------------------------------
DX11PixelShader::DX11PixelShader(
	ID3D11Device* d3ddev, 
	const wstring& filename,
	IShaderCompileLog* log)
	: ps(NULL)
{
	Create(d3ddev, filename, "PS", log);
}

//------------------------------------------------------------------------------
DX11PixelShader::DX11PixelShader(
	ID3D11Device* d3ddev, 
	const wstring& filename, 
	const string& entry,
	IShaderCompileLog* log)
	: ps(NULL)
{
	Create(d3ddev, filename, entry, log);
}

//------------------------------------------------------------------------------
void DX11PixelShader::Create(
	ID3D11Device* d3ddev, 
	const wstring& filename, 
	const string& entry,
	IShaderCompileLog* log)
{
	// Compile the pixel shader
	ComPtrT<ID3DBlob> pPSBlob;
	HRESULT hr = CompileShaderFromFile((WCHAR*)filename.c_str(), entry.c_str(), "ps_5_0", &pPSBlob, log);
	if (FAILED(hr)) { throw hr; }

	// Create the pixel shader
	hr = d3ddev->CreatePixelShader(
		pPSBlob->GetBufferPointer(),
		pPSBlob->GetBufferSize(),
		NULL,
		&ps);
	if (FAILED(hr)) { throw hr; }
}

//------------------------------------------------------------------------------
DX11ComputeShader::DX11ComputeShader(ID3D11Device* d3ddev, const wstring& filename)
	: cs(NULL)
{
	// Compile the Compute shader
	ComPtrT<ID3DBlob> pCSBlob;
	HRESULT hr = CompileShaderFromFile((WCHAR*)filename.c_str(), "CS", "cs_4_0", &pCSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		throw hr;
	}

	// Create the Compute shader
	hr = d3ddev->CreateComputeShader(
		pCSBlob->GetBufferPointer(),
		pCSBlob->GetBufferSize(),
		NULL,
		&cs);
	if (FAILED(hr)) { throw hr; }
}

//------------------------------------------------------------------------------
DX11GeometryShader::DX11GeometryShader(
	ID3D11Device* d3ddev,
	const wstring& filename,
	D3D11_SO_DECLARATION_ENTRY* entry,
	UINT entrySize)
	: gs(NULL)
{
	Create(d3ddev, filename, "GS", entry, entrySize);
}

//------------------------------------------------------------------------------
DX11GeometryShader::DX11GeometryShader(
	ID3D11Device* d3ddev,
	const wstring& filename,
	const string& entryName,
	D3D11_SO_DECLARATION_ENTRY* entry,
	UINT entrySize)
	: gs(NULL)
{
	Create(d3ddev, filename, entryName, entry, entrySize);
}

//------------------------------------------------------------------------------
void DX11GeometryShader::Create(
	ID3D11Device* d3ddev,
	const wstring& filename,
	const string& entryName,
	D3D11_SO_DECLARATION_ENTRY* entry,
	UINT entrySize)
{
	// Compile the Geometry shader
	ComPtrT<ID3DBlob> pCSBlob;
	HRESULT hr = CompileShaderFromFile((WCHAR*)filename.c_str(), entryName.c_str(), "gs_4_0", &pCSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		throw hr;
	}

	// Create the Geometry shader
	hr = d3ddev->CreateGeometryShaderWithStreamOutput(
		pCSBlob->GetBufferPointer(),
		pCSBlob->GetBufferSize(),
		entry,
		entrySize,
		NULL,
		0,
		0,
		NULL,
		&gs);
	if (FAILED(hr)) { throw hr; }
}