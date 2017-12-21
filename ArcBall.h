#pragma once

#include <Windows.h>
#include <DirectXMath.h>

class IArcBall {
public:
	virtual ~IArcBall();

	virtual LRESULT HandleMessages(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) = 0;

	virtual DirectX::XMMATRIX GetRotationMatrix() const = 0;

	static IArcBall* Create();
};
