#include "pch.h"
#include <Windows.h>

#include <functional>

#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "winmm.lib")

using namespace std;

static HINSTANCE g_hInst = NULL;
static HWND g_hWnd = NULL;

static int windowWidth = 1280;
static int windowHeight = 720;

//------------------------------------------------------------------------------
extern INT WINAPI DX11Main(HINSTANCE hInst, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow);

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	return DX11Main(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}

//------------------------------------------------------------------------------
HRESULT InitWindow(HWND& hWnd, HINSTANCE hInstance, int nCmdShow, WNDPROC wndProc)
{
	hWnd = nullptr;

	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = wndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;// LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
	wcex.hCursor = NULL;//LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"DX11RendererWindowClass";
	wcex.hIconSm = NULL;// LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
	if (!RegisterClassEx(&wcex))
	{
		return E_FAIL;
	}

	// Create window
	g_hInst = hInstance;
	RECT rc = { 0, 0, windowWidth, windowHeight, };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(
		L"DX11RendererWindowClass", 
		L"DX11Renderer", 
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 
		CW_USEDEFAULT, 
		rc.right - rc.left, 
		rc.bottom - rc.top, 
		NULL, 
		NULL, 
		hInstance,
		NULL);

	if (!g_hWnd)
	{
		return E_FAIL;
	}

	ShowWindow(g_hWnd, nCmdShow);

	hWnd = g_hWnd;

	return S_OK;
}

//------------------------------------------------------------------------------
int MessagePump(const function<void()>& functor)
{
	// Main message loop
	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			functor();
		}
	}

	return (int)msg.wParam;
}
