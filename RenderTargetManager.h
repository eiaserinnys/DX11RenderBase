#pragma once

class IDX11RenderTarget;

class IRenderTargetManager {
public:
	virtual ~IRenderTargetManager();

	virtual void Restore() = 0;

	virtual void Clear() = 0;

	virtual IDX11RenderTarget* GetCurrent() = 0;
	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;

	static IRenderTargetManager* Create(
		ID3D11Device* dev, 
		IDXGISwapChain* swapChain, 
		ID3D11DeviceContext* devCtx);
};