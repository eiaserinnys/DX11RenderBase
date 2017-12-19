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
// 메모리 누수 검사용 블록
#if _DEBUG && CC_TARGET_PLATFORM == CC_PLATFORM_WIN32

#define _CRTDBG_MAP_ALLOC 
#include <crtdbg.h>
	
#ifndef DBG_NEW
	#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
	#define new DBG_NEW
#endif 

#endif

//-----------------------------------------------------------------------------
// 널리 쓰는 C/C++ 헤더
#include <string>
#include <vector>
#include <memory>

#include <stdint.h>
typedef uint8_t byte_t;

//-----------------------------------------------------------------------------
// 대단히 자주 쓰는 것들
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
