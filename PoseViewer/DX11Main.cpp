#include "pch.h"

#include <functional>

#include <windows.h>

#include <d3d11.h>
#include <d3dcompiler.h>

#include "Global.h"
#include "Render.h"

using namespace std;

//------------------------------------------------------------------------------
auto_ptr<GlobalContext> global;
static auto_ptr<DX11Render> render;

//------------------------------------------------------------------------------
// Forward declarations
extern HRESULT InitWindow(HWND& hWnd, HINSTANCE hInstance, int nCmdShow, WNDPROC wndProc);
extern int MessagePump(const function<void()>&);

static void CleanupDevice();
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
int WINAPI DX11Main(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	HWND hWnd = nullptr;
	if (FAILED(InitWindow(hWnd, hInstance, nCmdShow, WndProc)))
	{
		return 0;
	}

	try
	{
		global.reset(new GlobalContext(hWnd));

		render.reset(new DX11Render(hWnd));
	}
	catch (...)
	{
		return 0;
	}

	auto exitCode = MessagePump([] { render->Render(nullptr, 0, false); });

	CleanupDevice();

	return exitCode;
}

//------------------------------------------------------------------------------
void CleanupDevice()
{
	render.reset(NULL);
	global.reset(NULL);
}

//------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_KEYDOWN:
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
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

