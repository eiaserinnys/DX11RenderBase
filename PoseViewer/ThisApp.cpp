#include "pch.h"
#include "ThisApp.h"

#include "Global.h"
#include "Render.h"

using namespace std;

class ThisApp : public IThisApp {
public:
	unique_ptr<GlobalContext> global;
	unique_ptr<DX11Render> render;
	HWND hWnd;

	ThisApp(HWND hWnd)
		: hWnd(hWnd)
	{
		global.reset(new GlobalContext(hWnd));
		render.reset(new DX11Render(hWnd, global->d3d11.get()));
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