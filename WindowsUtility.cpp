#include "pch.h"
#include "WindowsUtility.h"

using namespace std;

//------------------------------------------------------------------------------
void WindowsUtility::FillWindowClass(
	WNDCLASSEX& wcex, 
	HINSTANCE hInstance, 
	LPWSTR className, 
	WNDPROC wndProc)
{
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = wndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;// LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = className;
	wcex.hIconSm = NULL;// LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
}

//------------------------------------------------------------------------------
HWND WindowsUtility::CreateOverlappedWindow(
	HINSTANCE hInstance, 
	LPWSTR className, 
	LPWSTR windowName, 
	int width, 
	int height)
{
	RECT rc = { 0, 0, width, height, };

	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	return CreateWindow(
		className,
		windowName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		NULL,
		NULL,
		hInstance,
		NULL);
}

//------------------------------------------------------------------------------
pair<HRESULT, HWND> WindowsUtility::RegisterAndCreateOverlappedWindow(
	HINSTANCE hInstance, 
	LPWSTR className, 
	LPWSTR windowName,
	int windowWidth, 
	int windowHeight, 
	int nCmdShow, 
	WNDPROC wndProc)
{
	// Register class
	WNDCLASSEX wcex;
	WindowsUtility::FillWindowClass(wcex, hInstance, className, wndProc);
	if (!RegisterClassEx(&wcex)) { return make_pair((HRESULT)E_FAIL, (HWND)NULL); }

	// Create window
	HWND hWnd = WindowsUtility::CreateOverlappedWindow(
		hInstance, className, windowName, windowWidth, windowHeight);

	if (!hWnd) { return make_pair((HRESULT)E_FAIL, (HWND)NULL); }

	ShowWindow(hWnd, nCmdShow);

	return make_pair(S_OK, hWnd);
}

//------------------------------------------------------------------------------
int WindowsUtility::MessagePump(const function<void()>& functor)
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

//------------------------------------------------------------------------------
LRESULT WindowsUtility::DefaultWndProc(
	HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;

			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}