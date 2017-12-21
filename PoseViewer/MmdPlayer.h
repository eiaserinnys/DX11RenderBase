#pragma once

#include <DirectXMath.h>

#include "pmx.h"
#include "vmd.h"
#include "OpenPose.h"

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

	virtual void Update(OpenPose::Frame& frame, OpenPose::Frame& firstFrame) = 0;

	virtual pmx::PmxModel* GetModel() = 0;
	virtual vmd::VmdMotion* GetMotion() = 0;

	static IMmdPlayer* Create();
};