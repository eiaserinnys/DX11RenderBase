#include "pch.h"
#include "TextureManager.h"

#include <DDSTextureLoader.h>
#include <DirectXTex.h>

using namespace std;
using namespace DirectX;

static wstring ToUnicode(const string& str)
{
	const size_t cSize = str.length() + 1;
	wstring wc(cSize, L'#');
	mbstowcs(&wc[0], str.c_str(), cSize);
	return wc;
}

class TextureManager : public ITextureManager {
public:
	//--------------------------------------------------------------------------
	TextureManager(ID3D11Device* d3dDev)
		: d3dDev(d3dDev)
	{}

	//--------------------------------------------------------------------------
	~TextureManager()
	{
		for (auto tit = textures.begin(); tit != textures.end(); ++tit)
		{
			if (tit->second.first != NULL) { tit->second.first->Release(); }
			if (tit->second.second != NULL) { tit->second.second->Release(); }
		}
	}

	//--------------------------------------------------------------------------
	void UnloadTexture(const string& fileName)
	{
		auto tit = textures.find(fileName);
		if (tit != textures.end())
		{
			tit->second.first->Release();
			tit->second.second->Release();
			textures.erase(tit);
		}
	}

	//--------------------------------------------------------------------------
	void Reload()
	{
		for (auto tit = textures.begin(); tit != textures.end(); ++tit)
		{
			if (tit->second.first != nullptr) { tit->second.first->Release(); }
			tit->second.first = NULL;

			if (tit->second.second != nullptr) { tit->second.second->Release(); }
			tit->second.second = NULL;

			tit->second = LoadTextureInternal(tit->first);
		}
	}

	//--------------------------------------------------------------------------
	pair<ID3D11Resource*, ID3D11ShaderResourceView*>
		Get(const string& fileName)
	{
		auto tit = textures.find(fileName);
		if (tit != textures.end())
		{
			return tit->second;
		}
		else
		{
			auto res = LoadTextureInternal(fileName);
			textures.insert(make_pair(fileName, res));
			return res;
		}
	}

	//--------------------------------------------------------------------------
	pair<ID3D11Resource*, ID3D11ShaderResourceView*>
		LoadTextureInternal(const std::string& fileName)
	{
		ID3D11Resource* texture;
		ID3D11ShaderResourceView* view;

		string extName;
		{
			string::size_type idx;

			idx = fileName.rfind('.');

			if (idx != std::string::npos)
			{
				extName = fileName.substr(idx + 1);
			}
			else
			{
				// No extension found
			}
		}

		if (_stricmp(extName.c_str(), "tga") == 0)
		{
			std::wstring ws(fileName.size(), L' '); // Overestimate number of code points.
			ws.resize(mbstowcs(&ws[0], fileName.c_str(), fileName.size())); // Shrink to fit.

			ScratchImage iimage;
			if (SUCCEEDED(LoadFromTGAFile(ws.c_str(), nullptr, iimage)))
			{
				if (SUCCEEDED(CreateTexture(
					d3dDev,
					iimage.GetImages(),
					iimage.GetImageCount(),
					iimage.GetMetadata(),
					&texture)))
				{
					if (SUCCEEDED(d3dDev->CreateShaderResourceView(
						texture, nullptr, &view)))
					{
						return make_pair(texture, view);
					}

					// 여기에 올 수가 있나...싶지만?
					texture->Release();
				}
			}
		}
		else
		{
			if (SUCCEEDED(CreateDDSTextureFromFile(
				d3dDev, ToUnicode(fileName).c_str(), &texture, &view)))
			{
				return make_pair(texture, view);
			}
		}

		return make_pair(nullptr, nullptr);
	}

	pair<ID3D11Resource*, ID3D11ShaderResourceView*>
		CreateVideoTexture(
			const string& name,
			int width,
			int height)
	{
		UnloadTexture(name);

		ID3D11Texture2D* texture = nullptr;
		ID3D11ShaderResourceView* view = nullptr;

		if (SUCCEEDED(d3dDev->CreateTexture2D(
			&CD3D11_TEXTURE2D_DESC(
				DXGI_FORMAT_B8G8R8A8_UNORM,
				width,
				height,
				1,
				1,
				D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET),
			nullptr,
			&texture)))
		{
#if 0
			ID3D11Resource* resource;

			if (SUCCEEDED(texture->QueryInterface(
				IID_ID3D11Resource,
				(void**)&resource)))
			{
				hr = g_pd3dDevice->CreateShaderResourceView(resource, nullptr, &view);
				if (SUCCEEDED(hr))
				{
					auto ret = make_pair(resource, view);
					textures.insert(make_pair(name, ret));
					return ret;
				}
			}

			texture->Release();
#endif
		}

		return make_pair(nullptr, nullptr);
	}

private:
	ID3D11Device* d3dDev;

	std::map<
		std::string,
		std::pair<
		ID3D11Resource*,
		ID3D11ShaderResourceView*>> textures;
};

ITextureManager::~ITextureManager()
{}

ITextureManager* ITextureManager::Create(ID3D11Device* d3dDev)
{ return new TextureManager(d3dDev); }