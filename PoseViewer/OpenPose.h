#pragma once

#include <string>
#include <vector>

#include <DirectXMath.h>

struct OpenPose {
public:
	struct Frame
	{
		DirectX::XMFLOAT3 pos[18];

		DirectX::XMFLOAT3 com = DirectX::XMFLOAT3(0, 0, 0);

		std::vector<DirectX::XMMATRIX> worldTx;

		void CalculateTx();
	};

	static void Load(const std::string& fileName, Frame& frame);
};
