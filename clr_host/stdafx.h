// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#  define STRICT
#endif

#ifndef WINVER
#  define WINVER 0x0400
#endif

#ifndef _WIN32_WINNT
#  define _WIN32_WINNT 0x0500
#endif

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

// disable code analysis warnings from standard headers
#pragma warning (push)
#pragma warning (disable: 6011 6054 6309 6386 6387 6535)
#include "resource.h"
#include <iostream>
#include <map>
#include <atlbase.h>
#include <atlcom.h>
#include <comdef.h>
#include <tchar.h>
#include <mscoree.h>
#import "Release/mysql_managed_interface.tlb" no_namespace
#pragma warning (pop)
#include <metahost.h>
#pragma comment(lib, "mscoree.lib")

using namespace ATL;
using namespace std;

_COM_SMARTPTR_TYPEDEF(ISupportErrorInfo, __uuidof(ISupportErrorInfo));
_COM_SMARTPTR_TYPEDEF(IErrorInfo, __uuidof(IErrorInfo));