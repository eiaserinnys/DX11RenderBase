#include "pch.h"
#include "ThisApp.h"

#include <DirectXMath.h>

#include <WindowsUtility.h>
#include <Utility.h>

#include "RenderContext.h"
#include "Render.h"

#include "OpenPose.h"
#include "OpenPoseRenderer.h"

#include "Vmd.h"

using namespace std;
using namespace DirectX;

class ThisApp : public IThisApp {
public:
	unique_ptr<RenderContext> global;
	unique_ptr<IOpenPoseRenderer> openPoseRender;
	unique_ptr<DX11Render> render;
	HWND hWnd;

	vector<OpenPose::Frame> frames;
	int cur = 0;

	bool advance = false;
	int pivotTime = 0;

	XMFLOAT3 com;

	ThisApp(HWND hWnd)
		: hWnd(hWnd)
	{
		global.reset(new RenderContext(hWnd));
		render.reset(new DX11Render(hWnd, global->d3d11.get()));
		openPoseRender.reset(IOpenPoseRenderer::Create(global.get()));

		Load();
	}

	void Load()
	{
		// VMD를 로드해본다
		std::ifstream is;
		is.open("Vmd/Happy Hands Meme.vmd", std::ios::in | std::ios::binary);

		unique_ptr<vmd::VmdMotion> vmdMotion;
		if (is.is_open())
		{
			vmdMotion = vmd::VmdMotion::LoadFromStream(&is);
		}

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
		float dist = 0.75f;
		sceneDesc.Build(hWnd, com + XMFLOAT3(0, -dist, dist), com, rot);

		openPoseRender->Render(frames[cur], sceneDesc);

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