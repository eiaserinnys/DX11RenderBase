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
protected:
	struct ContentDesc
	{
		wstring pathName;
		string entry;
		unique_ptr<ContentType> shader;
	};

public:
	//--------------------------------------------------------------------------
	ShaderManagerT(DX11Device* device)
		: device(device)
	{
		assert(device != nullptr);
	}

	//--------------------------------------------------------------------------
	~ShaderManagerT()
	{
		CleanUp();
	}

	//--------------------------------------------------------------------------
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

	//--------------------------------------------------------------------------
	bool Load(
		const string& key, 
		const wstring& pathName, 
		const string& entry, 
		IShaderCompileLog* log, 
		bool force)
	{
		ContentDesc* desc = nullptr;

		auto prev = contents.find(key);
		if (prev != contents.end())
		{
			// 이미 로드되어 있는 항목이다
			// 강제 리로드가 아니면 리턴하자
			if (!force) { return prev->second != nullptr; }

			desc = prev->second;
		}
		else
		{
			desc = new ContentDesc;
			desc->pathName = pathName;
			desc->entry = entry;

			contents.insert(make_pair(key, desc));
		}

		return LoadByDesc(desc, log);
	}

	//--------------------------------------------------------------------------
	bool LoadByDesc(ContentDesc* desc, IShaderCompileLog* log)
	{
		try
		{
			auto content = new ContentType(
				device->g_pd3dDevice,
				desc->pathName,
				desc->entry,
				log);

			desc->shader.reset(content);
		}
		catch (...)
		{
			return false;
		}

		return true;
	}

	//--------------------------------------------------------------------------
	void Reload(IShaderCompileLog* log)
	{
		for (auto it = contents.begin(); it != contents.end(); ++it)
		{
			LoadByDesc(it->second, log);
		}
	}

	//--------------------------------------------------------------------------
	ContentType* Find(const string& key)
	{
		auto found = contents.find(key);
		if (found != contents.end()) { return found->second->shader.get(); }
		return nullptr;
	}

protected:
	DX11Device* device = nullptr;

private:
	map<string, ContentDesc*>  contents;
};

//------------------------------------------------------------------------------
class VertexShaderManager :
	public ShaderManagerT<IVertexShaderManager, DX11VertexShader> {
public:
	typedef ShaderManagerT<IVertexShaderManager, DX11VertexShader> ParentType;

	VertexShaderManager(DX11Device* device) : ParentType(device) {}

	void Set(const string& key)
	{
		auto found = Find(key);

		if (found != nullptr)
		{ 
			device->immDevCtx->VSSetShader(found->vs, NULL, 0);
		}
		else
		{
			device->immDevCtx->VSSetShader(NULL, NULL, 0);
		}
	}

	virtual DX11VertexShader* Find(const string& key)
	{
		return ParentType::Find(key);
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

	void Set(const string& key)
	{
		auto found = Find(key);

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