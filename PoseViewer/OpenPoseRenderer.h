#pragma once

#include <DX11Device.h>

#include "RenderContext.h"
#include "SceneDescriptor.h"

#include "OpenPose.h"

class IOpenPoseRenderer {
public:
	virtual ~IOpenPoseRenderer();

	virtual void Render(
		const OpenPose::Frame& frame,
		const SceneDescriptor& sceneDesc) = 0;

	static IOpenPoseRenderer* Create(RenderContext* context);
};