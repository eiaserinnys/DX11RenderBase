#include "pch.h"
#include "ThisApp.h"

#include <WindowsUtility.h>
#include <Utility.h>

#include "Global.h"
#include "Render.h"

#include "OpenPose.h"

using namespace std;

class ThisApp : public IThisApp {
public:
	unique_ptr<GlobalContext> global;
	unique_ptr<DX11Render> render;
	HWND hWnd;

	vector<OpenPose::Frame> frames;

	ThisApp(HWND hWnd)
		: hWnd(hWnd)
	{
		global.reset(new GlobalContext(hWnd));
		render.reset(new DX11Render(hWnd, global->d3d11.get()));

		Load();
	}

	void Load()
	{
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
			}
			catch (const exception& e)
			{
				WindowsUtility::Debug(L"%S\n", e.what());
				break;
			}
		}
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
		render->Render(nullptr, 0, false);
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