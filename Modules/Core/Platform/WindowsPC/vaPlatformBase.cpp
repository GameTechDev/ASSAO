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

#include "Core/vaCore.h"
#include "vaPlatformBase.h"

#include "Core/vaStringTools.h"

#include "System/vaPlatformSocket.h"
#include "Core/System/vaFileTools.h"

#include "Core/vaLog.h"

#include "assert.h"

bool evilg_inOtherMessageLoop_PreventTick = false;

namespace VertexAsylum
{
    //void vaNetworkManagerWin32_CreateManager();
}

using namespace VertexAsylum;

static HWND    g_mainWindow = NULL;

void vaWindows::SetMainHWND( HWND hWnd )
{
    g_mainWindow = hWnd;
}

HWND vaWindows::GetMainHWND( )
{
    return g_mainWindow;
}


void vaPlatformBase::Initialize( )
{
    srand( ( unsigned )::GetTickCount( ) );

    //vaNetworkManagerWin32_CreateManager();
}

void vaPlatformBase::Deinitialize( )
{

}

void vaPlatformBase::DebugOutput( const wchar_t * message )
{
    OutputDebugString( message );
}

void vaPlatformBase::Error( const wchar_t * messageString )
{
    DebugOutput( messageString );
    evilg_inOtherMessageLoop_PreventTick = true;
    //::MessageBoxW( NULL, messageString, L"Fatal error", MB_ICONERROR | MB_OK );
    VA_LOG_ERROR( messageString );
    evilg_inOtherMessageLoop_PreventTick = false;
    assert( false );
    exit( 1 );
}

void vaPlatformBase::Warning( const wchar_t * messageString )
{
    DebugOutput( messageString );
    evilg_inOtherMessageLoop_PreventTick = true;
    //::MessageBoxW( NULL, messageString, L"Warning", MB_ICONWARNING | MB_OK );
    VA_LOG_WARNING( messageString );
    evilg_inOtherMessageLoop_PreventTick = false;
}

bool vaPlatformBase::MessageBoxYesNo( const wchar_t * titleString, const wchar_t * messageString )
{
    evilg_inOtherMessageLoop_PreventTick = true;
    int res = ::MessageBoxW( NULL, messageString, titleString, MB_ICONQUESTION | MB_YESNO );
    evilg_inOtherMessageLoop_PreventTick = false;

    return res == IDYES;
}

wstring vaCore::GetWorkingDirectory( )
{
    wchar_t buffer[4096];
    GetCurrentDirectory( _countof( buffer ), buffer );
    return wstring( buffer ) + L"\\";
}

wstring vaCore::GetExecutableDirectory( )
{
    wchar_t buffer[4096];

    GetModuleFileName( NULL, buffer, _countof( buffer ) );

    wstring outDir;
    vaStringTools::SplitPath( buffer, &outDir, NULL, NULL );

    return outDir;
}

string vaCore::GetCPUIDName( )
{
    // Get extended ids.
    int CPUInfo[4] = { -1 };
    __cpuid( CPUInfo, 0x80000000 );
    unsigned int nExIds = CPUInfo[0];

    // Get the information associated with each extended ID.
    char CPUBrandString[0x40] = { 0 };
    for( unsigned int i = 0x80000000; i <= nExIds; ++i )
    {
        __cpuid( CPUInfo, i );

        // Interpret CPU brand string and cache information.
        if( i == 0x80000002 )
        {
            memcpy( CPUBrandString,
                CPUInfo,
                sizeof( CPUInfo ) );
        }
        else if( i == 0x80000003 )
        {
            memcpy( CPUBrandString + 16,
                CPUInfo,
                sizeof( CPUInfo ) );
        }
        else if( i == 0x80000004 )
        {
            memcpy( CPUBrandString + 32, CPUInfo, sizeof( CPUInfo ) );
        }
    }

    return CPUBrandString;
}

bool vaFileTools::EnsureDirectoryExists( const wchar_t * path )
{
    //if( DirectoryExists( path ) )
    //   return true;

    int pathLength = (int)wcslen( path );

    const wchar_t separator = L'|';

    wchar_t * workPathStr = new wchar_t[pathLength + 3];

    wcscpy_s( workPathStr, pathLength + 3, path );

    for( int i = 0; i < pathLength; i++ )
    {
        if( ( workPathStr[i] == L'\\' ) || ( workPathStr[i] == L'/' ) )
            workPathStr[i] = separator;
    }

    wchar_t * nextSep;

    while( ( nextSep = wcschr( workPathStr, separator ) ) != 0 )
    {
        *nextSep = 0;
        if( wcslen( workPathStr ) > 2 || wcslen( workPathStr ) && workPathStr[1] != L':' ) ::CreateDirectoryW( workPathStr, 0 );
        *nextSep = L'\\';
    }

    {
        pathLength = (int)wcslen( workPathStr );
        if( workPathStr[1] != L':' && ( pathLength > 1 || pathLength && workPathStr[0] != L'\\' ) || pathLength > 3 || pathLength > 2 && workPathStr[2] != L'\\' )
            CreateDirectoryW( workPathStr, 0 );
    }
    delete[] workPathStr;

    return true; // DirectoryExists( path );
}
