#pragma once

class IDX11RenderTarget;

class IRenderTargetManager {
public:
	virtual ~IRenderTargetManager();

	virtual void Restore() = 0;
	virtual void Restore(const std::string& name) = 0;

	virtual void Clear() = 0;

	virtual IDX11RenderTarget* GetCurrent() = 0;
	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;

	virtual void CreateGenericRenderTarget(
		const std::string& name,
		DXGI_FORMAT fmt, 
		int width, 
		int height) = 0;

	virtual void CreateGenericRenderTarget(
		const std::string& name,
		DXGI_FORMAT* fmt,
		int formatCount, 
		int width,
		int height) = 0;

	virtual void CreateDepthStencilTarget(
		const std::string& name,
		DXGI_FORMAT fmt,
		DXGI_FORMAT dsvFmt,
		DXGI_FORMAT srvFmt,
		int width,
		int height) = 0;

	virtual IDX11RenderTarget* GetRenderTarget(const std::string& name) = 0;

	virtual void ReleaseRenderTarget(const std::string& name) = 0;

	static IRenderTargetManager* Create(
		ID3D11Device* dev, 
		IDXGISwapChain* swapChain, 
		ID3D11DeviceContext* devCtx);
};