#include "pch.h"
#include "MmdRenderer.h"

#include <DX11Buffer.h>
#include <DX11Shader.h>
#include <DX11InputLayout.h>
#include <DX11StateBlocks.h>
#include <DX11ConstantBufferT.h>

#include <Utility.h>

#include "SceneDescriptor.h"
#include "MmdPlayer.h"

using namespace std;
using namespace DirectX;

//------------------------------------------------------------------------------
struct MmdConstant
{
	XMMATRIX mWorld;
	XMMATRIX viewProj;
	XMFLOAT4 eyePos;
};

//------------------------------------------------------------------------------
struct MmdEffectConstants
	: public DX11ConstantBufferT<MmdConstant>
{
	typedef DX11ConstantBufferT<MmdConstant> Parent;

	MmdEffectConstants(
		ID3D11Device* d3dDev,
		ID3D11DeviceContext* devCtx)
		: Parent(d3dDev, devCtx) {}

	void Update(
		ID3D11DeviceContext* devCtx,
		const SceneDescriptor& sceneDesc)
	{
		cbChangesEveryFrameMem.mWorld = sceneDesc.world;
		cbChangesEveryFrameMem.viewProj = XMMatrixMultiply(sceneDesc.proj, sceneDesc.view);
		cbChangesEveryFrameMem.eyePos = sceneDesc.eye;

		UpdateInternal(devCtx);
	}
};

//------------------------------------------------------------------------------
class MmdRenderer : public IMmdRenderer {
public:
	int primitiveCount;

	//--------------------------------------------------------------------------
	MmdRenderer(RenderContext* context)
		: context(context)
	{
		ID3D11Device* d3dDev = context->d3d11->g_pd3dDevice;
		ID3D11DeviceContext* devCtx = context->d3d11->immDevCtx;

		context->vs->Load(fxFileName);
		context->ps->Load(fxFileName);

		// Define the input layout
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = ARRAYSIZE(layout);

		auto vs = context->vs->Find(fxFileName);

		vertexLayout.reset(IDX11InputLayout::Create(
			d3dDev, layout, numElements, vs->pVSBlob));

		{
			D3D11_RASTERIZER_DESC rsDesc;
			IDX11RasterizerState::DefaultDesc(rsDesc);
			rsDesc.FillMode = D3D11_FILL_WIREFRAME;
			rasterState.reset(IDX11RasterizerState::Create(d3dDev, rsDesc));

			D3D11_DEPTH_STENCIL_DESC dsDesc;
			IDX11DepthStencilState::DefaultDesc(dsDesc);
			depthState.reset(IDX11DepthStencilState::Create(d3dDev, dsDesc));
		}

		{
			D3D11_DEPTH_STENCIL_DESC dsDesc;
			IDX11DepthStencilState::DefaultDesc(dsDesc);
			dsDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
			depthStateWire.reset(IDX11DepthStencilState::Create(d3dDev, dsDesc));
		}

		{
			blendState.reset(IDX11BlendState::Create_AlphaBlend(d3dDev));
		}

		constants.reset(new MmdEffectConstants(d3dDev, devCtx));

		pos.reset(IDX11Buffer::Create_DynamicVB(
			context->d3d11->g_pd3dDevice,
			sizeof(XMFLOAT3),
			40000 * sizeof(XMFLOAT3)));

		ind.reset(IDX11Buffer::Create_DynamicIB(
			context->d3d11->g_pd3dDevice,
			sizeof(UINT32),
			150000 * sizeof(UINT32)));
	}

	//--------------------------------------------------------------------------
	void Render(
		const IMmdPlayer* player,
		const SceneDescriptor& sceneDesc)
	{
		pos->UpdateDiscard(
			context->d3d11->immDevCtx,
			player->GetVertices(),
			player->GetVerticesCount());

		ind->UpdateDiscard(
			context->d3d11->immDevCtx,
			(void*) player->GetIndices(),
			player->GetIndicesCount());

		context->d3d11->immDevCtx->ClearState();

		context->d3d11->RestoreRenderTarget();

		constants->Update(context->d3d11->immDevCtx, sceneDesc);

		// 버텍스 입력 레이아웃
		vertexLayout->Apply(context->d3d11->immDevCtx);

		// 상수들
		rasterState->Apply(context->d3d11->immDevCtx);
		depthState->Apply(context->d3d11->immDevCtx);
		blendState->Apply(context->d3d11->immDevCtx);

		context->vs->Set(fxFileName);
		context->ps->Set(fxFileName);

		pos->ApplyVB(context->d3d11->immDevCtx, 0, 0);
		ind->ApplyIB(context->d3d11->immDevCtx, 0);
		context->d3d11->immDevCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		context->d3d11->immDevCtx->DrawIndexed(player->GetIndicesCount(), 0, 0);
	}

	//--------------------------------------------------------------------------
	RenderContext* context;

	const wchar_t* fxFileName = L"Shaders/Mmd.fx";

	unique_ptr<IDX11Buffer> pos;
	unique_ptr<IDX11Buffer> ind;

	unique_ptr<MmdEffectConstants> constants;

	unique_ptr<IDX11RasterizerState> rasterState;
	unique_ptr<IDX11DepthStencilState> depthState;
	unique_ptr<IDX11BlendState> blendState;

	unique_ptr<IDX11RasterizerState> rasterStateWire;
	unique_ptr<IDX11DepthStencilState> depthStateWire;

	unique_ptr<IDX11InputLayout> vertexLayout;
};

IMmdRenderer::~IMmdRenderer()
{}

IMmdRenderer* IMmdRenderer::Create(RenderContext* context)
{
	return new MmdRenderer(context);
}
