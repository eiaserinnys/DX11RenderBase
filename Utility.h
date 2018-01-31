#pragma once

#include <stdio.h>
#include <stdarg.h>

#include <string>

#include <DirectXMath.h>

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

struct Utility
{
	static inline std::string Format(const char* format, ...)
	{
		char buffer[4096];
		va_list vaList;
		va_start(vaList, format);
		_vsnprintf_s(buffer, 4096, format, vaList);
		va_end(vaList);

		return buffer;
	}

	static inline std::wstring FormatW(const wchar_t* format, ...)
	{
		wchar_t buffer[4096];
		va_list vaList;
		va_start(vaList, format);
		_vsnwprintf_s(buffer, 4096, format, vaList);
		va_end(vaList);

		return buffer;
	}

	template <typename Value>
	static inline Value Square(Value a) { return a * a; }
};

namespace DirectX
{
	//------------------------------------------------------------------------------
	inline const XMFLOAT3 operator - (const XMFLOAT3& a)
	{
		return XMFLOAT3(-a.x, -a.y, -a.z);
	}

	//------------------------------------------------------------------------------
	inline XMFLOAT3 operator + (const XMFLOAT3& a, const XMFLOAT3& b)
	{
		XMFLOAT3 ret;
		ret.x = (a.x + b.x);
		ret.y = (a.y + b.y);
		ret.z = (a.z + b.z);
		return ret;
	}

	//------------------------------------------------------------------------------
	inline XMFLOAT2 operator - (const XMFLOAT2& a, const XMFLOAT2& b)
	{
		XMFLOAT2 ret;
		ret.x = (a.x - b.x);
		ret.y = (a.y - b.y);
		return ret;
	}

	inline XMFLOAT3 operator - (const XMFLOAT3& a, const XMFLOAT3& b)
	{
		XMFLOAT3 ret;
		ret.x = (a.x - b.x);
		ret.y = (a.y - b.y);
		ret.z = (a.z - b.z);
		return ret;
	}

	inline XMFLOAT4 operator - (const XMFLOAT4& a, const XMFLOAT4& b)
	{
		XMFLOAT4 ret;
		ret.x = (a.x - b.x);
		ret.y = (a.y - b.y);
		ret.z = (a.z - b.z);
		ret.w = (a.w - b.w);
		return ret;
	}

	//------------------------------------------------------------------------------
	inline const XMFLOAT3 operator * (const XMFLOAT3& a, float b)
	{
		return XMFLOAT3(a.x * b, a.y * b, a.z * b);
	}

	//------------------------------------------------------------------------------
	inline const XMFLOAT3 operator * (float b, const XMFLOAT3& a)
	{
		return XMFLOAT3(a.x * b, a.y * b, a.z * b);
	}

	//------------------------------------------------------------------------------
	inline const XMFLOAT3 operator / (const XMFLOAT3& a, float b)
	{
		return XMFLOAT3(a.x / b, a.y / b, a.z / b);
	}

	//------------------------------------------------------------------------------
	inline bool operator < (const XMFLOAT3& a, const XMFLOAT3& b)
	{
		if (a.x < b.x) { return true; }
		if (a.x == b.x)
		{
			if (a.y < b.y) { return true; }
			if (a.y == b.y) { return a.z < b.z; }
		}
		return false;
	}

	//------------------------------------------------------------------------------
	inline bool operator == (const XMFLOAT3& a, const XMFLOAT3& b)
	{
		return a.x == b.x && a.y == b.y && a.z == b.z;
	}

	//------------------------------------------------------------------------------
	inline XMFLOAT3 Add(const XMFLOAT3& a, const XMFLOAT3& b)
	{
		XMFLOAT3 ret;
		ret.x = (a.x + b.x);
		ret.y = (a.y + b.y);
		ret.z = (a.z + b.z);
		return ret;
	}

	//------------------------------------------------------------------------------
	inline float Dot(const XMFLOAT2& a, const XMFLOAT2& b)
	{
		return a.x * b.x + a.y * b.y;
	}

	//------------------------------------------------------------------------------
	inline float Dot(const XMFLOAT3& a, const XMFLOAT3& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	//------------------------------------------------------------------------------
	inline float Dot(const XMFLOAT4& a, const XMFLOAT4& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	}

	//------------------------------------------------------------------------------
	inline XMFLOAT3 Cross(const XMFLOAT3& a, const XMFLOAT3& b)
	{
		XMFLOAT3 v;
		v.x = a.y * b.z - a.z * b.y;
		v.y = a.z * b.x - a.x * b.z;
		v.z = a.x * b.y - a.y * b.x;
		return v;
	}

	//------------------------------------------------------------------------------
	inline XMFLOAT3 Normalize(const XMFLOAT3& a)
	{
		return a / sqrtf(Dot(a, a));
	}

	//------------------------------------------------------------------------------
	inline float Length(const XMFLOAT2& a)
	{ return sqrtf(Dot(a, a)); }

	inline float Length(const XMFLOAT3& a)
	{ return sqrtf(Dot(a, a)); }

	inline float Length(const XMFLOAT4& a)
	{ return sqrtf(Dot(a, a)); }

	//------------------------------------------------------------------------------
	inline float Distance(const XMFLOAT2& a, const XMFLOAT2& b)
	{ return Length(a - b); }

	inline float Distance(const XMFLOAT3& a, const XMFLOAT3& b)
	{ return Length(a - b); }

	inline float Distance(const XMFLOAT4& a, const XMFLOAT4& b)
	{ return Length(a - b); }

	//------------------------------------------------------------------------------
	inline XMFLOAT3 Transform(const XMFLOAT3& f3i, const XMMATRIX& m)
	{
		XMFLOAT3 f3o;
		XMStoreFloat3(&f3o, XMVector3Transform(XMLoadFloat3(&f3i), m));
		return f3o;
	}

	//------------------------------------------------------------------------------
	inline XMFLOAT3 TransformNormal(const XMFLOAT3& f3i, const XMMATRIX& m)
	{
		XMFLOAT3 f3o;
		XMStoreFloat3(&f3o, XMVector3TransformNormal(XMLoadFloat3(&f3i), m));
		return f3o;
	}

};