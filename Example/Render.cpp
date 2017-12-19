#include "pch.h"

#include <stdio.h>

#include <DirectXTex.h>

#include "Global.h"
#include "Render.h"

using namespace std;
using namespace DirectX;

extern auto_ptr<GlobalContext> global;

IToRender::~IToRender()
{
}

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
DX11Render::DX11Render(HWND hwnd)
	: hwnd(hwnd)
{
	//lastTime = timeGetTime();
}

//------------------------------------------------------------------------------
void DX11Render::Render(RenderTuple* tuples, int count, bool wireframe)
{
	if (tuples != nullptr && count > 0)
	{
		RenderInternal(tuples, count, BakeFlag::None, wireframe);
	}
}

//------------------------------------------------------------------------------
void DX11Render::Bake(IToRender* render, const string& srcTexture, const wstring& destTexture)
{
	RenderTuple tuple;
	tuple.render = render;
	tuple.wireframe = Wireframe::False;
	tuple.flag = RenderFlag::Textured;
	tuple.texture = srcTexture;

	auto flag = BakeFlag::Unwrap;

	Begin(flag);

	RenderInternal(&tuple, 1, flag, false);

	//End(flag);
	{
		ScratchImage img;

		HRESULT hr = DirectX::CaptureTexture(
			global->d3d11->g_pd3dDevice,
			global->d3d11->immDevCtx,
			global->d3d11->GetRenderTarget()->GetTexture(),
			img);

		if (SUCCEEDED(hr))
		{
			DirectX::SaveToTGAFile(*img.GetImage(0, 0, 0), destTexture.c_str());
		}
	}
}

//------------------------------------------------------------------------------
void DX11Render::UpsampleDepth(IToRender* render, vector<float>& buffer, bool fhd)
{
	RenderTuple tuple;
	tuple.render = render;
	tuple.wireframe = Wireframe::False;
	tuple.flag = RenderFlag::VertexColor;

	auto flag = fhd ? BakeFlag::DepthUpsample : BakeFlag::WLS;

	Begin(flag);

	RenderInternal(&tuple, 1, flag, false);

	//End(flag);
	{
		ScratchImage img;

		HRESULT hr = DirectX::CaptureTexture(
			global->d3d11->g_pd3dDevice,
			global->d3d11->immDevCtx,
			global->d3d11->GetRenderTarget()->GetTexture(),
			img);

		if (SUCCEEDED(hr))
		{
			auto image = img.GetImage(0, 0, 0);

			const float* byFloat = (const float*)image->pixels;

			auto width = global->d3d11->GetRenderTarget()->GetWidth();
			auto height = global->d3d11->GetRenderTarget()->GetHeight();

			buffer.resize(width * height);

			memcpy(&buffer[0], byFloat, sizeof(buffer[0]) * buffer.size());
		}
	}
}

//------------------------------------------------------------------------------
void DX11Render::Begin(BakeFlag::Value bake)
{
	switch (bake)
	{
	default:
	case BakeFlag::None:
		global->d3d11->SetScreenshotMode(DX11Device::RenderTarget::Backbuffer);
		break;

	case BakeFlag::Unwrap:
		global->d3d11->SetScreenshotMode(DX11Device::RenderTarget::ForUnwrap);
		break;

	case BakeFlag::DepthUpsample:
		global->d3d11->SetScreenshotMode(DX11Device::RenderTarget::ForUpsample);
		break;

	case BakeFlag::WLS:
		global->d3d11->SetScreenshotMode(DX11Device::RenderTarget::ForWls);
		break;
	}

	global->d3d11->RestoreRenderTarget();
	global->d3d11->ClearRenderTarget();
}

//------------------------------------------------------------------------------
void DX11Render::End()
{
	global->d3d11->g_pSwapChain->Present(0, 0);
}

//------------------------------------------------------------------------------
void DX11Render::RenderInternal(
	RenderTuple* tuple, int count, BakeFlag::Value bake, bool wireframe)
{
	// 포인트 클라우드 렌더링
	for (int i = 0; i < count; ++i)
	{
		RenderPointCloud_(tuple[i], wireframe);
	}
}

//------------------------------------------------------------------------------
void DX11Render::RenderPointCloud_(RenderTuple& tuple, bool wireframe)
{
	SceneDescriptor temp = sceneDesc;
	temp.world = XMLoadFloat4x4(&tuple.world);

	//MaterialOverride matOvr;
	//matOvr.wireframe = GetWireframe(tuple.wireframe, wireframe);
	//matOvr.noZCompare = tuple.noZCompare;

	//global->dxr->pcEffect->Begin(global->d3d11.get(), temp, matOvr);

	//global->dxr->pcEffect->SetMaterial(global->d3d11.get());

	//tuple.render->Render();

	//global->dxr->pcEffect->End(global->d3d11.get());
}

//------------------------------------------------------------------------------
void DX11Render::RenderBackground()
{
	global->d3d11->immDevCtx->ClearState();

	global->d3d11->RestoreRenderTarget();

	global->d3d11->immDevCtx->VSSetShader(global->dxr->quadVS->vs, NULL, 0);
	global->d3d11->immDevCtx->PSSetShader(global->dxr->quadPS->ps, NULL, 0);
#if (!SHOW_PRELIT)
	global->d3d11->SetTexture(0, "Textures/background.dds");
#else
	DX11Mesh& mesh = *global->dxr->mesh[0].get();
	global->d3d11->immDevCtx->PSSetShaderResources(0, 1, &mesh.litResult->srv);
#endif

	global->d3d11->immDevCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	global->d3d11->immDevCtx->Draw(3, 0);
}

