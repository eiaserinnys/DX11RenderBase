#include "pch.h"
#include "MmdRenderer.h"

#include <DX11Buffer.h>
#include <DX11Shader.h>
#include <DX11InputLayout.h>
#include <DX11StateBlocks.h>
#include <DX11ConstantBufferT.h>

#include <Utility.h>

#include "SceneDescriptor.h"

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
			20000 * sizeof(XMFLOAT3)));

		ind.reset(IDX11Buffer::Create_DynamicIB(
			context->d3d11->g_pd3dDevice,
			sizeof(UINT16),
			50000));
	}

	//--------------------------------------------------------------------------
	void Render(
		const pmd::PmdModel* model,
		const SceneDescriptor& sceneDesc)
	{
		PrepareModel(model);

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

		context->d3d11->immDevCtx->DrawIndexed(model->indices.size() / 3, 0, 0);
	}

	//--------------------------------------------------------------------------
	void MatMul(float dst[16], float m0[16], float m1[16]) 
	{
		for (int i = 0; i < 4; ++i) 
		{
			for (int j = 0; j < 4; ++j) 
			{
				dst[4 * i + j] = 0;
				for (int k = 0; k < 4; ++k) 
				{
					dst[4 * i + j] += m0[4 * k + j] * m1[4 * i + k];
				}
			}
		}
	}

	void MatVMul(XMFLOAT3& w, const XMMATRIX& m_, const XMFLOAT3& v) 
	{
		const float* m = (const float*) &m_;
		w.x = m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12];
		w.y = m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13];
		w.z = m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14];
	}

	//--------------------------------------------------------------------------
	void PrepareModel(const pmd::PmdModel* model)
	{
		// 로컬 트랜스폼을 빌드
		vector<XMMATRIX> localTx;
		localTx.resize(model->bones.size());

		for (size_t i = 0; i < model->bones.size(); ++i)
		{
			auto& bone = model->bones[i];

			localTx[i] = XMMatrixIdentity();

			//if (bone.motions.empty()) 
			{
				//bone.rotation[0] = 0.0;
				//bone.rotation[1] = 0.0;
				//bone.rotation[2] = 0.0;
				//bone.rotation[3] = 1.0;

				if (bone.parent_bone_index == 0xffff) 
				{
					localTx[i].r[3].m128_f32[0] = bone.bone_head_pos[0];
					localTx[i].r[3].m128_f32[1] = bone.bone_head_pos[1];
					localTx[i].r[3].m128_f32[2] = bone.bone_head_pos[2];
				}
				else 
				{
					auto& parent = model->bones[bone.parent_bone_index];

					localTx[i].r[3].m128_f32[0] = bone.bone_head_pos[0] - parent.bone_head_pos[0];
					localTx[i].r[3].m128_f32[1] = bone.bone_head_pos[1] - parent.bone_head_pos[1];
					localTx[i].r[3].m128_f32[2] = bone.bone_head_pos[2] - parent.bone_head_pos[2];
				}
			}
		}

		// 월드 트랜스폼을 빌드
		vector<XMMATRIX> worldTx;
		worldTx.resize(model->bones.size());

		for (size_t i = 0; i < model->bones.size(); ++i)
		{
			auto& bone = model->bones[i];

			if (bone.parent_bone_index == 0xFFFF)
			{
				worldTx[i] = localTx[i];
			}
			else 
			{
				MatMul(
					(float*)&worldTx[i],
					(float*)&worldTx[bone.parent_bone_index],
					(float*)&localTx[i]);
			}
		}

		// 버텍스를 트랜스폼
		vector<XMFLOAT3> vertices;
		vertices.resize(model->vertices.size());

		for (int i = 0; i < model->vertices.size(); i++) 
		{
			auto& pv = model->vertices[i];
			XMFLOAT3 p0, p1;
			p0.x = pv.position[0];
			p0.y = pv.position[1];
			p0.z = pv.position[2];
			p1.x = pv.position[0];
			p1.y = pv.position[1];
			p1.z = pv.position[2];
			unsigned short b0 = pv.bone_index[0];
			unsigned short b1 = pv.bone_index[1];

			auto& m0 = worldTx[b0];
			auto& m1 = worldTx[b1];

			XMFLOAT3 v0;
			XMFLOAT3 v1;
			XMFLOAT3 v;

			// Bone matrix is defined in absolute coordinates.
			// Pass a vertex in relative coordinate to bone matrix.
			p0.x -= model->bones[b0].bone_head_pos[0];
			p0.y -= model->bones[b0].bone_head_pos[1];
			p0.z -= model->bones[b0].bone_head_pos[2];
			p1.x -= model->bones[b1].bone_head_pos[0];
			p1.y -= model->bones[b1].bone_head_pos[1];
			p1.z -= model->bones[b1].bone_head_pos[2];

			MatVMul(v0, m0, p0);
			MatVMul(v1, m1, p1);
			float w = pv.bone_weight / 100.0f;
			v.x = w * v0.x + (1.0f - w) * v1.x;
			v.y = w * v0.y + (1.0f - w) * v1.y;
			v.z = w * v0.z + (1.0f - w) * v1.z;

			vertices[i] = v;
		}

		pos->UpdateDiscard(
			context->d3d11->immDevCtx,
			&vertices[0],
			vertices.size());

		ind->UpdateDiscard(
			context->d3d11->immDevCtx,
			&model->indices[0],
			model->indices.size());
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
