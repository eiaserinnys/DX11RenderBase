#include <DirectXMath.h>

#include "DX11Device.h"
#include "DX11ConstantBufferT.h"

//------------------------------------------------------------------------------
struct SimpleConstantBody
{
	DirectX::XMMATRIX worldViewProj;
	DirectX::XMMATRIX invWorldViewT;
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
		const DirectX::XMMATRIX& wvp, 
		const DirectX::XMMATRIX& invWvT, 
		const DirectX::XMFLOAT4& eye)
	{
		using namespace DirectX;

		changing.worldViewProj = wvp;
		changing.invWorldViewT = invWvT;
		changing.eyePos = eye;

		UpdateInternal(devCtx);
	}
};
