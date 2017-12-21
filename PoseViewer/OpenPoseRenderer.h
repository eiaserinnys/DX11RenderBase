#pragma once

#include <DX11Device.h>

#include "RenderContext.h"
#include "SceneDescriptor.h"

#include "OpenPose.h"

class DX11Render;

class IOpenPoseRenderer {
public:
	virtual ~IOpenPoseRenderer();

	virtual void Render(
		DX11Render* render,
		const OpenPose::Frame& frame,
		const SceneDescriptor& sceneDesc) = 0;

	static IOpenPoseRenderer* Create(RenderContext* context);
};