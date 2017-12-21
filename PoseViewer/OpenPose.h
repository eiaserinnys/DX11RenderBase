#pragma once

#include <string>
#include <vector>

#include <DirectXMath.h>

struct OpenPose {
public:
	struct Frame
	{
		Frame();

		DirectX::XMFLOAT3 pos[18];

		std::vector<DirectX::XMMATRIX> worldTx;
		std::vector<DirectX::XMMATRIX> worldInvTx;

		DirectX::XMFLOAT3 com = DirectX::XMFLOAT3(0, 0, 0);
		DirectX::XMMATRIX comTx;

		void CalculateTx();

		void CalculateCom();

		void CalculateLink();
	};

	static void Load(const std::string& fileName, Frame& frame);
};
