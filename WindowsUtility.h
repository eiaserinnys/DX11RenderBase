#pragma once

#include <functional>
#include <utility>

#include <Windows.h>

struct WindowsUtility
{
	static void FillWindowClass(
		WNDCLASSEX& wcex,
		HINSTANCE hInstance,
		LPWSTR className,
		WNDPROC wndProc);

	static HWND CreateOverlappedWindow(
		HINSTANCE hInstance,
		LPWSTR className,
		LPWSTR windowName,
		int width,
		int height);

	static std::pair<HRESULT, HWND> RegisterAndCreateOverlappedWindow(
		HINSTANCE hInstance,
		LPWSTR className,
		LPWSTR windowName,
		int windowWidth,
		int windowHeight,
		int nCmdShow,
		WNDPROC wndProc);

	static int MessagePump(const std::function<void()>& functor);
};