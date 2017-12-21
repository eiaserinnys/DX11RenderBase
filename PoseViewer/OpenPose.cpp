#include "pch.h"
#include "OpenPose.h"

#include <rapidjson.h>
#include <document.h>

#include <WindowsUtility.h>
#include <Utility.h>

using namespace std;
using namespace rapidjson;
using namespace DirectX;

#if 0
static XMMATRIX GetPose(
	OpenPose::Frame& f,
	int pivot,
	int next,
	int ref)
{
	XMFLOAT3 front = f.pos[next] - f.pos[pivot];
	XMFLOAT3 left_ = f.pos[ref] - f.pos[pivot];
	XMFLOAT3 up = Cross(front, left_);
	XMFLOAT3 left = Cross(up, front);

	XMMATRIX bone = XMMatrixIdentity();
	if (Length(front) == 0 || Length(left) == 0 || Length(up) == 0)
	{
		bone = XMMatrixIdentity();
	}
	else
	{
		Normalize(front);
		Normalize(left);
		Normalize(up);

		bone.r[0].m128_f32[0] = front.x;
		bone.r[0].m128_f32[1] = front.y;
		bone.r[0].m128_f32[2] = front.z;

		bone.r[1].m128_f32[0] = left.x;
		bone.r[1].m128_f32[1] = left.y;
		bone.r[1].m128_f32[2] = left.z;

		bone.r[2].m128_f32[0] = up.x;
		bone.r[2].m128_f32[1] = up.y;
		bone.r[2].m128_f32[2] = up.z;

		//bone = XMMatrixLookAtLH(
		//	XMLoadFloat3(&f.pos[pivot]),
		//	XMLoadFloat3(&f.pos[next]),
		//	XMLoadFloat3(&up));
	}

	return bone;
}
#endif

OpenPose::Frame::Frame()
{
	for (int i = 0; i < COUNT_OF(pos); ++i)
	{
		pos[i] = XMFLOAT3(0, 0, 0);
	}
}

void OpenPose::Frame::CalculateLink()
{
	worldInvTx.resize(worldTx.size());
	for (int i = 0; i < worldTx.size(); ++i)
	{
		XMMATRIX base = worldTx[i];
		base.r[3].m128_f32[0] = 0;
		base.r[3].m128_f32[1] = 0;
		base.r[3].m128_f32[2] = 0;

		worldInvTx[i] = XMMatrixInverse(nullptr, base);
	}
}

void OpenPose::Frame::CalculateTx()
{
	CalculateCom();

	worldTx.resize(COUNT_OF(pos));

	XMMATRIX id = XMMatrixIdentity();

	for (int i = 0; i < worldTx.size(); ++i)
	{
		worldTx[i] = id;

		worldTx[i].r[3].m128_f32[0] = pos[i].x;
		worldTx[i].r[3].m128_f32[1] = pos[i].y;
		worldTx[i].r[3].m128_f32[2] = pos[i].z;

	}

	// �� ������ �ڽ��� ��Ʈ�� ����Ʈ�� ������,
	// ��Ʈ ������ �� �������� �ϴ� Ʈ�������� ����Ѵ�

	// �ϴ� �ٸ��� �غ���
	pair<int, int> p[] =
	{
		make_pair(2, 3),
		make_pair(3, 4),

		make_pair(5, 6),
		make_pair(6, 7),

		make_pair(8, 9),
		make_pair(9, 10),

		make_pair(11, 12),
		make_pair(12, 13),
	};

	for (int i = 0; i < COUNT_OF(p); ++i)
	{
		int pivot = p[i].first;

		XMMATRIX& targetTx = worldTx[p[i].second];

		XMFLOAT3 targetPos(
			targetTx.r[3].m128_f32[0],
			targetTx.r[3].m128_f32[1],
			targetTx.r[3].m128_f32[2]);
		XMFLOAT3 myPos = pos[pivot];

		XMFLOAT3 front(
			targetTx.r[2].m128_f32[0],
			targetTx.r[2].m128_f32[1],
			targetTx.r[2].m128_f32[2]);
		XMFLOAT3 up = myPos - targetPos;
		up = Normalize(up);

		XMFLOAT3 left = Cross(up, front);

		XMMATRIX& myTx = worldTx[pivot];

		myTx.r[0].m128_f32[0] = left.x;
		myTx.r[0].m128_f32[1] = left.y;
		myTx.r[0].m128_f32[2] = left.z;

		myTx.r[1].m128_f32[0] = up.x;
		myTx.r[1].m128_f32[1] = up.y;
		myTx.r[1].m128_f32[2] = up.z;

		myTx.r[2].m128_f32[0] = front.x;
		myTx.r[2].m128_f32[1] = front.y;
		myTx.r[2].m128_f32[2] = front.z;
	}
}

void OpenPose::Frame::CalculateCom()
{
	// ���� COM�� ����Ѵ�
	int pivotBone = 8;
	int leftBone = 11;
	int upBone = 1;
	XMFLOAT3 left = pos[leftBone] - pos[pivotBone];
	left = Normalize(left);
	XMFLOAT3 up_ = XMFLOAT3(0, 1, 0);//pos[upBone] - pos[pivotBone];
	up_ = Normalize(up_);
	XMFLOAT3 front = Cross(left, up_);
	XMFLOAT3 up = Cross(left, front);

	com = (pos[leftBone] + pos[pivotBone] + pos[upBone]) / 3;

	comTx.r[0].m128_f32[0] = -left.x;
	comTx.r[0].m128_f32[1] = -left.y;
	comTx.r[0].m128_f32[2] = -left.z;

	comTx.r[1].m128_f32[0] = -up.x;
	comTx.r[1].m128_f32[1] = -up.y;
	comTx.r[1].m128_f32[2] = -up.z;

	comTx.r[2].m128_f32[0] = -front.x;
	comTx.r[2].m128_f32[1] = -front.y;
	comTx.r[2].m128_f32[2] = -front.z;

	comTx.r[3].m128_f32[0] = com.x;
	comTx.r[3].m128_f32[1] = com.y;
	comTx.r[3].m128_f32[2] = com.z;
}

class OpenPose_ {
public:
	OpenPose::Frame frame;

	OpenPose_(const string& fileName)
	{
		auto buffer = LoadJsonFile(fileName);

		int count = 0;

		// JSON���� �Ľ��Ѵ�
		Document document;
		if (document.Parse(buffer.c_str()).HasParseError())
		{
			throw invalid_argument("Json Parsing Error");
		}
		else
		{
			for (auto it = document.MemberBegin(); it != document.MemberEnd(); ++it)
			{
				if (strcmp(it->name.GetString(), "people") == 0)
				{
					ParsePeople(it->value);
				}
			}
		}
	}

	string LoadJsonFile(const string& fileName)
	{
		FILE* jsonFile = nullptr;
		fopen_s(&jsonFile, fileName.c_str(), "rt");

		if (jsonFile != nullptr)
		{
			fseek(jsonFile, 0, SEEK_END);
			size_t sz = ftell(jsonFile);
			fseek(jsonFile, 0, SEEK_SET);

			// ������ ��°�� �д´�
			char* buffer = new char[sz + 1];
			memset(buffer, 0, sz + 1);
			fread(buffer, sizeof(char), sz, jsonFile);
			buffer[sz] = 0;

			fclose(jsonFile);

			return buffer;
		}
		else
		{
			throw invalid_argument("File Not Found");
		}
	}

	void ParsePeople(const Value& people)
	{
		if (people.Size() != 1)
		{
			WindowsUtility::Debug(L"peopne - %d\n", people.Size());
		}

		for (Value::ConstValueIterator
			it = people.Begin(); it != people.End(); ++it)
		{
			auto& v = *it;

			// �̸�
			auto& pose = v["pose_keypoints"];

			if (pose.Size() != 54)
			{
				throw invalid_argument("bone size different");
			}

			int offset = 0;

			XMFLOAT3 total(0, 0, 0);

			for (Value::ConstValueIterator k = pose.Begin(); k != pose.End(); )
			{
				float scale = 1000.0f;

				frame.pos[offset].x = - k->GetFloat() / scale; k++;
				frame.pos[offset].y = - k->GetFloat() / scale; k++;
				frame.pos[offset].z = k->GetFloat() / scale; k++;

				total = total + frame.pos[offset];

				offset++;
			}

			//frame.com = total / (float) (pose.Size() / 3);
		}
	}
};

void OpenPose::Load(const string& fileName, Frame& frame)
{
	OpenPose_ pose(fileName);

	frame = pose.frame;

	frame.CalculateTx();

	frame.CalculateLink();

	for (int i = 0; i < COUNT_OF(frame.pos); ++i)
	{
		frame.worldTx[i] = XMMatrixMultiply(
			frame.worldInvTx[i],
			frame.worldTx[i]);
	}
}

void OpenPose::Load(const string& fileName, Frame& frame, const Frame& refFrame)
{
	OpenPose_ pose(fileName);

	frame = pose.frame;

	frame.CalculateTx();

	// ���۷����� �ι����� ������!
	for (int i = 0; i < COUNT_OF(frame.pos); ++i)
	{
		frame.worldTx[i] = XMMatrixMultiply(
			refFrame.worldInvTx[i],
			frame.worldTx[i]);
	}

	XMMATRIX id = XMMatrixIdentity();

	vector<XMMATRIX> localTx;
	localTx.resize(frame.worldTx.size(), id);

	frame.quat.resize(frame.worldTx.size(), XMFLOAT4(0, 0, 0, 1));

	auto& calcLocal = [&](int pivot, const XMMATRIX& parTx)
	{
		localTx[pivot] = XMMatrixMultiply(
			frame.worldTx[pivot], 
			XMMatrixInverse(nullptr, parTx));

		auto v = XMQuaternionRotationMatrix(localTx[pivot]);

		frame.quat[pivot].x = v.m128_f32[0];
		frame.quat[pivot].y = v.m128_f32[1];
		frame.quat[pivot].z = v.m128_f32[2];
		frame.quat[pivot].w = v.m128_f32[3];
	};

	calcLocal(2, frame.comTx);
	calcLocal(3, frame.worldTx[2]);

	calcLocal(5, frame.comTx);
	calcLocal(6, frame.worldTx[5]);

	calcLocal(8, frame.comTx);
	calcLocal(9, frame.worldTx[8]);

	calcLocal(11, frame.comTx);
	calcLocal(12, frame.worldTx[11]);

	auto v = XMQuaternionRotationMatrix(frame.comTx);
	frame.comQuat.x = v.m128_f32[0];
	frame.comQuat.y = v.m128_f32[1];
	frame.comQuat.z = v.m128_f32[2];
	frame.comQuat.w = v.m128_f32[3];
}

