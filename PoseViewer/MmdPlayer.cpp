#include "pch.h"
#include "MmdPlayer.h"

#include <map>
#include <algorithm>

#include <DirectXMath.h>

#include <WindowsUtility.h>
#include <Utility.h>

#include "Pmd.h"
#include "Vmd.h"
#include "Pmx.h"

#define USE_PMX 1

using namespace std;
using namespace DirectX;

//------------------------------------------------------------------------------
class MmdPlayer : public IMmdPlayer {
public:
	//unique_ptr<pmd::PmdModel> model;
	unique_ptr<pmx::PmxModel> model;
	unique_ptr<vmd::VmdMotion> motion;

	vector<XMFLOAT3> vertices;
	vector<int> indices;

	vector<vector<vmd::VmdBoneFrame>*> frames;

	vector<XMMATRIX> worldTx;

	int pivotTime = 0;
	int curFrame = 0;
	int lastFrame = 0;

	virtual pmx::PmxModel* GetModel() { return model.get(); }
	virtual vmd::VmdMotion* GetMotion() { return motion.get(); }

	const XMFLOAT3* GetVertices() const { return vertices.empty() ? nullptr : &vertices[0]; }
	int GetVerticesCount() const { return (int) vertices.size(); }

	const int* GetIndices() const { return indices.empty() ? nullptr : &indices[0]; }
	int GetIndicesCount() const { return (int)indices.size(); }

	virtual const DirectX::XMMATRIX* GetWorldTransform() const { return worldTx.empty() ? nullptr : &worldTx[0]; }
	virtual int GetWorldTransformCount() const { return worldTx.size(); }

	~MmdPlayer()
	{
		for (auto& f : frames)
		{
			if (f != nullptr)
			{
				delete f;
			}
		}

		frames.clear();
	}

	void Load()
	{
#if !USE_PMX
		LoadPmd();
#else
		LoadPmx();
#endif

		LoadVmd();

		LinkAnimation();
	}

#if !USE_PMX
	void LoadPmd()
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
	}
#else
	void LoadPmx()
	{
		model.reset(new pmx::PmxModel);

		std::ifstream is;
		is.open("MMD/Tda Miku DressB.pmx", std::ios::in | std::ios::binary);

		if (is.is_open())
		{
			model->Read(&is);

			for (size_t i = 0; i < model->bone_count; ++i)
			{
				auto& b = model->bones[i];

				WindowsUtility::Debug(
					L"[%03d] %s (%f,%f,%f)",
					i,
					b.bone_name.c_str(),
					b.position[0],
					b.position[1],
					b.position[2]);

				if (b.parent_index >= 0)
				{
					WindowsUtility::Debug(
						L" -> [%03d] %s\n",
						b.parent_index,
						model->bones[b.parent_index].bone_name.c_str());
				}
				else
				{
					WindowsUtility::Debug(L"\n");
				}
			}
		}
	}
#endif

	void LoadVmd()
	{
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

	static bool VMDMotionSorter(
		const vmd::VmdBoneFrame& m1, 
		const vmd::VmdBoneFrame& m2) 
	{
		return m1.frame < m2.frame;
	}

	void LinkAnimation()
	{
		// 이름 리스트를 만든다
#if USE_PMX
		map<wstring, int> modelBones;

		for (int i = 0; i < model->bone_count; ++i)
		{
			modelBones.insert(make_pair(model->bones[i].bone_name, i));
		}

		// 애니메이션에서 매칭되는 본에 프레임을 누적한다
		frames.resize(model->bone_count);
#else
		map<string, int> modelBones;

		for (int i = 0; i < model->bones.size(); ++i)
		{
			modelBones.insert(make_pair(model->bones[i].name, i));
		}

		// 애니메이션에서 매칭되는 본에 프레임을 누적한다
		frames.resize(model->bones.size());
#endif

		for (auto& f : motion->bone_frames)
		{
			auto it = modelBones.find(vmd::sjis2utf8(f.name));
			if (it != modelBones.end())
			{
				int index = it->second;
				if (frames[index] == nullptr) 
				{
					frames[index] = new vector<vmd::VmdBoneFrame>;
				}

				frames[index]->push_back(f);

				if (f.frame > lastFrame) { lastFrame = f.frame; }
			}
		}

		// 소트한다 -_-
		for (auto& frame : frames)
		{
			if (frame != nullptr)
			{
				// Motion data may not be ordererd in frame number. Sort it here.
				sort(frame->begin(), frame->end(), VMDMotionSorter);
			}
		}
	}

	pair<int, int> FindMotionSegment(
		int frame,
		std::vector<vmd::VmdBoneFrame> &motions) 
	{
		pair<int, int> ms;
		ms.first = 0;
		ms.second = motions.size() - 1;

		if (frame >= motions[ms.second].frame) 
		{
			ms.first = ms.second;
			ms.second = -1;
			return ms;
		}

		while (true) 
		{
			int middle = (ms.first + ms.second) / 2;

			if (middle == ms.first) 
			{
				return ms;
			}

			if (motions[middle].frame == frame) 
			{
				ms.first = middle;
				ms.second = -1;
				return ms;
			}
			else if (motions[middle].frame > frame) 
			{
				ms.second = middle;
			}
			else 
			{
				ms.first = middle;
			}
		}
	}

	static double BezierEval(unsigned char *ip, float t) 
	{
		double xa = ip[0] / 256.0;
		double xb = ip[2] / 256.0;
		double ya = ip[1] / 256.0;
		double yb = ip[3] / 256.0;

		double min = 0;
		double max = 1;

		double ct = t;
		while (true) 
		{
			double x11 = xa * ct;
			double x12 = xa + (xb - xa) * ct;
			double x13 = xb + (1.0 - xb) * ct;

			double x21 = x11 + (x12 - x11) * ct;
			double x22 = x12 + (x13 - x12) * ct;

			double x3 = x21 + (x22 - x21) * ct;

			if (fabs(x3 - t) < 0.0001)
			{
				double y11 = ya * ct;
				double y12 = ya + (yb - ya) * ct;
				double y13 = yb + (1.0 - yb) * ct;

				double y21 = y11 + (y12 - y11) * ct;
				double y22 = y12 + (y13 - y12) * ct;

				double y3 = y21 + (y22 - y21) * ct;

				return y3;
			}
			else if (x3 < t) 
			{
				min = ct;
			}
			else 
			{
				max = ct;
			}
			ct = min * 0.5 + max * 0.5;
		}
	}

	static inline void MyQSlerp(
		XMFLOAT4& p, 
		const XMFLOAT4& q,
		const XMFLOAT4& r,
		double t) 
	{
		double qdotr = q.x * r.x + q.y * r.y + q.z * r.z + q.w * r.w;
		// Clamp to prevent NaN. But is this OK?
		if (qdotr > 1.0)
			qdotr = 1.0;
		if (qdotr < -1.0)
			qdotr = -1.0;

		if (qdotr < 0.0) {
			double theta = acos(-qdotr);

			if (fabs(theta) < 1.0e-10) {
				p = q;
				return;
			}

			double st = sin(theta);
			double inv_st = 1.0 / st;
			double c0 = sin((1.0 - t) * theta) * inv_st;
			double c1 = sin(t * theta) * inv_st;

			p.x = c0 * q.x - c1 * r.x;
			p.y = c0 * q.y - c1 * r.y;
			p.z = c0 * q.z - c1 * r.z;
			p.w = c0 * q.w - c1 * r.w;

		}
		else {

			double theta = acos(qdotr);

			if (fabs(theta) < 1.0e-10) {
				p = q;
				return;
			}

			double st = sin(theta);
			double inv_st = 1.0 / st;
			double c0 = sin((1.0 - t) * theta) * inv_st;
			double c1 = sin(t * theta) * inv_st;

			p.x = c0 * q.x + c1 * r.x;
			p.y = c0 * q.y + c1 * r.y;
			p.z = c0 * q.z + c1 * r.z;
			p.w = c0 * q.w + c1 * r.w;
		}
	}

	static inline void QuatToMatrix(XMMATRIX& m_, const XMFLOAT4 &q)
	{
		// test
		XMVECTOR quat = XMLoadFloat4(&q);
		m_ = XMMatrixRotationQuaternion(quat);
	}

	static void InterpolateMotion(
		XMFLOAT4& rotation, 
		float position[3],
		const std::vector<vmd::VmdBoneFrame> &motions, 
		const pair<int, int>& ms,
		float frame) 
	{
		if (ms.second == -1)
		{
			position[0] = motions[ms.first].position[0];
			position[1] = motions[ms.first].position[1];
			position[2] = motions[ms.first].position[2];
			rotation.x = motions[ms.first].orientation[0];
			rotation.y = motions[ms.first].orientation[1];
			rotation.z = motions[ms.first].orientation[2];
			rotation.w = motions[ms.first].orientation[3];
		}
		else 
		{
			int diff = motions[ms.second].frame - motions[ms.first].frame;
			float a0 = frame - motions[ms.first].frame;
			float ratio = a0 / diff; // [0, 1]

			// Use interpolation parameter

			// http://harigane.at.webry.info/201103/article_1.html

			unsigned char interpX[4];
			unsigned char interpY[4];
			unsigned char interpZ[4];
			unsigned char interpR[4];

			char* interpolation = (char*)(motions[ms.first].interpolation);
			for (int k = 0; k < 4; k++)
			{
				interpX[k] = interpolation[k * 4 + 0];
				interpY[k] = interpolation[k * 4 + 1];
				interpZ[k] = interpolation[k * 4 + 2];
				interpR[k] = interpolation[k * 4 + 3];
			}

			float tx = BezierEval(interpX, ratio);
			float ty = BezierEval(interpY, ratio);
			float tz = BezierEval(interpZ, ratio);
			float tr = BezierEval(interpR, ratio);
			position[0] = (1.0 - tx) * motions[ms.first].position[0] + tx * motions[ms.second].position[0];
			position[1] = (1.0 - ty) * motions[ms.first].position[1] + ty * motions[ms.second].position[1];
			position[2] = (1.0 - tz) * motions[ms.first].position[2] + tz * motions[ms.second].position[2];

			XMFLOAT4 r0;
			r0.x = motions[ms.first].orientation[0];
			r0.y = motions[ms.first].orientation[1];
			r0.z = motions[ms.first].orientation[2];
			r0.w = motions[ms.first].orientation[3];

			XMFLOAT4 r1;
			r1.x = motions[ms.second].orientation[0];
			r1.y = motions[ms.second].orientation[1];
			r1.z = motions[ms.second].orientation[2];
			r1.w = motions[ms.second].orientation[3];

			MyQSlerp(rotation, r0, r1, tr);
		}
	}

#if USE_PMX
	void Update(OpenPose::Frame& opf, OpenPose::Frame& firstOpf)
	{
		int frame = 0;

		if (pivotTime == 0) { pivotTime = timeGetTime(); }

		auto elapsedSec = (timeGetTime() - pivotTime) / 1000.0;
		frame = (int)(elapsedSec * 30);

		// 로컬 트랜스폼을 빌드
		vector<XMMATRIX> localTx;
		localTx.resize(model->bone_count);

		for (size_t i = 0; i < model->bone_count; ++i)
		{
			auto& bone = model->bones[i];

			localTx[i] = XMMatrixIdentity();

			//if (frames[i] == nullptr) 
			if (true)
			{
				//bone.rotation[0] = 0.0;
				//bone.rotation[1] = 0.0;
				//bone.rotation[2] = 0.0;
				//bone.rotation[3] = 1.0;

				XMFLOAT4 motionRot(0, 0, 0, 1);

				if (i == 4) { motionRot = opf.comQuat; }

				if (i == 36) { motionRot = opf.quat[2]; }
				if (i == 43) { motionRot = opf.quat[3]; }
				if (i == 19) { motionRot = opf.quat[5]; }
				if (i == 26) { motionRot = opf.quat[6]; }

				if (i == 69) { motionRot = opf.quat[8]; }
				if (i == 70) { motionRot = opf.quat[9]; }
				if (i == 65) { motionRot = opf.quat[11]; }
				if (i == 66) { motionRot = opf.quat[12]; }

				float motionPos[3] = { 0, 0, 0 };
				QuatToMatrix(localTx[i], motionRot);

				if (bone.parent_index < 0)
				{
					localTx[i].r[3].m128_f32[0] = bone.position[0] + motionPos[0];
					localTx[i].r[3].m128_f32[1] = bone.position[1] + motionPos[1];
					localTx[i].r[3].m128_f32[2] = bone.position[2] + motionPos[2];
				}
				else
				{
					auto& parent = model->bones[bone.parent_index];

					localTx[i].r[3].m128_f32[0] = (bone.position[0] - parent.position[0]) + motionPos[0];
					localTx[i].r[3].m128_f32[1] = (bone.position[1] - parent.position[1]) + motionPos[1];
					localTx[i].r[3].m128_f32[2] = (bone.position[2] - parent.position[2]) + motionPos[2];
				}

				if (bone.parent_index < 0)
				{
					localTx[i].r[3].m128_f32[0] = bone.position[0];
					localTx[i].r[3].m128_f32[1] = bone.position[1];
					localTx[i].r[3].m128_f32[2] = bone.position[2];
				}
				else
				{
					auto& parent = model->bones[bone.parent_index];

					localTx[i].r[3].m128_f32[0] = bone.position[0] - parent.position[0];
					localTx[i].r[3].m128_f32[1] = bone.position[1] - parent.position[1];
					localTx[i].r[3].m128_f32[2] = bone.position[2] - parent.position[2];
				}
			}
			else
			{
				auto ms = FindMotionSegment(frame, *frames[i]);

				XMFLOAT4 motionRot;
				float motionPos[3];
				InterpolateMotion(motionRot, motionPos, *frames[i], ms, frame);

				QuatToMatrix(localTx[i], motionRot);

				if (bone.parent_index < 0)
				{
					localTx[i].r[3].m128_f32[0] = bone.position[0] + motionPos[0];
					localTx[i].r[3].m128_f32[1] = bone.position[1] + motionPos[1];
					localTx[i].r[3].m128_f32[2] = bone.position[2] + motionPos[2];
				}
				else
				{
					auto& parent = model->bones[bone.parent_index];

					localTx[i].r[3].m128_f32[0] = (bone.position[0] - parent.position[0]) + motionPos[0];
					localTx[i].r[3].m128_f32[1] = (bone.position[1] - parent.position[1]) + motionPos[1];
					localTx[i].r[3].m128_f32[2] = (bone.position[2] - parent.position[2]) + motionPos[2];
				}
			}
		}

#if 1
		// com을 일단 복사해보자!
		float comScale = 50.0f;

		localTx[4].r[3].m128_f32[0] = (opf.com.x - firstOpf.com.x) * comScale;
		localTx[4].r[3].m128_f32[1] = (opf.com.y - firstOpf.com.y) * comScale;
		localTx[4].r[3].m128_f32[2] = (opf.com.z - firstOpf.com.z) * comScale;
#endif

		// 월드 트랜스폼을 빌드
		worldTx.resize(model->bone_count);

		for (size_t i = 0; i < model->bone_count; ++i)
		{
			auto& bone = model->bones[i];

			if (bone.parent_index < 0)
			{
				worldTx[i] = localTx[i];
			}
			else
			{
				MatMul(
					worldTx[i],
					worldTx[bone.parent_index],
					localTx[i]);
			}
		}

		// 버텍스를 트랜스폼
		vertices.resize(model->vertex_count);

		for (int i = 0; i < model->vertex_count; i++)
		{
			auto& pv = model->vertices[i];
			XMFLOAT3 p0, p1;
			p0.x = pv.positon[0];
			p0.y = pv.positon[1];
			p0.z = pv.positon[2];
			p1.x = pv.positon[0];
			p1.y = pv.positon[1];
			p1.z = pv.positon[2];

			unsigned short b0;
			unsigned short b1;
			float weight = 0;

			switch (pv.skinning_type) {
			case pmx::PmxVertexSkinningType::BDEF1:
				b0 = ((pmx::PmxVertexSkinningBDEF1*) pv.skinning.get())->bone_index;
				b1 = ((pmx::PmxVertexSkinningBDEF1*) pv.skinning.get())->bone_index;
				weight = 1;
				break;
			case pmx::PmxVertexSkinningType::BDEF2:
				b0 = ((pmx::PmxVertexSkinningBDEF2*) pv.skinning.get())->bone_index1;
				b1 = ((pmx::PmxVertexSkinningBDEF2*) pv.skinning.get())->bone_index2;
				weight = ((pmx::PmxVertexSkinningBDEF2*) pv.skinning.get())->bone_weight;
				break;
			case pmx::PmxVertexSkinningType::BDEF4:
				b0 = ((pmx::PmxVertexSkinningBDEF4*) pv.skinning.get())->bone_index1;
				b1 = ((pmx::PmxVertexSkinningBDEF4*) pv.skinning.get())->bone_index2;
				weight = ((pmx::PmxVertexSkinningBDEF4*) pv.skinning.get())->bone_weight1;
				break;
			case pmx::PmxVertexSkinningType::SDEF:
				b0 = ((pmx::PmxVertexSkinningSDEF*) pv.skinning.get())->bone_index1;
				b1 = ((pmx::PmxVertexSkinningSDEF*) pv.skinning.get())->bone_index2;
				weight = ((pmx::PmxVertexSkinningSDEF*) pv.skinning.get())->bone_weight;
				break;
			case pmx::PmxVertexSkinningType::QDEF:
				b0 = ((pmx::PmxVertexSkinningQDEF*) pv.skinning.get())->bone_index1;
				b1 = ((pmx::PmxVertexSkinningQDEF*) pv.skinning.get())->bone_index2;
				weight = ((pmx::PmxVertexSkinningQDEF*) pv.skinning.get())->bone_weight1;
				break;
			}

			auto& m0 = worldTx[b0];
			auto& m1 = worldTx[b1];

			XMFLOAT3 v0;
			XMFLOAT3 v1;
			XMFLOAT3 v;

			// Bone matrix is defined in absolute coordinates.
			// Pass a vertex in relative coordinate to bone matrix.
			p0.x -= model->bones[b0].position[0];
			p0.y -= model->bones[b0].position[1];
			p0.z -= model->bones[b0].position[2];
			p1.x -= model->bones[b1].position[0];
			p1.y -= model->bones[b1].position[1];
			p1.z -= model->bones[b1].position[2];

			MatVMul(v0, m0, p0);
			MatVMul(v1, m1, p1);
			float w = weight / 100.0f;
			v.x = w * v0.x + (1.0f - w) * v1.x;
			v.y = w * v0.y + (1.0f - w) * v1.y;
			v.z = w * v0.z + (1.0f - w) * v1.z;

			vertices[i] = v / 20.0f;
		}

		indices.resize(model->index_count);
		if (!indices.empty())
		{
			memcpy(
				&indices[0], 
				&model->indices[0], 
				model->index_count * sizeof(indices[0]));
		}
	}

#else
	void Update()
	{
		int frame = 0;

		if (pivotTime == 0) { pivotTime = timeGetTime(); }

		auto elapsedSec = (timeGetTime() - pivotTime) / 1000.0;
		frame = (int) (elapsedSec * 30);

		// 로컬 트랜스폼을 빌드
		vector<XMMATRIX> localTx;
		localTx.resize(model->bones.size());

		for (size_t i = 0; i < model->bones.size(); ++i)
		{
			auto& bone = model->bones[i];

			localTx[i] = XMMatrixIdentity();

			//if (frames[i] == nullptr) 
			if (true)
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
			else
			{
				auto ms = FindMotionSegment(frame, *frames[i]);

				XMFLOAT4 motionRot;
				float motionPos[3];
				InterpolateMotion(motionRot, motionPos, *frames[i], ms, frame);

				QuatToMatrix(localTx[i], motionRot);

				if (bone.parent_bone_index == 0xffff)
				{
					localTx[i].r[3].m128_f32[0] = bone.bone_head_pos[0] + motionPos[0];
					localTx[i].r[3].m128_f32[1] = bone.bone_head_pos[1] + motionPos[1];
					localTx[i].r[3].m128_f32[2] = bone.bone_head_pos[2] + motionPos[2];
				}
				else 
				{
					auto& parent = model->bones[bone.parent_bone_index];

					localTx[i].r[3].m128_f32[0] = (bone.bone_head_pos[0] - parent.bone_head_pos[0]) + motionPos[0];
					localTx[i].r[3].m128_f32[1] = (bone.bone_head_pos[1] - parent.bone_head_pos[1]) + motionPos[1];
					localTx[i].r[3].m128_f32[2] = (bone.bone_head_pos[2] - parent.bone_head_pos[2]) + motionPos[2];
				}
			}
		}

		// 월드 트랜스폼을 빌드
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
#endif

	static void MatMul(XMMATRIX& dst, const XMMATRIX& m0, const XMMATRIX& m1)
	{
		dst = XMMatrixMultiply(m1, m0);
	}

	static void MatVMul(XMFLOAT3& w, const XMMATRIX& m_, const XMFLOAT3& v)
	{
#if 0
		const float* m = (const float*)&m_;
		w.x = m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12];
		w.y = m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13];
		w.z = m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14];
#endif

		XMVECTOR v_ = XMLoadFloat3(&v);
		XMVECTOR t = XMVector3Transform(v_, m_);

		w = XMFLOAT3(t.m128_f32[0], t.m128_f32[1], t.m128_f32[2]);
	}
};

IMmdPlayer::~IMmdPlayer()
{}

IMmdPlayer* IMmdPlayer::Create()
{ return new MmdPlayer; }