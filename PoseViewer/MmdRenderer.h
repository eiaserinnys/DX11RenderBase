#pragma once

#include <DX11Device.h>

#include "RenderContext.h"
#include "SceneDescriptor.h"

#include "OpenPose.h"

#include "Pmd.h"

class IMmdRenderer {
public:
	virtual ~IMmdRenderer();

	virtual void Render(
		const pmd::PmdModel* model,
		const SceneDescriptor& sceneDesc) = 0;

	static IMmdRenderer* Create(RenderContext* context);
};