#include "pch.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <DirectXMath.h>
#include "Utility.h"

#include "EulerControl.h"

using namespace std;
using namespace DirectX;

class EulerControl : public IEulerControl
{
public:
	EulerControl(float yaw, float pitch)
	{
		Reset();

		curYawPitch = downYawPitch = XMFLOAT2(yaw, pitch);

		downPt = XMFLOAT2(0, 0);
		curPt = XMFLOAT2(0, 0);
		offset.x = offset.y = 0;

		RECT rc;
		GetClientRect(GetForegroundWindow(), &rc);
		SetWindow(rc.right, rc.bottom);
	}

	void Reset()
	{
		rotation = XMMatrixIdentity();

		isDragging = false;
		radius = 1.0f;
	}

	void SetWindow(_In_ INT nWidth, _In_ INT nHeight, _In_ float fRadius = 0.9f)
	{
		width = nWidth;
		height = nHeight;
		radius = fRadius;
		center.x = float(width) / 2.0f;
		center.y = float(height) / 2.0f;
	}

	void OnBegin(_In_ int nX, _In_ int nY)   // start the rotation (pass current mouse position)
	{
		// Only enter the drag state if the click falls
		// inside the click rectangle.
		if (nX >= offset.x && nX < offset.x + width &&
			nY >= offset.y && nY < offset.y + height)
		{
			isDragging = true;

			XMVECTOR v = ScreenToVector(float(nX), float(nY));
			XMStoreFloat2(&downPt, v);

			downYawPitch = curYawPitch;
		}
	}

	void OnMove(_In_ int nX, _In_ int nY)    // continue the rotation (pass current mouse position)
	{
		if (isDragging)
		{
			XMVECTOR curr = ScreenToVector((float)nX, (float)nY);
			XMStoreFloat2(&curPt, curr);

			XMFLOAT2 delta = XMFLOAT2(
				curPt.x - downPt.x,
				curPt.y - downPt.y);

			float yaw = delta.x * (float)M_PI;
			float pitch = delta.y * (float)M_PI;

			curYawPitch.x = fmodf(downYawPitch.x + yaw, 2 * M_PI);
			curYawPitch.y = downYawPitch.y + pitch;

			if (curYawPitch.y < 0) { curYawPitch.y = 0; }

			float maxP = 85 / 180.0f * M_PI;
			if (curYawPitch.y > maxP) { curYawPitch.y = maxP; }
		}
	}

	void OnEnd()                               // end the rotation 
	{
		isDragging = false;
	}

	// Or call this to automatically handle left, middle, right buttons
	LRESULT HandleMessages(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
	{
		// Current mouse position
		int mouseX = (short)LOWORD(lParam);
		int mouseY = (short)HIWORD(lParam);

		switch (uMsg) {
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
			SetCapture(hWnd);
			OnBegin(mouseX, mouseY);
			return TRUE;

		case WM_RBUTTONUP:
			ReleaseCapture();
			OnEnd();
			return TRUE;

		case WM_CAPTURECHANGED:
			if ((HWND)lParam != hWnd)
			{
				ReleaseCapture();
				OnEnd();
			}
			return TRUE;

		case WM_MOUSEMOVE:
			if (MK_RBUTTON & wParam)
			{
				OnMove(mouseX, mouseY);
			}
			return TRUE;
		}

		return FALSE;
	}

	void Update(const XMFLOAT3& target, float dolly) 
	{
		XMFLOAT3 view(
			cosf(curYawPitch.x) * cosf(curYawPitch.y),
			- sinf(curYawPitch.y),
			sinf(curYawPitch.x) * cosf(curYawPitch.y));

		eye = target - dolly * view;

		XMVECTOR eyeV = XMLoadFloat3(&eye);
		XMVECTOR targetV = XMLoadFloat3(&target);
		XMVECTOR upV = XMLoadFloat3(&XMFLOAT3(0, 1, 0));
		rotation = XMMatrixLookAtRH(eyeV, targetV, upV);
	}

	const XMMATRIX& GetRotationMatrix() const { return rotation; }
	const XMFLOAT3& GetEyePosition() const { return eye; }


protected:
	XMMATRIX rotation;        // Matrix for arc ball's orientation
	XMFLOAT3 eye;

	POINT offset;               // window offset, or upper-left corner of window
	INT width;                  // arc ball's window width
	INT height;                 // arc ball's window height
	XMFLOAT2 center;            // center of arc ball 
	float radius;               // arc ball's radius in screen coords

	bool isDragging;            // Whether user is dragging arc ball

	XMFLOAT2 downYawPitch;
	XMFLOAT2 curYawPitch;
	XMFLOAT2 downPt;            // starting point of rotation arc
	XMFLOAT2 curPt;         // current point of rotation arc

	XMVECTOR ScreenToVector(_In_ float fScreenPtX, _In_ float fScreenPtY)
	{
		float x = -(fScreenPtX - offset.x - width / 2) / (width / 2);
		float y = -(fScreenPtY - offset.y - height / 2) / (height / 2);
		return XMVectorSet(x, y, 0, 0);
	}
};

IEulerControl::~IEulerControl()
{}

IEulerControl* IEulerControl::Create(float yaw, float pitch)
{ return new EulerControl(yaw, pitch); }
