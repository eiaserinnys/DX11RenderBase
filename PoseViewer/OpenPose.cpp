#include "pch.h"
#include "OpenPose.h"

#include <rapidjson.h>
#include <document.h>

#include <WindowsUtility.h>
#include <Utility.h>

using namespace std;
using namespace rapidjson;
using namespace DirectX;

class OpenPose_ {
public:
	OpenPose::Frame frame;

	OpenPose_(const string& fileName)
	{
		auto buffer = LoadJsonFile(fileName);

		int count = 0;

		// JSON으로 파싱한다
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

			// 파일을 통째로 읽는다
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

			// 이름
			auto& pose = v["pose_keypoints"];

			if (pose.Size() != 54)
			{
				throw invalid_argument("bone size different");
			}

			int offset = 0;

			XMFLOAT3 total(0, 0, 0);

			for (Value::ConstValueIterator k = pose.Begin(); k != pose.End(); )
			{
				frame.pos[offset].x = k->GetFloat(); k++;
				frame.pos[offset].y = k->GetFloat(); k++;
				frame.pos[offset].z = k->GetFloat(); k++;

				total = total + frame.pos[offset];

				offset++;
			}

			frame.com = total / (float) (pose.Size() / 3);
		}
	}
};

void OpenPose::Load(const string& fileName, Frame& frame)
{
	OpenPose_ pose(fileName);

	frame = pose.frame;
}

