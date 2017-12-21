#include "pch.h"
#include "ThisApp.h"

#include <DirectXMath.h>

#include <WindowsUtility.h>
#include <Utility.h>
#include <ArcBall.h>

#include "RenderContext.h"
#include "Render.h"

#include "OpenPose.h"
#include "OpenPoseRenderer.h"

#include "vmd.h"
#include "MmdPlayer.h"
#include "MmdRenderer.h"
#include "MmdBoneRenderer.h"

using namespace std;
using namespace DirectX;

class ThisApp : public IThisApp {
public:
	unique_ptr<RenderContext> global;
	unique_ptr<IOpenPoseRenderer> openPoseRender;
	unique_ptr<IMmdPlayer> mmdPlayer;
	unique_ptr<IMmdRenderer> mmdRender;
	unique_ptr<IMmdBoneRenderer> mmdBoneRender;
	unique_ptr<DX11Render> render;
	HWND hWnd;

	unique_ptr<IArcBall> arcBall;

	vector<OpenPose::Frame> frames;
	int cur = 0;

	bool advance = false;
	int lastTime = 0;
	int totalAdvance = 0;

	XMFLOAT3 com;

	OpenPose::Frame refFrame;

	bool init = false;

	ThisApp(HWND hWnd)
		: hWnd(hWnd)
	{
		global.reset(new RenderContext(hWnd));
		render.reset(new DX11Render(hWnd, global->d3d11.get()));
		openPoseRender.reset(IOpenPoseRenderer::Create(global.get()));
		mmdRender.reset(IMmdRenderer::Create(global.get()));
		mmdBoneRender.reset(IMmdBoneRenderer::Create(global.get()));

		arcBall.reset(IArcBall::Create());

		Load();
	}

	void Load()
	{
		mmdPlayer.reset(IMmdPlayer::Create());
		mmdPlayer->Load();

		// 레퍼런스 포즈를 로드한다
		OpenPose::Load("Pose/miku_keypoints.json", refFrame);
		com = refFrame.com;

		//frames.push_back(refFrame);

#if 1
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

				OpenPose::Load(fileName, frame, refFrame);

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

				//WindowsUtility::Debug(L"%d, %f,%f,%f\n", i, frame.com.x, frame.com.y, frame.com.z);

				com = com + frame.com;
			}
			catch (const exception& e)
			{
				WindowsUtility::Debug(L"%S\n", e.what());
				break;
			}
		}

		com = com / (float) frames.size();
#endif

		// 일단 막 vmd를 만들어보자
		unique_ptr<vmd::VmdMotion> vmdMotionOut;
		{
			vmdMotionOut.reset(new vmd::VmdMotion);

			vmdMotionOut->model_name = vmd::utf82sjis(L"初音ミク");
			vmdMotionOut->version = 2;

			auto model = mmdPlayer->GetModel();
			auto motion = mmdPlayer->GetMotion();

			for (int i = 0; i < frames.size(); ++i)
			{
				auto& f = frames[i];

				auto& writeRotation = [&](vmd::VmdBoneFrame& bf, int mi, XMFLOAT4& quat)
				{
					bf.frame = i;

					memcpy(
						bf.interpolation,
						motion->bone_frames[0].interpolation,
						sizeof(char) * 4 * 4 * 4);

					bf.name = vmd::utf82sjis(model->bones[mi].bone_name);

					bf.orientation[0] = quat.x;
					bf.orientation[1] = quat.y;
					bf.orientation[2] = quat.z;
					bf.orientation[3] = quat.w;

					bf.position[0] = 0;
					bf.position[1] = 0;
					bf.position[2] = 0;
				};

				{
					vmd::VmdBoneFrame bf;

					writeRotation(bf, 4, f.comQuat);

					auto move = f.com - frames[0].com;
					float scale = 100;

					bf.position[0] = move.x * scale;
					bf.position[1] = move.y * scale;
					bf.position[2] = move.z * scale;

					vmdMotionOut->bone_frames.push_back(bf);
				}

				vmd::VmdBoneFrame bf;

				//if (i == 36) { motionRot = opf.quat[2]; }
				//if (i == 43) { motionRot = opf.quat[3]; }
				//if (i == 19) { motionRot = opf.quat[5]; }
				//if (i == 26) { motionRot = opf.quat[6]; }

				//if (i == 69) { motionRot = opf.quat[8]; }
				//if (i == 70) { motionRot = opf.quat[9]; }
				//if (i == 65) { motionRot = opf.quat[11]; }
				//if (i == 66) { motionRot = opf.quat[12]; }

				writeRotation(bf, 36, f.quat[2]);
				vmdMotionOut->bone_frames.push_back(bf);

				writeRotation(bf, 43, f.quat[3]);
				vmdMotionOut->bone_frames.push_back(bf);

				writeRotation(bf, 19, f.quat[5]);
				vmdMotionOut->bone_frames.push_back(bf);

				writeRotation(bf, 26, f.quat[6]);
				vmdMotionOut->bone_frames.push_back(bf);

				writeRotation(bf, 69, f.quat[8]);
				vmdMotionOut->bone_frames.push_back(bf);

				writeRotation(bf, 70, f.quat[9]);
				vmdMotionOut->bone_frames.push_back(bf);

				writeRotation(bf, 65, f.quat[11]);
				vmdMotionOut->bone_frames.push_back(bf);

				writeRotation(bf, 66, f.quat[12]);
				vmdMotionOut->bone_frames.push_back(bf);
			}
		}

		vmdMotionOut->SaveToFile(
			L"D:\\__Development__\\MikuMikuDanceE_v931x64\\UserFile\\Motion\\RedFlavor.vmd");
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

	IArcBall* GetArcBall() { return arcBall.get(); }

	virtual void Do()
	{
		render->Begin();

		SceneDescriptor sceneDesc;

		XMMATRIX rot = arcBall->GetRotationMatrix();

		{
			float dist = 1.0f;
			XMFLOAT3 ofs = XMFLOAT3(0.5, 0, -0.1f);
			sceneDesc.Build(hWnd, com + XMFLOAT3(0.5, 0, -dist) + ofs, com + ofs, rot);
			openPoseRender->Render(render.get(), frames[cur], sceneDesc);

			render->RenderText(sceneDesc.worldViewProj);
		}

		{
			mmdPlayer->Update(frames[cur], frames[0]);

			XMFLOAT3 ofs(0, 0, 0);
			float dist = 5.0f;
			sceneDesc.Build(
				hWnd, 
				XMFLOAT3(0.f, 0.3, -3), 
				XMFLOAT3(0.f, 0.3f, 0), 
				rot);
			mmdRender->Render(mmdPlayer.get(), sceneDesc);
			//mmdBoneRender->Render(render.get(), mmdPlayer.get(), sceneDesc);

			render->RenderText(sceneDesc.worldViewProj);
		}

		if (advance)
		{
			int curTime = timeGetTime();

			if (lastTime == 0)
			{
				lastTime = curTime;
			}

			int curAdv = curTime - lastTime;
			lastTime = curTime;

			totalAdvance += curAdv;

			cur = (int)(totalAdvance / 1000.0f * 30) ;
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
			lastTime = 0;
		}
	}
};

IThisApp::~IThisApp()
{}

IThisApp* IThisApp::Create(HWND hWnd)
{ return new ThisApp(hWnd); }