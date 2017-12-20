#include "pch.h"
#include "ShaderManager.h"

#include "DX11Device.h"
#include "DX11Shader.h"

#include <assert.h>

#undef new
#undef delete

#include <string>
#include <map>

using namespace std;

//------------------------------------------------------------------------------
template <typename BaseType, typename ContentType>
class ShaderManagerT : public BaseType {
public:
	ShaderManagerT(DX11Device* device)
		: device(device)
	{
		assert(device != nullptr);
	}

	~ShaderManagerT()
	{
		CleanUp();
	}

	void CleanUp()
	{
		for (auto c : contents)
		{
			if (c.second != nullptr)
			{
				delete c.second;
			}
		}

		contents.clear();
	}

	bool Load(const wstring& pathName, const string& entry, bool force)
	{
		auto prev = contents.find(pathName);
		if (prev != contents.end())
		{
			// 이미 로드되어 있는 항목이다
			// 강제 리로드가 아니면 리턴하자
			if (!force) { return prev->second != nullptr; }
		}

		try
		{
			auto content = new ContentType(
				device->g_pd3dDevice,
				pathName, 
				entry);

			// 기존 항목이 있으면 날리고
			if (prev != contents.end())
			{
				if (!prev->second) { delete prev->second; }
				contents.erase(prev);
			}

			contents.insert(make_pair(pathName, content));
		}
		catch (...)
		{
			return false;
		}

		return true;
	}

	ContentType* Find(const wstring& pathName)
	{
		auto found = contents.find(pathName);
		if (found != contents.end()) { return found->second; }
		return nullptr;
	}

protected:
	DX11Device* device = nullptr;

private:
	map<wstring, ContentType*>  contents;
};

//------------------------------------------------------------------------------
class VertexShaderManager :
	public ShaderManagerT<IVertexShaderManager, DX11VertexShader> {
public:
	typedef ShaderManagerT<IVertexShaderManager, DX11VertexShader> ParentType;

	VertexShaderManager(DX11Device* device) : ParentType(device) {}

	void Set(const wstring& pathName)
	{
		auto found = Find(pathName);

		if (found != nullptr)
		{ 
			device->immDevCtx->VSSetShader(found->vs, NULL, 0);
		}
		else
		{
			device->immDevCtx->VSSetShader(NULL, NULL, 0);
		}
	}

	virtual DX11VertexShader* Find(const std::wstring& pathName)
	{
		return ParentType::Find(pathName);
	}
};

IVertexShaderManager::~IVertexShaderManager()
{}

IVertexShaderManager* IVertexShaderManager::Create(DX11Device* device)
{ return new VertexShaderManager(device); }

//------------------------------------------------------------------------------
class PixelShaderManager :
	public ShaderManagerT<IPixelShaderManager, DX11PixelShader> {
public:
	typedef ShaderManagerT<IPixelShaderManager, DX11PixelShader> ParentType;

	PixelShaderManager(DX11Device* device) : ParentType(device) {}

	void Set(const wstring& pathName)
	{
		auto found = Find(pathName);

		if (found != nullptr)
		{
			device->immDevCtx->PSSetShader(found->ps, NULL, 0);
		}
		else
		{
			device->immDevCtx->PSSetShader(NULL, NULL, 0);
		}
	}
};

IPixelShaderManager::~IPixelShaderManager()
{}

IPixelShaderManager* IPixelShaderManager::Create(DX11Device* device)
{ return new PixelShaderManager(device); }