#include "pch.h"
#include <Windows.h>

#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "winmm.lib")

HINSTANCE g_hInst = NULL;
HWND g_hWnd = NULL;

static int windowWidth = 1280;
static int windowHeight = 720;

extern INT WINAPI wWinMain_DX11(HINSTANCE hInst, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow);

HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow, WNDPROC wndProc)
{
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
	wcex.lpszClassName = L"TutorialWindowClass";
	wcex.hIconSm = NULL;// LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	// Create window
	g_hInst = hInstance;
	RECT rc = { 0, 0, windowWidth, windowHeight, };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(L"TutorialWindowClass", L"Dazza Prerenderer", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
		NULL);
	if (!g_hWnd)
		return E_FAIL;

	ShowWindow(g_hWnd, nCmdShow);

	return S_OK;
}

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	return wWinMain_DX11(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}