#include "pch.h"

#include "DX11Shader.h"
#include <d3dcompiler.h>

using namespace std;

HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
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

	ID3DBlob* pErrorBlob = nullptr;
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
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		if (pErrorBlob) pErrorBlob->Release();
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

DX11VertexShader::DX11VertexShader(ID3D11Device* d3ddev, const wstring& filename)
	: pVSBlob(NULL)
	, hr(S_OK)
	, vs(NULL)
{
	Create(d3ddev, filename, "VS");
}

DX11VertexShader::DX11VertexShader(ID3D11Device* d3ddev, const wstring& filename, const string& entry)
	: pVSBlob(NULL)
	, hr(S_OK)
	, vs(NULL)
{
	Create(d3ddev, filename, entry);
}

void DX11VertexShader::Create(ID3D11Device* d3ddev, const wstring& filename, const string& entry)
{
	HRESULT hr;

	// Compile the vertex shader
	hr = CompileShaderFromFile((WCHAR*)filename.c_str(), entry.c_str(), "vs_5_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		throw hr;
	}

	// Create the vertex shader
	hr = d3ddev->CreateVertexShader(
		pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(),
		NULL,
		&vs);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		pVSBlob = NULL;
		throw hr;
	}
}

DX11VertexShader::~DX11VertexShader()
{
	if (pVSBlob != NULL) { pVSBlob->Release(); pVSBlob = NULL; }
	if (vs != NULL) { vs->Release(); vs = NULL; }
}

DX11PixelShader::DX11PixelShader(ID3D11Device* d3ddev, const wstring& filename)
	: hr(S_OK)
	, ps(NULL)
{
	Create(d3ddev, filename, "PS");
}

DX11PixelShader::DX11PixelShader(ID3D11Device* d3ddev, const wstring& filename, const string& entry)
	: hr(S_OK)
	, ps(NULL)
{
	Create(d3ddev, filename, entry);
}

void DX11PixelShader::Create(ID3D11Device* d3ddev, const wstring& filename, const string& entry)
{
	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
	hr = CompileShaderFromFile((WCHAR*)filename.c_str(), entry.c_str(), "ps_5_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		throw hr;
	}

	// Create the pixel shader
	hr = d3ddev->CreatePixelShader(
		pPSBlob->GetBufferPointer(),
		pPSBlob->GetBufferSize(),
		NULL,
		&ps);
	pPSBlob->Release();
	pPSBlob = NULL;
	if (FAILED(hr))
	{
		throw hr;
	}
}

DX11PixelShader::~DX11PixelShader()
{
	if (ps != NULL) { ps->Release(); ps = NULL; }
}

DX11ComputeShader::DX11ComputeShader(ID3D11Device* d3ddev, const wstring& filename)
	: hr(S_OK)
	, cs(NULL)
{
	// Compile the Compute shader
	ID3DBlob* pCSBlob = NULL;
	hr = CompileShaderFromFile((WCHAR*)filename.c_str(), "CS", "cs_4_0", &pCSBlob);
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
	pCSBlob->Release();
	pCSBlob = NULL;
	if (FAILED(hr))
	{
		throw hr;
	}
}

DX11ComputeShader::~DX11ComputeShader()
{
	if (cs != NULL) { cs->Release(); cs = NULL; }
}

DX11GeometryShader::DX11GeometryShader(
	ID3D11Device* d3ddev,
	const wstring& filename,
	D3D11_SO_DECLARATION_ENTRY* entry,
	UINT entrySize)
	: hr(S_OK)
	, gs(NULL)
{
	Create(d3ddev, filename, "GS", entry, entrySize);
}

DX11GeometryShader::DX11GeometryShader(
	ID3D11Device* d3ddev,
	const wstring& filename,
	const string& entryName,
	D3D11_SO_DECLARATION_ENTRY* entry,
	UINT entrySize)
	: hr(S_OK)
	, gs(NULL)
{
	Create(d3ddev, filename, entryName, entry, entrySize);
}

DX11GeometryShader::~DX11GeometryShader()
{
	if (gs != NULL) { gs->Release(); gs = NULL; }
}

void DX11GeometryShader::Create(
	ID3D11Device* d3ddev,
	const wstring& filename,
	const string& entryName,
	D3D11_SO_DECLARATION_ENTRY* entry,
	UINT entrySize)
{
	// Compile the Geometry shader
	ID3DBlob* pCSBlob = NULL;
	hr = CompileShaderFromFile((WCHAR*)filename.c_str(), entryName.c_str(), "gs_4_0", &pCSBlob);
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
	pCSBlob->Release();
	pCSBlob = NULL;
	if (FAILED(hr))
	{
		throw hr;
	}
}