#include "pch.h"
#include "ShaderDefineManager.h"

#include "DX11Device.h"

#include "ShaderManager.h"
#include "ShaderDefine.h"

using namespace std;

class ShaderDefineManager : public IShaderDefineManager {
public:
	ShaderDefineManager(DX11Device* device)
		: device(device)
	{
		vs.reset(IVertexShaderManager::Create(device));
		ps.reset(IPixelShaderManager::Create(device));
	}

	~ShaderDefineManager()
	{
		for (auto it = sds.begin(); it != sds.end(); ++it)
		{
			delete it->second;
		}
		sds.clear();
	}

	void Release(const string& key)
	{
		auto it = sds.find(key);
		if (it != sds.end())
		{
			delete it->second;
			sds.erase(it);
		}
	}

	void Load(
		const string& key, 
		const wstring& fileName,
		D3D11_INPUT_ELEMENT_DESC* layout,
		UINT layoutCount)
	{
		Release(key);

		auto sd = new ShaderDefine(
			key, 
			device->g_pd3dDevice,
			vs.get(),
			ps.get(),
			fileName,
			layout,
			layoutCount, 
			logger);

		sds.insert(make_pair(key, sd));
	}

	void Reload()
	{
		vs->Reload(logger);
		ps->Reload(logger);
	}

	bool Set(const string& key)
	{
		auto it = sds.find(key);
		if (it != sds.end())
		{
			return it->second->Set(device->immDevCtx, vs.get(), ps.get());
		}
	}

	void SetCompileLogger(IShaderCompileLog* log)
	{
		logger = log;
	}

private:
	DX11Device* device;
	unique_ptr<IVertexShaderManager> vs;
	unique_ptr<IPixelShaderManager> ps;
	map<string, ShaderDefine*> sds;
	IShaderCompileLog* logger;
};

IShaderDefineManager::~IShaderDefineManager()
{}

IShaderDefineManager* IShaderDefineManager::Create(DX11Device* device)
{ return new ShaderDefineManager(device); }