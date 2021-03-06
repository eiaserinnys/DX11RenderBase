#include "pch.h"
#include "OpenPoseRenderer.h"

#include <DX11Buffer.h>
#include <DX11Shader.h>
#include <DX11InputLayout.h>
#include <DX11StateBlocks.h>
#include <DX11ConstantBufferT.h>

#include <Utility.h>

#include "Render.h"

#include "SceneDescriptor.h"

using namespace std;
using namespace DirectX;

//------------------------------------------------------------------------------
struct OpenPoseConstant
{
	XMMATRIX mWorld;
	XMMATRIX viewProj;
	XMFLOAT4 eyePos;
};

//------------------------------------------------------------------------------
struct OpenPoseEffectConstants 
	: public DX11ConstantBufferT<OpenPoseConstant>
{
	typedef DX11ConstantBufferT<OpenPoseConstant> Parent;

	OpenPoseEffectConstants(
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
class OpenPoseRenderer : public IOpenPoseRenderer {
public:
	//--------------------------------------------------------------------------
	OpenPoseRenderer(RenderContext* context)
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

		constants.reset(new OpenPoseEffectConstants(d3dDev, devCtx));
	
		pos.reset(IDX11Buffer::Create_DynamicVB(
			context->d3d11->g_pd3dDevice,
			sizeof(XMFLOAT3),
			200 * sizeof(XMFLOAT3)));

		col.reset(IDX11Buffer::Create_DynamicVB(
			context->d3d11->g_pd3dDevice,
			sizeof(DWORD),
			200 * sizeof(DWORD)));

		ind.reset(IDX11Buffer::Create_DynamicIB(
			context->d3d11->g_pd3dDevice,
			sizeof(UINT16),
			600 * sizeof(UINT16)));
	}

	//--------------------------------------------------------------------------
	void Render(
		DX11Render* render,
		const OpenPose::Frame& frame,
		const SceneDescriptor& sceneDesc)
	{
		for (int i = 0; i < COUNT_OF(frame.pos); ++i)
		{
			//render->RenderText(TextToRender(
			//	frame.pos[i],
			//	Utility::FormatW(L"%d", i),
			//	XMFLOAT4(0, 0, 0, 1)));
		}

		// https://github.com/CMU-Perceptual-Computing-Lab/openpose/blob/master/doc/output.md
		UINT16 indData[] =
		{
			0, 1,

			1, 2,
			2, 3,
			3, 4,

			1, 5,
			5, 6,
			6, 7,

			1, 8,
			8, 9,
			9, 10,

			1, 11,
			11, 12,
			12, 13,

			0, 14,
			0, 15,
			14, 16,
			15, 17,
		};

		vector<UINT16> ind_;
		vector<DWORD> col_;
		ind_.resize(COUNT_OF(indData));
		memcpy(&ind_[0], indData, sizeof(indData[0]) * COUNT_OF(indData));

		vector<XMFLOAT3> pos_;
		for (int i = 0; i < COUNT_OF(frame.pos); ++i)
		{
			pos_.push_back(frame.pos[i]);
			col_.push_back(0xff000000);
		}

		auto& renderTx = [&](const XMMATRIX& tx)
		{
			XMFLOAT3 pivot(
				tx.r[3].m128_f32[0],
				tx.r[3].m128_f32[1],
				tx.r[3].m128_f32[2] + 0.01f);

			XMFLOAT3 x(
				tx.r[0].m128_f32[0],
				tx.r[0].m128_f32[1],
				tx.r[0].m128_f32[2]);
			XMFLOAT3 y(
				tx.r[1].m128_f32[0],
				tx.r[1].m128_f32[1],
				tx.r[1].m128_f32[2]);
			XMFLOAT3 z(
				tx.r[2].m128_f32[0],
				tx.r[2].m128_f32[1],
				tx.r[2].m128_f32[2]);

			float scale = 0.01f;

			ind_.push_back(pos_.size());
			ind_.push_back(pos_.size() + 1);
			pos_.push_back(pivot);
			pos_.push_back(pivot + x * scale);
			col_.push_back(0xff0000ff);
			col_.push_back(0xff0000ff);

			ind_.push_back(pos_.size());
			ind_.push_back(pos_.size() + 1);
			pos_.push_back(pivot);
			pos_.push_back(pivot + y * scale);
			col_.push_back(0xff00ff00);
			col_.push_back(0xff00ff00);

			ind_.push_back(pos_.size());
			ind_.push_back(pos_.size() + 1);
			pos_.push_back(pivot);
			pos_.push_back(pivot + z * scale);
			col_.push_back(0xffff0000);
			col_.push_back(0xffff0000);
		};

		renderTx(frame.comTx);

		for (int i = 0; i < COUNT_OF(frame.pos); ++i)
		{
			renderTx(frame.worldTx[i]);
		}

		pos->UpdateDiscard(
			context->d3d11->immDevCtx, 
			&pos_[0],
			pos_.size());

		col->UpdateDiscard(
			context->d3d11->immDevCtx,
			&col_[0],
			col_.size());

		ind->UpdateDiscard(
			context->d3d11->immDevCtx,
			&ind_[0],
			ind_.size());

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
		col->ApplyVB(context->d3d11->immDevCtx, 1, 0);
		ind->ApplyIB(context->d3d11->immDevCtx, 0);
		context->d3d11->immDevCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

		context->d3d11->immDevCtx->DrawIndexed(ind_.size(), 0, 0);
	}

	//--------------------------------------------------------------------------
	RenderContext* context;

	const wchar_t* fxFileName = L"Shaders/OpenPose.fx";

	unique_ptr<IDX11Buffer> pos;
	unique_ptr<IDX11Buffer> col;
	unique_ptr<IDX11Buffer> ind;

	unique_ptr<OpenPoseEffectConstants> constants;

	unique_ptr<IDX11RasterizerState> rasterState;
	unique_ptr<IDX11DepthStencilState> depthState;
	unique_ptr<IDX11BlendState> blendState;

	unique_ptr<IDX11RasterizerState> rasterStateWire;
	unique_ptr<IDX11DepthStencilState> depthStateWire;

	unique_ptr<IDX11InputLayout> vertexLayout;
};

IOpenPoseRenderer::~IOpenPoseRenderer()
{}

IOpenPoseRenderer* IOpenPoseRenderer::Create(RenderContext* context)
{ return new OpenPoseRenderer(context); }
