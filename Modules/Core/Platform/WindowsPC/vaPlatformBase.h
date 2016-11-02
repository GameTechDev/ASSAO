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

//#ifndef _WINDOWS_

//#define STRICT
// 
// // Works with Windows 2000 and later and Windows 98 or later
// #undef _WIN32_IE
// #undef WINVER
// #undef _WIN32_WINDOWS
// #undef _WIN32_WINNT
// #define WINVER         0x0500 
// #define _WIN32_WINDOWS 0x0410 
// #define _WIN32_WINNT   0x0500 

#define VC_EXTRALEAN		        // Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN         // Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include <tchar.h>

#include "crtdbg.h"

//#endif

namespace VertexAsylum
{
   class vaPlatformBase
   {
   public:
      static void       Initialize( );
      static void       Deinitialize( );
      static void       Error( const wchar_t * messageString );
      static void       Warning( const wchar_t * messageString );
      static void       DebugOutput( const wchar_t * message );
      static bool       MessageBoxYesNo( const wchar_t * titleString, const wchar_t * messageString );
   };

   class vaWindows
   {
   public:
      static void       SetMainHWND( HWND hWnd );
      static HWND       GetMainHWND( );
   };
}

