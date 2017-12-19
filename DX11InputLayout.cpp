#include "pch.h"
#include "DX11InputLayout.h"

class DX11InputLayout : public IDX11InputLayout {
public:
	DX11InputLayout(
		ID3D11Device* dev,
		D3D11_INPUT_ELEMENT_DESC* layout,
		int numElements,
		ID3DBlob* blob)
		: inputLayout(NULL)
	{
		// Create the input layout
		HRESULT hr = dev->CreateInputLayout(
			layout,
			numElements,
			blob->GetBufferPointer(),
			blob->GetBufferSize(),
			&inputLayout);
		if (FAILED(hr)) { throw hr; }
	}

	~DX11InputLayout()
	{
		if (inputLayout != NULL) { inputLayout->Release(); inputLayout = NULL; }
	}

	virtual void Apply(ID3D11DeviceContext* devCtx)
	{
		devCtx->IASetInputLayout(inputLayout);
	}

	ID3D11InputLayout* inputLayout;
};

IDX11InputLayout::~IDX11InputLayout() {}

IDX11InputLayout* IDX11InputLayout::Create(
	ID3D11Device* dev,
	D3D11_INPUT_ELEMENT_DESC* layout,
	int numElements,
	ID3DBlob* blob)
{
	return new DX11InputLayout(dev, layout, numElements, blob);
}