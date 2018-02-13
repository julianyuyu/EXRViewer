// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <wchar.h>
#include <string.h>
#include <windows.h>

#define WSTR_MATCH(wstr1, wstr2)	(!wcscmp((LPCWSTR)(wstr1), (LPCWSTR)(wstr2)))
#define WSTR_IMATCH(wstr1, wstr2)	(!_wcsicmp((LPCWSTR)(wstr1), (LPCWSTR)(wstr2)))
#define WSTR_EMPTY(wstr)	        WSTR_MATCH((wstr), L"")

#define SAFEDELETE(ptr)		do { if (ptr) { delete (ptr); (ptr) = nullptr; } } while(0)
#define SAFEDELETEARRAY(ptr)	do { if (ptr) { delete [] (ptr); (ptr) = nullptr; } } while(0)

#define SAFERELEASE(p)  { if(p) { (p)->Release(); (p)=nullptr; } }
