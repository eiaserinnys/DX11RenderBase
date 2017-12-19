#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

#include <windows.h>

#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 )

#include <d3d11.h>
#include <d3dcompiler.h>

#include <memory>
#include <string>
#include <map>
#include <list>

#define _WINDOWS 1

//-----------------------------------------------------------------------------
// �޸� ���� �˻�� ���
#if _DEBUG && CC_TARGET_PLATFORM == CC_PLATFORM_WIN32

#define _CRTDBG_MAP_ALLOC 
#include <crtdbg.h>
	
#ifndef DBG_NEW
	#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
	#define new DBG_NEW
#endif 

#endif

//-----------------------------------------------------------------------------
// �θ� ���� C/C++ ���
#include <string>
#include <vector>
#include <memory>

#include <stdint.h>
typedef uint8_t byte_t;

//-----------------------------------------------------------------------------
// ����� ���� ���� �͵�
class INoncopyable
{
public:
	INoncopyable() = default;
	~INoncopyable() = default;

	INoncopyable(const INoncopyable&) = delete;
	void operator = (const INoncopyable&) = delete;
};

template <typename T, size_t N>
size_t countof(T(&arr)[N])
{
	return N;
}

#ifdef DX11RENDER_DLL
#define DX11RENDER_API 
#else
#define DX11RENDER_API 
#endif
