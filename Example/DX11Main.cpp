//--------------------------------------------------------------------------------------
// File: Tutorial07.cpp
//
// This application demonstrates texturing
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "pch.h"

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include "Global.h"
#include "Render.h"

using namespace std;

//------------------------------------------------------------------------------
// Global Variables
extern HINSTANCE	g_hInst;
extern HWND			g_hWnd;

auto_ptr<GlobalContext> global;
auto_ptr<DX11Render> render;

//------------------------------------------------------------------------------
// Forward declarations
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow, WNDPROC wndProc);
static void CleanupDevice();
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
int WINAPI wWinMain_DX11(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(InitWindow(hInstance, nCmdShow, WndProc)))
		return 0;

	try
	{
		global.reset(new GlobalContext(g_hWnd));
	}
	catch (HRESULT)
	{
		return 0;
	}

	try
	{
		render.reset(new DX11Render(g_hWnd));
	}
	catch (HRESULT)
	{
		return 0;
	}

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
			//render->Render();
		}
	}

	CleanupDevice();

	return (int)msg.wParam;
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
				MessageBox(g_hWnd, L"Reload Error", L"Reload Error", MB_OK);
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

