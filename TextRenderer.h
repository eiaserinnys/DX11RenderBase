#pragma once

#include <DirectXMath.h>

//------------------------------------------------------------------------------
struct TextToRender
{
	TextToRender(
		const DirectX::XMFLOAT3& p,
		const std::wstring& t,
		const DirectX::XMFLOAT4& c)
		: pos(p), ofs(0, 0), is3d(true), text(t), clr(c) {}

	TextToRender(
		const DirectX::XMFLOAT3& p,
		const DirectX::XMFLOAT2& o,
		const std::wstring& t,
		const DirectX::XMFLOAT4& c)
		: pos(p), ofs(o), is3d(true), text(t), clr(c) {}

	TextToRender(
		const DirectX::XMFLOAT2& p,
		const std::wstring& t,
		const DirectX::XMFLOAT4& c)
		: ofs(p), is3d(false), text(t), clr(c) {}

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 ofs;
	bool is3d;

	std::wstring text;
	DirectX::XMFLOAT4 clr;
};

//------------------------------------------------------------------------------
class ITextRenderer {
public:
	virtual ~ITextRenderer();

	virtual void Enqueue(const TextToRender& text) = 0;

	virtual void Draw(const DirectX::XMMATRIX& wvp, const DirectX::XMFLOAT2& extent) = 0;

	static ITextRenderer* Create(ID3D11Device* dev, ID3D11DeviceContext* devCtx);
};