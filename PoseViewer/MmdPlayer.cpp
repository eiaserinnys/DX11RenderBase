#include "pch.h"
#include "MmdPlayer.h"

#include <DirectXMath.h>

#include <WindowsUtility.h>
#include <Utility.h>

#include "Pmd.h"
#include "Vmd.h"

using namespace std;
using namespace DirectX;

//------------------------------------------------------------------------------
class MmdPlayer : public IMmdPlayer {
public:
	unique_ptr<pmd::PmdModel> model;
	unique_ptr<vmd::VmdMotion> motion;

	vector<XMFLOAT3> vertices;
	vector<uint16_t> indices;

	const XMFLOAT3* GetVertices() const { return vertices.empty() ? nullptr : &vertices[0]; }
	int GetVerticesCount() const { return (int) vertices.size(); }

	const uint16_t* GetIndices() const { return indices.empty() ? nullptr : &indices[0]; }
	int GetIndicesCount() const { return (int)indices.size(); }

	void Load()
	{
		// PMD를 로드해본다
		model = pmd::PmdModel::LoadFromFile("MMD/md_m.pmd");

		for (size_t i = 0; i < model->bones.size(); ++i)
		{
			auto& b = model->bones[i];

			WindowsUtility::Debug(
				L"[%03d] %s (%d)",
				i,
				vmd::sjis2utf8(b.name).c_str(),
				b.bone_type);

			if (b.parent_bone_index != 0xffff)
			{
				WindowsUtility::Debug(
					L" -> [%03d] %s\n",
					b.parent_bone_index,
					vmd::sjis2utf8(model->bones[b.parent_bone_index].name).c_str());
			}
			else
			{
				WindowsUtility::Debug(L"\n");
			}
		}

		// VMD를 로드해본다
		std::ifstream is;
		is.open("MMD/Happy Hands Meme.vmd", std::ios::in | std::ios::binary);

		if (is.is_open())
		{
			motion = vmd::VmdMotion::LoadFromStream(&is);
		}

		for (auto frame : motion->bone_frames)
		{
			WindowsUtility::Debug(
				L"%d %s %f,%f,%f %f,%f,%f,%f\n",
				frame.frame,
				frame.wname.c_str(),
				frame.position[0],
				frame.position[1],
				frame.position[2],
				frame.orientation[0],
				frame.orientation[1],
				frame.orientation[2],
				frame.orientation[3]);
		}
	}

	void Update()
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

			vertices[i] = v / 20.0f;
		}

		indices.resize(model->indices.size());
		if (!indices.empty())
		{
			memcpy(&indices[0], &model->indices[0], model->indices.size() * sizeof(uint16_t));
		}
	}

	static void MatMul(float dst[16], float m0[16], float m1[16])
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

	static void MatVMul(XMFLOAT3& w, const XMMATRIX& m_, const XMFLOAT3& v)
	{
		const float* m = (const float*)&m_;
		w.x = m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12];
		w.y = m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13];
		w.z = m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14];
	}
};

IMmdPlayer::~IMmdPlayer()
{}

IMmdPlayer* IMmdPlayer::Create()
{ return new MmdPlayer; }