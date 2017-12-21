#pragma once

#include <DX11Device.h>

#include "RenderContext.h"
#include "SceneDescriptor.h"
#include "MmdPlayer.h"

struct DX11Render;

class IMmdBoneRenderer {
public:
	virtual ~IMmdBoneRenderer();

	virtual void Render(
		DX11Render* render, 
		const IMmdPlayer* player,
		const SceneDescriptor& sceneDesc) = 0;

	static IMmdBoneRenderer* Create(RenderContext* context);
};