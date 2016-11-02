///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016, Intel Corporation
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation 
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of 
// the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
// SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// File changes (yyyy-mm-dd)
// 2016-09-07: filip.strugar@intel.com: first commit (extracted from VertexAsylum codebase, 2006-2016 by Filip Strugar)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core/vaCoreIncludes.h"

#ifndef UNICODE
#error "DXUT requires a Unicode build."
#endif

#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d11_2.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment( lib, "d3dcompiler.lib" )

//#define DXUT_AUTOLIB
//
//// #define DXUT_AUTOLIB to automatically include the libs needed for DXUT 
//#ifdef DXUT_AUTOLIB
//#pragma comment( lib, "comctl32.lib" )
//#pragma comment( lib, "dxguid.lib" )
//#pragma comment( lib, "ole32.lib" )
//#pragma comment( lib, "uuid.lib" )
//#endif
//
//#pragma warning( disable : 4100 ) // disable unreferenced formal parameter warnings for /W4 builds


