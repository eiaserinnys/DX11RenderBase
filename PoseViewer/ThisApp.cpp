﻿#include "pch.h"
#include "ThisApp.h"

#include <DirectXMath.h>

#include <WindowsUtility.h>
#include <Utility.h>

#include "RenderContext.h"
#include "Render.h"

#include "OpenPose.h"
#include "OpenPoseRenderer.h"

#include "Pmd.h"
#include "Vmd.h"
#include "MmdRenderer.h"

using namespace std;
using namespace DirectX;

class ThisApp : public IThisApp {
public:
	unique_ptr<RenderContext> global;
	unique_ptr<IOpenPoseRenderer> openPoseRender;
	unique_ptr<IMmdRenderer> mmdRender;
	unique_ptr<DX11Render> render;
	HWND hWnd;

	vector<OpenPose::Frame> frames;
	int cur = 0;

	bool advance = false;
	int pivotTime = 0;

	XMFLOAT3 com;

	unique_ptr<pmd::PmdModel> pmdModel;

	bool init = false;

	ThisApp(HWND hWnd)
		: hWnd(hWnd)
	{
		global.reset(new RenderContext(hWnd));
		render.reset(new DX11Render(hWnd, global->d3d11.get()));
		openPoseRender.reset(IOpenPoseRenderer::Create(global.get()));
		mmdRender.reset(IMmdRenderer::Create(global.get()));

		Load();
	}

	void Load()
	{
		// PMD를 로드해본다
		pmdModel = pmd::PmdModel::LoadFromFile("MMD/md_m.pmd");

		for (size_t i = 0; i < pmdModel->bones.size(); ++i)
		{
			auto& b = pmdModel->bones[i];

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
					vmd::sjis2utf8(pmdModel->bones[b.parent_bone_index].name).c_str());
			}
			else
			{
				WindowsUtility::Debug(L"\n");
			}
		}

		// VMD를 로드해본다
		std::ifstream is;
		is.open("MMD/Happy Hands Meme.vmd", std::ios::in | std::ios::binary);

		unique_ptr<vmd::VmdMotion> vmdMotion;
		if (is.is_open())
		{
			vmdMotion = vmd::VmdMotion::LoadFromStream(&is);
		}

		for (auto frame : vmdMotion->bone_frames)
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

		// 오픈포즈를 로드해본다
		com = XMFLOAT3(0, 0, 0);

		for (int i = 0; ; ++i)
		{
			try
			{
				auto fileName = Utility::Format(
					"Pose/redvelvet-redflavor-mod_%012d_keypoints.json",
					i);

				OpenPose::Frame frame;

				OpenPose::Load(fileName, frame);

				for (int i = 0; i < COUNT_OF(frame.pos); ++i)
				{
					if (frame.pos[i].x == 0 &&
						frame.pos[i].y == 0 &&
						frame.pos[i].z == 0)
					{
						const auto& last = *frames.rbegin();
						frame.pos[i] = last.pos[i];
					}
				}

				frames.push_back(frame);

				com = com + frame.com;
			}
			catch (const exception& e)
			{
				WindowsUtility::Debug(L"%S\n", e.what());
				break;
			}
		}

		com = com / (float) frames.size();

		// 일단 막 vmd를 만들어보자
		unique_ptr<vmd::VmdMotion> vmdMotionOut;
		{
			vmdMotionOut.reset(new vmd::VmdMotion);

			vmdMotionOut->model_name = vmd::utf82sjis(L"初音ミク");
			vmdMotionOut->version = 2;

			int fi =  0;
			for (auto f : frames)
			{
				AddFrame(vmdMotion.get(), vmdMotionOut.get(), f, fi);
				fi += 5;
			}
		}

		vmdMotionOut->SaveToFile(
			L"D:\\__Development__\\MikuMikuDanceE_v931x64\\UserFile\\Motion\\RedFlavor.vmd");
	}

	XMMATRIX GetPose(
		OpenPose::Frame& f,
		int pivot,
		int next,
		int ref)
	{
		XMFLOAT3 front = f.pos[next] - f.pos[pivot];
		XMFLOAT3 left_ = f.pos[ref] - f.pos[pivot];
		XMFLOAT3 up = Cross(front, left_);
		XMFLOAT3 left = Cross(up, front);

		XMMATRIX bone = XMMatrixIdentity();
		if (Length(front) == 0 || Length(left) == 0 || Length(up) == 0)
		{
			bone = XMMatrixIdentity();
		}
		else
		{
			Normalize(front);
			Normalize(left);
			Normalize(up);

			bone.r[0].m128_f32[0] = front.x;
			bone.r[0].m128_f32[1] = front.y;
			bone.r[0].m128_f32[2] = front.z;

			bone.r[1].m128_f32[0] = left.x;
			bone.r[1].m128_f32[1] = left.y;
			bone.r[1].m128_f32[2] = left.z;

			bone.r[2].m128_f32[0] = up.x;
			bone.r[2].m128_f32[1] = up.y;
			bone.r[2].m128_f32[2] = up.z;

			//bone = XMMatrixLookAtLH(
			//	XMLoadFloat3(&f.pos[pivot]),
			//	XMLoadFloat3(&f.pos[next]),
			//	XMLoadFloat3(&up));
		}

		return bone;
	}

	XMMATRIX AddBone(
		vmd::VmdMotion* vmdMotion,
		vmd::VmdMotion* vmdMotionOut,
		OpenPose::Frame& f,
		int fi, 
		const wstring& name,
		int pivot, 
		int next, 
		int ref, 
		const XMMATRIX& parent)
	{
		vmd::VmdBoneFrame vf;
		vf.name = vmd::utf82sjis(name);
		vf.frame = fi;
		vf.position[0] = 0;
		vf.position[1] = 0;
		vf.position[2] = 0;

		XMMATRIX bone = GetPose(f, pivot, next, ref);

		XMMATRIX invPar = XMMatrixInverse(nullptr, parent);
		auto local = XMMatrixMultiply(invPar, bone);

		XMVECTOR quat = XMQuaternionRotationMatrix(local);

		vf.orientation[0] = quat.m128_f32[0];
		vf.orientation[1] = quat.m128_f32[1];
		vf.orientation[2] = quat.m128_f32[2];
		vf.orientation[3] = quat.m128_f32[3];

		memcpy(
			vf.interpolation,
			&vmdMotion->bone_frames[0].interpolation,
			sizeof(char) * 4 * 4 * 4);

		vmdMotionOut->bone_frames.push_back(vf);

		return bone;
	}

	void AddFrame(
		vmd::VmdMotion* vmdMotion,
		vmd::VmdMotion* vmdMotionOut,
		OpenPose::Frame& f,
		int fi)
	{
		XMMATRIX base = GetPose(f, 1, 8, 11);
		XMMATRIX rs = AddBone(vmdMotion, vmdMotionOut, f, fi, L"右肩", 2, 3, 1, base);
		AddBone(vmdMotion, vmdMotionOut, f, fi, L"右ひじ", 3, 4, 1, rs);
	}

	~ThisApp()
	{
		CleanUp();
	}

	void CleanUp()
	{
		render.reset(NULL);
		global.reset(NULL);
	}

	virtual void Do()
	{
		render->Begin();

		SceneDescriptor sceneDesc;

		XMMATRIX rot = XMMatrixIdentity();
		{
			//float dist = 0.75f;
			//sceneDesc.Build(hWnd, com + XMFLOAT3(0, -dist, dist), com, rot);
			//openPoseRender->Render(frames[cur], sceneDesc);
		}

		{
			XMFLOAT3 ofs(0, 0, 0);
			float dist = 5.0f;
			sceneDesc.Build(hWnd, ofs + XMFLOAT3(0, -30, 0), ofs, rot);
			mmdRender->Render(pmdModel.get(), sceneDesc);
		}

		if (advance)
		{
			if (pivotTime == 0)
			{
				pivotTime = timeGetTime();
			}

			int totalAdvance = timeGetTime() - pivotTime;

			cur = (int)(totalAdvance / 1000.0f * 30);
			cur = cur % frames.size();
		}

		render->End();
	}

	virtual void OnKeyDown(WPARAM wParam, LPARAM lParam)
	{
		if (wParam == VK_F5)
		{
			try
			{
				global->Reload();
			}
			catch (HRESULT)
			{
				MessageBox(hWnd, L"Reload Error", L"Reload Error", MB_OK);
			}
		}
		if (wParam == VK_F1)
		{
			advance = !advance;
		}
	}
};

IThisApp::~IThisApp()
{}

IThisApp* IThisApp::Create(HWND hWnd)
{ return new ThisApp(hWnd); }