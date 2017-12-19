#pragma once

#include <d3d11.h>

class IDX11InputLayout {
public:
	virtual ~IDX11InputLayout();
	virtual void Apply(ID3D11DeviceContext* devCtx) = 0;

	static IDX11InputLayout* Create(
		ID3D11Device* dev,
		D3D11_INPUT_ELEMENT_DESC* layout,
		int numElements,
		ID3DBlob* blob);
};