#pragma once

#include <DirectXMath.h>

class IMmdPlayer {
public:
	virtual ~IMmdPlayer();

	virtual const DirectX::XMFLOAT3* GetVertices() const = 0;
	virtual int GetVerticesCount() const = 0;

	virtual const uint16_t* GetIndices() const = 0;
	virtual int GetIndicesCount() const = 0;

	virtual void Load() = 0;

	virtual void Update() = 0;

	static IMmdPlayer* Create();
};