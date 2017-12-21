#pragma once

#include <DX11Device.h>

#include "RenderContext.h"
#include "SceneDescriptor.h"
#include "MmdPlayer.h"

class IMmdRenderer {
public:
	virtual ~IMmdRenderer();

	virtual void Render(
		const IMmdPlayer* player,
		const SceneDescriptor& sceneDesc) = 0;

	static IMmdRenderer* Create(RenderContext* context);
};