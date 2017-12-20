#pragma once

#include <Windows.h>

class IThisApp {
public:
	virtual ~IThisApp();

	virtual void Do() = 0;

	virtual void OnKeyDown(WPARAM wParam, LPARAM lParam) = 0;

	static IThisApp* Create(HWND hWnd);
};