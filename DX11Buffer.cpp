#include "pch.h"
#include "DX11Buffer.h"

class DX11Buffer : public IDX11Buffer {
public:
	DX11Buffer(
		ID3D11Device* dev,
		const D3D11_BUFFER_DESC& bd,
		UINT stride,
		UINT count,
		void* data)
		: buffer(NULL)
		, srv(NULL)
		, uav(NULL)
		, stride(stride)
		, count(count)
	{
		HRESULT hr;

		if (data != NULL)
		{
			D3D11_SUBRESOURCE_DATA initData;
			ZeroMemory(&initData, sizeof(initData));
			initData.pSysMem = data;
			if (FAILED(hr = dev->CreateBuffer(&bd, &initData, &buffer))) { throw hr; }
		}
		else
		{
			if (FAILED(hr = dev->CreateBuffer(&bd, NULL, &buffer))) { throw hr; }
		}
	}
	~DX11Buffer()
	{
		if (buffer != NULL) { buffer->Release(); buffer = NULL; }
		if (srv != NULL) { srv->Release(); srv = NULL; }
		if (uav != NULL) { uav->Release(); uav = NULL; }
	}

	virtual HRESULT CreateShaderResourceView(ID3D11Device* dev)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC sbSRVDesc;
		sbSRVDesc.Buffer.ElementOffset = 0;
		sbSRVDesc.Buffer.ElementWidth = stride;
		sbSRVDesc.Buffer.FirstElement = 0;
		sbSRVDesc.Buffer.NumElements = count;
		sbSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
		sbSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		return dev->CreateShaderResourceView(buffer, &sbSRVDesc, &srv);
	}

	virtual IDX11Buffer* CreateStaging(ID3D11Device* dev)
	{
		D3D11_BUFFER_DESC stagingBufferDesc;
		stagingBufferDesc.BindFlags = 0;
		stagingBufferDesc.Usage = D3D11_USAGE_STAGING;
		stagingBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		stagingBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		stagingBufferDesc.StructureByteStride = stride;
		stagingBufferDesc.ByteWidth = stride * count;
		return new DX11Buffer(dev, stagingBufferDesc, stride, count, NULL);
	}

	virtual HRESULT CreateUnorderedAccessView(ID3D11Device* dev)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC sbUAVDesc;
		sbUAVDesc.Buffer.FirstElement = 0;
		sbUAVDesc.Buffer.Flags = 0;
		sbUAVDesc.Buffer.NumElements = count;
		sbUAVDesc.Format = DXGI_FORMAT_UNKNOWN;
		sbUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		return dev->CreateUnorderedAccessView(buffer, &sbUAVDesc, &uav);
	}

	virtual void ApplyVB(ID3D11DeviceContext* devCtx, UINT slot, UINT offset)
	{
		UINT stride_ = stride;
		devCtx->IASetVertexBuffers(slot, 1, &buffer, &stride_, &offset);
	}

	virtual void ApplyIB(ID3D11DeviceContext* devCtx, UINT offset)
	{
		if (stride == sizeof(UINT))
		{
			devCtx->IASetIndexBuffer(buffer, DXGI_FORMAT_R32_UINT, offset);
		}
		else if (stride == sizeof(WORD))
		{
			devCtx->IASetIndexBuffer(buffer, DXGI_FORMAT_R16_UINT, offset);
		}
		else
		{
			// ??????????????
		}
	}

	virtual void SetVSResource(ID3D11DeviceContext* devCtx, UINT slot)
	{
		devCtx->VSSetShaderResources(slot, 1, &srv);
	}

	virtual void SetCSResource(ID3D11DeviceContext* devCtx, UINT slot)
	{
		devCtx->CSSetShaderResources(slot, 1, &srv);
	}

	virtual void CSSetUnorderedAccessViews(ID3D11DeviceContext* devCtx, UINT slot)
	{
		UINT initCounts = 0;
		devCtx->CSSetUnorderedAccessViews(slot, 1, &uav, &initCounts);
	}

	virtual void OMSetRenderTargetsAndUnorderedAccessViews(ID3D11DeviceContext* devCtx, UINT slot)
	{
		UINT initCounts = 0;
		devCtx->OMSetRenderTargetsAndUnorderedAccessViews(
			D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, NULL, NULL,
			slot, 1, &uav, &initCounts);
	}

	virtual HRESULT MapWriteDiscard(ID3D11DeviceContext* devCtx, D3D11_MAPPED_SUBRESOURCE& mapped)
	{
		return devCtx->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	}

	virtual HRESULT MapRead(ID3D11DeviceContext* devCtx, D3D11_MAPPED_SUBRESOURCE& mapped)
	{
		return devCtx->Map(buffer, 0, D3D11_MAP_READ, 0, &mapped);
	}

	virtual void Unmap(ID3D11DeviceContext* devCtx)
	{
		devCtx->Unmap(buffer, 0);
	}

	virtual HRESULT UpdateDiscard(ID3D11DeviceContext* devCtx, const void* data)
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		HRESULT hr = MapWriteDiscard(devCtx, mapped);
		if (SUCCEEDED(hr))
		{
			memcpy(mapped.pData, data, stride * count);
			Unmap(devCtx);
		}
		return hr;
	}

	virtual HRESULT UpdateDiscard(ID3D11DeviceContext* devCtx, const void* data, UINT toUpdate)
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		HRESULT hr = MapWriteDiscard(devCtx, mapped);
		if (SUCCEEDED(hr))
		{
			memcpy(mapped.pData, data, stride * toUpdate);
			Unmap(devCtx);
		}
		return hr;
	}

	virtual void Copy(ID3D11DeviceContext* devCtx, IDX11Buffer* rhs)
	{
		devCtx->CopyResource(buffer, rhs->Get());
	}

	virtual ID3D11Buffer* Get() { return buffer; }

	virtual UINT GetCount() { return count; }
	virtual UINT GetStride() { return stride; }

private:
	ID3D11Buffer* buffer;
	ID3D11ShaderResourceView* srv;
	ID3D11UnorderedAccessView* uav;
	UINT stride, count;
};

IDX11Buffer::~IDX11Buffer() {}

IDX11Buffer* IDX11Buffer::Create_DynamicVB(
	ID3D11Device* dev, UINT stride, UINT count)
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = stride * count;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	return new DX11Buffer(dev, bd, stride, count, NULL);
}

IDX11Buffer* IDX11Buffer::Create_DefaultVB(
	ID3D11Device* dev, UINT stride, UINT count, void* data)
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = stride * count;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	return new DX11Buffer(dev, bd, stride, count, data);
}

IDX11Buffer* IDX11Buffer::Create_DefaultIB(
	ID3D11Device* dev, UINT stride, UINT count, void* data)
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = stride * count;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	return new DX11Buffer(dev, bd, stride, count, data);
}

IDX11Buffer* IDX11Buffer::Create_DynamicIB(
	ID3D11Device* dev, UINT stride, UINT count)
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = stride * count;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	return new DX11Buffer(dev, bd, stride, count, nullptr);
}

IDX11Buffer* IDX11Buffer::Create_ImmutableStructured(
	ID3D11Device* dev, UINT stride, UINT count, void* data)
{
	D3D11_BUFFER_DESC sbDesc;
	sbDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	sbDesc.CPUAccessFlags = 0;
	sbDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	sbDesc.StructureByteStride = stride;
	sbDesc.ByteWidth = stride * count;
	sbDesc.Usage = D3D11_USAGE_IMMUTABLE;
	return new DX11Buffer(dev, sbDesc, stride, count, data);
}

IDX11Buffer* IDX11Buffer::Create_DynamicStructured(
	ID3D11Device* dev, UINT stride, UINT count)
{
	D3D11_BUFFER_DESC sbDesc;
	sbDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	sbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	sbDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	sbDesc.StructureByteStride = stride;
	sbDesc.ByteWidth = stride * count;
	sbDesc.Usage = D3D11_USAGE_DYNAMIC;
	return new DX11Buffer(dev, sbDesc, stride, count, NULL);
}

IDX11Buffer* IDX11Buffer::Create_UnorderedStructured(
	ID3D11Device* dev, UINT stride, UINT count)
{
	D3D11_BUFFER_DESC sbDesc;
	sbDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	sbDesc.CPUAccessFlags = 0;
	sbDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	sbDesc.StructureByteStride = stride;
	sbDesc.ByteWidth = stride * count;
	sbDesc.Usage = D3D11_USAGE_DEFAULT;
	return new DX11Buffer(dev, sbDesc, stride, count, NULL);
}