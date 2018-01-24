#include "pch.h"
#include "TextRenderer.h"

#include <SpriteBatch.h>
#include <SpriteFont.h>

using namespace std;
using namespace DirectX;

//------------------------------------------------------------------------------
class TextRenderer : public ITextRenderer {
public:
	TextRenderer(ID3D11Device* dev, ID3D11DeviceContext* devCtx)
	{
		spriteBatch.reset(new SpriteBatch(devCtx));

		font.reset(new SpriteFont(
			dev, 
			L"Font/font12.spritefont"));
	}

	void Enqueue(const TextToRender& text) 
	{ textToRender.push_back(text); }
	
	void Draw(const XMMATRIX& wvp_, const XMFLOAT2& extent)
	{
		if (!textToRender.empty())
		{
			spriteBatch->Begin();

			XMMATRIX wvp = XMMatrixTranspose(wvp_);

			for (auto tit = textToRender.begin(); tit != textToRender.end(); ++tit)
			{
				XMVECTOR pos;
				if (tit->is3d)
				{
					pos = XMVector3Transform(XMLoadFloat3(&tit->pos), wvp);

					pos.m128_f32[0] /= pos.m128_f32[3];
					pos.m128_f32[1] /= pos.m128_f32[3];
					pos.m128_f32[2] /= pos.m128_f32[3];
					pos.m128_f32[3] /= pos.m128_f32[3];
					pos.m128_f32[0] = (1 + pos.m128_f32[0]) / 2 * extent.x;
					pos.m128_f32[1] = (1 - pos.m128_f32[1]) / 2 * extent.y;

					pos = pos + XMLoadFloat2(&tit->ofs);
				}
				else
				{
					pos = XMLoadFloat2(&tit->ofs);
				}

				font->DrawString(
					spriteBatch.get(),
					tit->text.c_str(),
					pos,
					XMLoadFloat4(&tit->clr));
			}

			spriteBatch->End();

			textToRender.clear();
		}
	}

private:
	list<TextToRender> textToRender;

	unique_ptr<SpriteBatch> spriteBatch;
	unique_ptr<SpriteFont> font;
};

//------------------------------------------------------------------------------
ITextRenderer::~ITextRenderer()
{}

ITextRenderer* ITextRenderer::Create(ID3D11Device* dev, ID3D11DeviceContext* devCtx)
{
	return new TextRenderer(dev, devCtx);
}