#pragma once

#include <string>

class DX11Device;
class DX11VertexShader;

class ITextureManager {
public:
	virtual ~ITextureManager();

	virtual std::pair<
		ID3D11Resource*,
		ID3D11ShaderResourceView*>	Get(const std::string& fileName) = 0;

	virtual void UnloadTexture(const std::string& fileName) = 0;

	virtual void Reload() = 0;

	static ITextureManager* Create(ID3D11Device* d3dDev);
};