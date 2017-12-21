#include "pch.h"

#include <DirectXMath.h>

#include "ArcBall.h"

using namespace std;
using namespace DirectX;

class CD3DArcBall : public IArcBall
{
public:
	CD3DArcBall()
	{
		Reset();

		m_vDownPt = XMFLOAT3(0, 0, 0);
		m_vCurrentPt = XMFLOAT3(0, 0, 0);
		m_Offset.x = m_Offset.y = 0;

		RECT rc;
		GetClientRect(GetForegroundWindow(), &rc);
		SetWindow(rc.right, rc.bottom);
	}

	// Functions to change behavior
	void Reset()
	{
		XMVECTOR qid = XMQuaternionIdentity();
		XMStoreFloat4(&m_qDown, qid);
		XMStoreFloat4(&m_qNow, qid);

		XMMATRIX id = XMMatrixIdentity();
		XMStoreFloat4x4(&m_mRotation, id);
		XMStoreFloat4x4(&m_mTranslation, id);
		XMStoreFloat4x4(&m_mTranslationDelta, id);

		m_bDrag = false;
		m_fRadiusTranslation = 1.0f;
		m_fRadius = 1.0f;
	}

	void SetTranslationRadius(_In_ float fRadiusTranslation)
	{
		m_fRadiusTranslation = fRadiusTranslation;
	}
	void SetWindow(_In_ INT nWidth, _In_ INT nHeight, _In_ float fRadius = 0.9f)
	{
		m_nWidth = nWidth;
		m_nHeight = nHeight;
		m_fRadius = fRadius;
		m_vCenter.x = float(m_nWidth) / 2.0f;
		m_vCenter.y = float(m_nHeight) / 2.0f;
	}
	void SetOffset(_In_ INT nX, _In_ INT nY) { m_Offset.x = nX; m_Offset.y = nY; }

	// Call these from client and use GetRotationMatrix() to read new rotation matrix
	void OnBegin(_In_ int nX, _In_ int nY)   // start the rotation (pass current mouse position)
	{
		// Only enter the drag state if the click falls
		// inside the click rectangle.
		if (nX >= m_Offset.x &&
			nX < m_Offset.x + m_nWidth &&
			nY >= m_Offset.y &&
			nY < m_Offset.y + m_nHeight)
		{
			m_bDrag = true;
			m_qDown = m_qNow;
			XMVECTOR v = ScreenToVector(float(nX), float(nY));
			XMStoreFloat3(&m_vDownPt, v);
		}
	}

	void OnMove(_In_ int nX, _In_ int nY)    // continue the rotation (pass current mouse position)
	{
		if (m_bDrag)
		{
			XMVECTOR curr = ScreenToVector((float)nX, (float)nY);
			XMStoreFloat3(&m_vCurrentPt, curr);

			XMVECTOR down = XMLoadFloat3(&m_vDownPt);
			XMVECTOR qdown = XMLoadFloat4(&m_qDown);

			XMVECTOR result = XMQuaternionMultiply(qdown, QuatFromBallPoints(down, curr));
			XMStoreFloat4(&m_qNow, result);
		}
	}

	void OnEnd()                               // end the rotation 
	{
		m_bDrag = false;
	}

	// Or call this to automatically handle left, middle, right buttons
	LRESULT HandleMessages(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
	{
		// Current mouse position
		int iMouseX = (short)LOWORD(lParam);
		int iMouseY = (short)HIWORD(lParam);

		switch (uMsg)
		{
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
			SetCapture(hWnd);
			OnBegin(iMouseX, iMouseY);
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

		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
			SetCapture(hWnd);
			// Store off the position of the cursor when the button is pressed
			m_ptLastMouse.x = iMouseX;
			m_ptLastMouse.y = iMouseY;
			return TRUE;

		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
			ReleaseCapture();
			return TRUE;

		case WM_MOUSEMOVE:
			if (MK_RBUTTON & wParam)
			{
				OnMove(iMouseX, iMouseY);
			}
			else if ((MK_LBUTTON & wParam) || (MK_MBUTTON & wParam))
			{
				// Normalize based on size of window and bounding sphere radius
				float fDeltaX = (m_ptLastMouse.x - iMouseX) * m_fRadiusTranslation / m_nWidth;
				float fDeltaY = (m_ptLastMouse.y - iMouseY) * m_fRadiusTranslation / m_nHeight;

				XMMATRIX mTranslationDelta;
				XMMATRIX mTranslation = XMLoadFloat4x4(&m_mTranslation);
				if (wParam & MK_LBUTTON)
				{
					mTranslationDelta = XMMatrixTranslation(-2 * fDeltaX, 2 * fDeltaY, 0.0f);
					mTranslation = XMMatrixMultiply(mTranslation, mTranslationDelta);
				}
				else  // wParam & MK_MBUTTON
				{
					mTranslationDelta = XMMatrixTranslation(0.0f, 0.0f, 5 * fDeltaY);
					mTranslation = XMMatrixMultiply(mTranslation, mTranslationDelta);
				}

				XMStoreFloat4x4(&m_mTranslationDelta, mTranslationDelta);
				XMStoreFloat4x4(&m_mTranslation, mTranslation);

				// Store mouse coordinate
				m_ptLastMouse.x = iMouseX;
				m_ptLastMouse.y = iMouseY;
			}
			return TRUE;
		}

		return FALSE;
	}

	// Functions to get/set state
	DirectX::XMMATRIX GetRotationMatrix() const
	{
		using namespace DirectX;
		XMVECTOR q = XMLoadFloat4(&m_qNow);
		return DirectX::XMMatrixRotationQuaternion(q);
	}
	DirectX::XMMATRIX GetTranslationMatrix() const { return DirectX::XMLoadFloat4x4(&m_mTranslation); }
	DirectX::XMMATRIX GetTranslationDeltaMatrix() const { return DirectX::XMLoadFloat4x4(&m_mTranslationDelta); }
	bool IsBeingDragged() const { return m_bDrag; }
	DirectX::XMVECTOR GetQuatNow() const { return DirectX::XMLoadFloat4(&m_qNow); }
	void SetQuatNow(_In_ DirectX::FXMVECTOR& q) { DirectX::XMStoreFloat4(&m_qNow, q); }

	static DirectX::XMVECTOR QuatFromBallPoints(_In_ DirectX::FXMVECTOR vFrom, _In_ DirectX::FXMVECTOR vTo)
	{
		using namespace DirectX;

		XMVECTOR dot = XMVector3Dot(vFrom, vTo);
		XMVECTOR vPart = XMVector3Cross(vFrom, vTo);
		return XMVectorSelect(dot, vPart, g_XMSelect1110);
	}

protected:
	DirectX::XMFLOAT4X4 m_mRotation;        // Matrix for arc ball's orientation
	DirectX::XMFLOAT4X4 m_mTranslation;     // Matrix for arc ball's position
	DirectX::XMFLOAT4X4 m_mTranslationDelta;// Matrix for arc ball's position

	POINT m_Offset;                         // window offset, or upper-left corner of window
	INT m_nWidth;                           // arc ball's window width
	INT m_nHeight;                          // arc ball's window height
	DirectX::XMFLOAT2 m_vCenter;            // center of arc ball 
	float m_fRadius;                        // arc ball's radius in screen coords
	float m_fRadiusTranslation;             // arc ball's radius for translating the target

	DirectX::XMFLOAT4 m_qDown;              // Quaternion before button down
	DirectX::XMFLOAT4 m_qNow;               // Composite quaternion for current drag
	bool m_bDrag;                           // Whether user is dragging arc ball

	POINT m_ptLastMouse;                    // position of last mouse point
	DirectX::XMFLOAT3 m_vDownPt;            // starting point of rotation arc
	DirectX::XMFLOAT3 m_vCurrentPt;         // current point of rotation arc

	DirectX::XMVECTOR ScreenToVector(_In_ float fScreenPtX, _In_ float fScreenPtY)
	{
		// Scale to screen
		float x = (fScreenPtX - m_Offset.x - m_nWidth / 2) / (m_fRadius * m_nWidth / 2);
		float y = (fScreenPtY - m_Offset.y - m_nHeight / 2) / (m_fRadius * m_nHeight / 2);

		float z = 0.0f;
		float mag = x * x + y * y;

		if (mag > 1.0f)
		{
			float scale = 1.0f / sqrtf(mag);
			x *= scale;
			y *= scale;
		}
		else
			z = sqrtf(1.0f - mag);

		return DirectX::XMVectorSet(x, y, z, 0);
	}
};

IArcBall::~IArcBall()
{
}

IArcBall* IArcBall::Create()
{
	return new CD3DArcBall();
}
