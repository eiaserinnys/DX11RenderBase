#include <DirectXMath.h>

#include "DX11Device.h"
#include "DX11ConstantBufferT.h"

//------------------------------------------------------------------------------
struct SimpleConstantBody
{
	DirectX::XMMATRIX mWorld;
	DirectX::XMMATRIX viewProj;
	DirectX::XMFLOAT4 eyePos;
};

//------------------------------------------------------------------------------
struct SimpleConstant
	: public DX11ConstantBufferT<SimpleConstantBody>
{
	typedef DX11ConstantBufferT<SimpleConstantBody> Parent;

	SimpleConstant(
		ID3D11Device* d3dDev,
		ID3D11DeviceContext* devCtx)
		: Parent(d3dDev, devCtx) {}

	void Update(
		ID3D11DeviceContext* devCtx,
		const DirectX::XMMATRIX& world, 
		const DirectX::XMMATRIX& view, 
		const DirectX::XMMATRIX& proj,
		const DirectX::XMFLOAT4& eye)
	{
		cbChangesEveryFrameMem.mWorld = world;
		cbChangesEveryFrameMem.viewProj = DirectX::XMMatrixMultiply(proj, view);
		cbChangesEveryFrameMem.eyePos = eye;

		UpdateInternal(devCtx);
	}
};
