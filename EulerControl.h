#pragma once

#include <Windows.h>
#include <DirectXMath.h>

class IEulerControl {
public:
	virtual ~IEulerControl();

	virtual LRESULT HandleMessages(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) = 0;

	virtual void Update(const DirectX::XMFLOAT3& target, float dolly) = 0;
	virtual const DirectX::XMFLOAT3& GetEyePosition() const = 0;
	virtual const DirectX::XMMATRIX& GetRotationMatrix() const = 0;

	static IEulerControl* Create(float yaw, float pitch);
};
