#pragma once

#undef new
#undef delete

#include "DX11Buffer.h"
#include "ComPtr.h"

template <typename Struct>
class DX11ConstantBufferT {
protected:
	Struct changing;
	ComPtrT<ID3D11Buffer> chaingBuf;

public:
	DX11ConstantBufferT(
		ID3D11Device* d3dDev,
		ID3D11DeviceContext* devCtx)
		: chaingBuf(NULL)
	{
		HRESULT hr;

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.ByteWidth = sizeof(Struct);
		hr = d3dDev->CreateBuffer(&bd, NULL, &chaingBuf);
		if (FAILED(hr)) { throw hr; }
	}

	void* operator new(size_t size) { return _aligned_malloc(size, 16); }
	void operator delete(void* ptr) { return _aligned_free(ptr); }

protected:
	void UpdateInternal(
		ID3D11DeviceContext* devCtx)
	{
		devCtx->UpdateSubresource(
			chaingBuf,
			0,
			NULL,
			&changing,
			0,
			0);

		ID3D11Buffer* buffers[] = { chaingBuf };
		devCtx->VSSetConstantBuffers(0, 1, buffers);
		devCtx->GSSetConstantBuffers(0, 1, buffers);
		devCtx->PSSetConstantBuffers(0, 1, buffers);
	}
};
