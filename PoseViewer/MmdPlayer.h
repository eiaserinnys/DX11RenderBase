#pragma once

#include <DirectXMath.h>

#include "pmd.h"

class IMmdPlayer {
public:
	virtual ~IMmdPlayer();

	virtual const DirectX::XMFLOAT3* GetVertices() const = 0;
	virtual int GetVerticesCount() const = 0;

	virtual const int* GetIndices() const = 0;
	virtual int GetIndicesCount() const = 0;

	virtual const DirectX::XMMATRIX* GetWorldTransform() const = 0;
	virtual int GetWorldTransformCount() const = 0;

	virtual void Load() = 0;

	virtual void Update() = 0;

	static IMmdPlayer* Create();
};