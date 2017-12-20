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
			// �̹� �ε�Ǿ� �ִ� �׸��̴�
			// ���� ���ε尡 �ƴϸ� ��������
			if (!force) { return prev->second != nullptr; }
		}

		try
		{
			auto content = new ContentType(
				device->g_pd3dDevice,
				pathName, 
				entry);

			// ���� �׸��� ������ ������
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
};

IVertexShaderManager::~IVertexShaderManager()
{}

IVertexShaderManager* IVertexShaderManager::Create(DX11Device* device)
{ return new VertexShaderManager(device); }