#include "pch.h"
#include "ThisApp.h"

#include <DirectXMath.h>

#include <WindowsUtility.h>
#include <Utility.h>

#include "RenderContext.h"
#include "Render.h"

#include "OpenPose.h"
#include "OpenPoseRenderer.h"

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

	int lastTime = 0;

	XMFLOAT3 com;

	ThisApp(HWND hWnd)
		: hWnd(hWnd)
	{
		global.reset(new RenderContext(hWnd));
		render.reset(new DX11Render(hWnd, global->d3d11.get()));
		openPoseRender.reset(IOpenPoseRenderer::Create(global.get()));

		lastTime = timeGetTime();

		Load();
	}

	void Load()
	{
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

		if (timeGetTime() - lastTime > 30)
		{
			lastTime = timeGetTime();

			cur++;
			if (cur >= frames.size()) { cur = 0; }
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
	}
};

IThisApp::~IThisApp()
{}

IThisApp* IThisApp::Create(HWND hWnd)
{ return new ThisApp(hWnd); }