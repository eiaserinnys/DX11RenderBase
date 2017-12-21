#include "pch.h"
#include "MmdBoneRenderer.h"

#include <set>
#include <vector>

#include <DX11Buffer.h>
#include <DX11Shader.h>
#include <DX11InputLayout.h>
#include <DX11StateBlocks.h>
#include <DX11ConstantBufferT.h>

#include <Utility.h>

#include "SceneDescriptor.h"
#include "MmdPlayer.h"
#include "Render.h"

using namespace std;
using namespace DirectX;

//------------------------------------------------------------------------------
struct MmdBoneConstant
{
	XMMATRIX mWorld;
	XMMATRIX viewProj;
	XMFLOAT4 eyePos;
};

//------------------------------------------------------------------------------
struct MmdBoneEffectConstants
	: public DX11ConstantBufferT<MmdBoneConstant>
{
	typedef DX11ConstantBufferT<MmdBoneConstant> Parent;

	MmdBoneEffectConstants(
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
class MmdBoneRenderer : public IMmdBoneRenderer {
public:
	//--------------------------------------------------------------------------
	MmdBoneRenderer(RenderContext* context)
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
			{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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

		constants.reset(new MmdBoneEffectConstants(d3dDev, devCtx));

		pos.reset(IDX11Buffer::Create_DynamicVB(
			context->d3d11->g_pd3dDevice,
			sizeof(XMFLOAT3),
			10000 * sizeof(XMFLOAT3)));

		col.reset(IDX11Buffer::Create_DynamicVB(
			context->d3d11->g_pd3dDevice,
			sizeof(DWORD),
			10000 * sizeof(DWORD)));

		ind.reset(IDX11Buffer::Create_DynamicIB(
			context->d3d11->g_pd3dDevice,
			sizeof(UINT16),
			10000 * sizeof(UINT16)));
	}

	//--------------------------------------------------------------------------
	void Render(
		DX11Render* render, 
		const IMmdPlayer* player,
		const SceneDescriptor& sceneDesc)
	{
		auto tx = player->GetWorldTransform();
		auto txCount = player->GetWorldTransformCount();

		vector<XMFLOAT3> pos;
		vector<DWORD> col;
		vector<uint16_t> ind;

		float scale = 0.05f;

		set<int> toRender;

		int toRenderList[] =
		{
			1,		// 全ての親
			2,		// センター
			//3,		// グルーブ
			4,		// 腰
			11,		// 上半身
			12,		// 上半身2
			14,		// 首
			16,		// 右肩P
			17,		// 右肩
			18,		// 右肩C
			19,		// 右腕
		};
		
		toRender.insert(toRenderList, toRenderList + COUNT_OF(toRenderList));

		for (int i = 0; i < txCount; ++i)
		{
			if (toRender.find(i) == toRender.end()) { continue; }

			XMFLOAT3 pivot(tx[i].r[3].m128_f32[0], tx[i].r[3].m128_f32[1], tx[i].r[3].m128_f32[2]);
			XMFLOAT3 x(tx[i].r[0].m128_f32[0], tx[i].r[0].m128_f32[1], tx[i].r[0].m128_f32[2]);
			XMFLOAT3 y(tx[i].r[1].m128_f32[0], tx[i].r[1].m128_f32[1], tx[i].r[1].m128_f32[2]);
			XMFLOAT3 z(tx[i].r[2].m128_f32[0], tx[i].r[2].m128_f32[1], tx[i].r[2].m128_f32[2]);

			pivot = pivot / 20;

			render->RenderText(
				TextToRender(pivot, Utility::FormatW(L"%d", i), XMFLOAT4(1, 1, 1, 1)));

			ind.push_back((uint16_t)pos.size());
			ind.push_back((uint16_t)pos.size() + 1);
			pos.push_back(pivot);
			pos.push_back(pivot + x * scale);
			col.push_back(0xffff0000);
			col.push_back(0xffff0000);

			ind.push_back((uint16_t)pos.size());
			ind.push_back((uint16_t)pos.size() + 1);
			pos.push_back(pivot);
			pos.push_back(pivot + y * scale);
			col.push_back(0xff00ff00);
			col.push_back(0xff00ff00);

			ind.push_back((uint16_t)pos.size());
			ind.push_back((uint16_t)pos.size() + 1);
			pos.push_back(pivot);
			pos.push_back(pivot + z * scale);
			col.push_back(0xff0000ff);
			col.push_back(0xff0000ff);
		}

		this->pos->UpdateDiscard(
			context->d3d11->immDevCtx,
			&pos[0],
			pos.size());

		this->col->UpdateDiscard(
			context->d3d11->immDevCtx,
			&col[0],
			col.size());

		this->ind->UpdateDiscard(
			context->d3d11->immDevCtx,
			&ind[0],
			ind.size());

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

		this->pos->ApplyVB(context->d3d11->immDevCtx, 0, 0);
		this->col->ApplyVB(context->d3d11->immDevCtx, 1, 0);
		this->ind->ApplyIB(context->d3d11->immDevCtx, 0);
		context->d3d11->immDevCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

		context->d3d11->immDevCtx->DrawIndexed(ind.size(), 0, 0);
	}

	//--------------------------------------------------------------------------
	RenderContext* context;

	const wchar_t* fxFileName = L"Shaders/MmdBone.fx";

	unique_ptr<IDX11Buffer> pos;
	unique_ptr<IDX11Buffer> col;
	unique_ptr<IDX11Buffer> ind;

	unique_ptr<MmdBoneEffectConstants> constants;

	unique_ptr<IDX11RasterizerState> rasterState;
	unique_ptr<IDX11DepthStencilState> depthState;
	unique_ptr<IDX11BlendState> blendState;

	unique_ptr<IDX11RasterizerState> rasterStateWire;
	unique_ptr<IDX11DepthStencilState> depthStateWire;

	unique_ptr<IDX11InputLayout> vertexLayout;
};

IMmdBoneRenderer::~IMmdBoneRenderer()
{}

IMmdBoneRenderer* IMmdBoneRenderer::Create(RenderContext* context)
{
	return new MmdBoneRenderer(context);
}
