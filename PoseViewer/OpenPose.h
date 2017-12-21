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

		std::vector<DirectX::XMMATRIX> localTx;
		std::vector<DirectX::XMFLOAT4> quat;

		DirectX::XMFLOAT3 com = DirectX::XMFLOAT3(0, 0, 0);
		DirectX::XMMATRIX comTx = DirectX::XMMatrixIdentity();
		DirectX::XMFLOAT4 comQuat = DirectX::XMFLOAT4(0, 0, 0, 1);

		void CalculateTx();

		void CalculateCom();

		void CalculateLink();
	};

	static void Load(const std::string& fileName, Frame& frame);
	static void Load(const std::string& fileName, Frame& frame, const Frame& refFrame);
};
