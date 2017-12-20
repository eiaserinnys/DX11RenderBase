#pragma once

#include <string>
#include <DirectXMath.h>

struct OpenPose {
public:
	struct Frame
	{
		DirectX::XMFLOAT3 pos[18];

		DirectX::XMFLOAT3 com;
	};

	static void Load(const std::string& fileName, Frame& frame);
};
