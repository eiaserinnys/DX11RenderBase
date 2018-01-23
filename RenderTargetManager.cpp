#include "pch.h"
#include "RenderTargetManager.h"

#include "DX11Device.h"
#include "DX11RenderTarget.h"

using namespace std;

class RenderTargetManager : public IRenderTargetManager {
public:
	//--------------------------------------------------------------------------
	RenderTargetManager(
		ID3D11Device* dev, 
		IDXGISwapChain* swapChain,
		ID3D11DeviceContext* devCtx)
		: dev(dev), devCtx(devCtx)
	{
		backBuffer.reset(IDX11RenderTarget::Create_BackBuffer(dev, swapChain));
		SelectBackBuffer();
	}

	//--------------------------------------------------------------------------
	~RenderTargetManager()
	{
		SelectBackBuffer();
		Restore();

		for (auto it = rts.begin(); it != rts.end(); ++it)
		{
			delete it->second;
		}
		rts.clear();
	}

	//--------------------------------------------------------------------------
	void CreateGenericRenderTarget(
		const string& name,
		DXGI_FORMAT fmt, 
		int width, 
		int height)
	{
		ReleaseRenderTarget(name);

		auto rt = IDX11RenderTarget::Create_GenericRenderTarget(dev, fmt, width, height);
		if (rt == nullptr)
		{
			throw runtime_error("No render target created");
		}

		rts.insert(make_pair(name, rt));
	}

	//--------------------------------------------------------------------------
	IDX11RenderTarget* GetRenderTarget(const string& name)
	{
		auto it = rts.find(name);
		if (it != rts.end())
		{
			return it->second;
		}
		return nullptr;
	}

	//--------------------------------------------------------------------------
	void ReleaseRenderTarget(const string& name)
	{
		auto it = rts.find(name);
		if (it != rts.end())
		{
			if (curTarget == it->second)
			{
				SelectBackBuffer();
			}

			delete it->second;

			rts.erase(it);
		}
	}

	//--------------------------------------------------------------------------
	void SelectBackBuffer()
	{ curTarget = backBuffer.get(); }

	//--------------------------------------------------------------------------
	void SelectBackBufferIfInvalid()
	{ if (curTarget == nullptr) { curTarget = backBuffer.get(); } }

	//--------------------------------------------------------------------------
	void Restore()
	{
		SelectBackBuffer();
		RestoreInternal();
	}

	//--------------------------------------------------------------------------
	void Restore(const string& name)
	{
		auto it = rts.find(name);
		if (it != rts.end())
		{
			curTarget = it->second;
			RestoreInternal();
		}
		else
		{
			throw runtime_error("The specified render target not found");
		}
	}

	//--------------------------------------------------------------------------
	void RestoreInternal()
	{
		curTarget->SetRenderTarget(devCtx);

		D3D11_VIEWPORT vp;
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

	unique_ptr<IDX11RenderTarget> backBuffer;

	map<string, IDX11RenderTarget*> rts;

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