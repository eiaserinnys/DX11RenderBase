#pragma once

#include <d3d11.h>

class IDX11Buffer {
public:
	virtual ~IDX11Buffer();

	virtual HRESULT CreateShaderResourceView(ID3D11Device* dev) = 0;
	virtual IDX11Buffer* CreateStaging(ID3D11Device* dev) = 0;
	virtual HRESULT CreateUnorderedAccessView(ID3D11Device* dev) = 0;

	virtual void ApplyVB(ID3D11DeviceContext* devCtx, UINT slot, UINT offset) = 0;
	virtual void ApplyIB(ID3D11DeviceContext* devCtx, UINT offset) = 0;

	virtual void SetVSResource(ID3D11DeviceContext* devCtx, UINT slot) = 0;
	virtual void SetCSResource(ID3D11DeviceContext* devCtx, UINT slot) = 0;
	virtual void CSSetUnorderedAccessViews(ID3D11DeviceContext* devCtx, UINT slot) = 0;
	virtual void OMSetRenderTargetsAndUnorderedAccessViews(ID3D11DeviceContext* devCtx, UINT slot) = 0;

	virtual HRESULT MapWriteDiscard(ID3D11DeviceContext* devCtx, D3D11_MAPPED_SUBRESOURCE& mapped) = 0;
	virtual HRESULT MapRead(ID3D11DeviceContext* devCtx, D3D11_MAPPED_SUBRESOURCE& mapped) = 0;
	virtual void Unmap(ID3D11DeviceContext* devCtx) = 0;

	virtual HRESULT UpdateDiscard(ID3D11DeviceContext* devCtx, const void* data) = 0;
	virtual HRESULT UpdateDiscard(ID3D11DeviceContext* devCtx, const void* data, UINT count) = 0;

	virtual void Copy(ID3D11DeviceContext* devCtx, IDX11Buffer* buffer) = 0;

	virtual ID3D11Buffer* Get() = 0;

	virtual UINT GetCount() = 0;
	virtual UINT GetStride() = 0;

public:
	static IDX11Buffer* Create_DefaultVB(ID3D11Device* dev, UINT stride, UINT count, void* data);
	static IDX11Buffer* Create_DynamicVB(ID3D11Device* dev, UINT stride, UINT count);

	static IDX11Buffer* Create_DefaultIB(ID3D11Device* dev, UINT stride, UINT count, void* data);
	static IDX11Buffer* Create_DynamicIB(ID3D11Device* dev, UINT stride, UINT count);

	static IDX11Buffer* Create_ImmutableStructured(ID3D11Device* dev, UINT stride, UINT count, void* data);
	static IDX11Buffer* Create_DynamicStructured(ID3D11Device* dev, UINT stride, UINT count);
	static IDX11Buffer* Create_UnorderedStructured(ID3D11Device* dev, UINT stride, UINT count);
};