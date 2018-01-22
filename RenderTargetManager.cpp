#include "pch.h"
#include "RenderTargetManager.h"

#include "DX11Device.h"
#include "DX11RenderTarget.h"

class RenderTargetManager : public IRenderTargetManager {
public:
	//--------------------------------------------------------------------------
	RenderTargetManager(
		ID3D11Device* dev, 
		IDXGISwapChain* swapChain,
		ID3D11DeviceContext* devCtx)
		: dev(dev), devCtx(devCtx)
	{
		// Create a render target view
		backBuffer.reset(IDX11RenderTarget::Create_BackBuffer(dev, swapChain));

		curTarget = backBuffer.get();
	}

	//--------------------------------------------------------------------------
	void SelectBackBufferIfInvalid()
	{
		if (curTarget == nullptr)
		{
			curTarget = backBuffer.get();
		}
	}

	//--------------------------------------------------------------------------
	void Restore()
	{
		SelectBackBufferIfInvalid();

		D3D11_VIEWPORT vp;

		curTarget->SetRenderTarget(devCtx);
		vp.Width = (FLOAT)curTarget->GetWidth();
		vp.Height = (FLOAT)curTarget->GetHeight();

		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		devCtx->RSSetViewports(1, &vp);
	}

	//--------------------------------------------------------------------------
	void Clear()
	{
		SelectBackBufferIfInvalid();

		//float ClearColor[4] = { 0.25f, 0.25f, 0.25f, 0.0f }; // red, green, blue, alpha
		float ClearColor[4] = { 1, 1, 1, 0.0f }; // red, green, blue, alpha

		curTarget->Clear(devCtx, ClearColor, 1.0f, 0);
	}

	//--------------------------------------------------------------------------
	IDX11RenderTarget* GetCurrent()
	{
		SelectBackBufferIfInvalid();

		return curTarget;
	}

	//--------------------------------------------------------------------------
	int GetWidth()
	{
		SelectBackBufferIfInvalid();
		return curTarget->GetWidth();
	}

	//--------------------------------------------------------------------------
	int GetHeight()
	{
		SelectBackBufferIfInvalid();
		return curTarget->GetHeight();
	}

	std::unique_ptr<IDX11RenderTarget>	backBuffer;

	IDX11RenderTarget* curTarget = nullptr;
	ID3D11Device* dev = nullptr;
	ID3D11DeviceContext* devCtx = nullptr;
};

//------------------------------------------------------------------------------
IRenderTargetManager::~IRenderTargetManager()
{}

//------------------------------------------------------------------------------
IRenderTargetManager* IRenderTargetManager::Create(
	ID3D11Device* dev, 
	IDXGISwapChain* swapChain, 
	ID3D11DeviceContext* devCtx)
	{ return new RenderTargetManager(dev, swapChain, devCtx); }