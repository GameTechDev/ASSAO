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

#include "Core/System/vaFileStream.h"

#include "Core/System/vaFileTools.h"

#include <stdio.h>
#include <io.h> 
   
#include <Commdlg.h>

using namespace VertexAsylum;

#pragma warning (disable : 4996)

static bool IsDots( const wchar_t * str )
{
   if( wcscmp( str, L"." ) && wcscmp( str, L".." ) ) 
      return false;
   return true;
}

#ifndef _S_IWRITE
   #define _S_IWRITE       0x0080          /* write permission, owner */
#endif

bool vaFileTools::DeleteDirectory( const wchar_t * path )
{
   HANDLE hFind;
   WIN32_FIND_DATA FindFileData;

   wchar_t DirPath[MAX_PATH];
   wchar_t FileName[MAX_PATH];

   wcscpy( DirPath, path );
   wcscat( DirPath, L"\\*" ); // searching all files
   wcscpy( FileName, path );
   wcscat( FileName, L"\\" );

   // find the first file
   hFind = FindFirstFile( DirPath, &FindFileData );
   if( hFind == INVALID_HANDLE_VALUE )
      return false;
   wcscpy(DirPath,FileName);

   bool bSearch = true;
   while( bSearch ) 
   {
      // until we find an entry
      if( FindNextFile(hFind,&FindFileData) )
      {
         if( IsDots(FindFileData.cFileName) )
            continue;
         wcscat( FileName,FindFileData.cFileName );
         if( ( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) 
         {
               // we have found a directory, recurse
               if( !DeleteDirectory(FileName) )
               {
                  FindClose(hFind);
                  return false; // directory couldn't be deleted
               }
               // remove the empty directory
               RemoveDirectory(FileName);
               wcscpy(FileName,DirPath);
         }
         else
         {
            if( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
               // change read-only file mode
               _wchmod( FileName, _S_IWRITE );

            if( !DeleteFile(FileName) ) 
            { 
               // delete the file
               FindClose(hFind);
               return false;
            }
            wcscpy(FileName,DirPath);
         }
      }
      else 
      {
         // no more files there
         if( GetLastError() == ERROR_NO_MORE_FILES )
            bSearch = false;
         else 
         {
            // some error occurred; close the handle and return FALSE
            FindClose( hFind );
            return false;
         }

      }

   }
   FindClose(hFind); // close the file handle

   return RemoveDirectory( path ) != 0; // remove the empty directory
}

bool vaFileTools::DirectoryExists( const wchar_t * path )
{
   DWORD attr = ::GetFileAttributes( path );

   if( attr == INVALID_FILE_ATTRIBUTES )
      return false;

   return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

wstring vaFileTools::OpenFileDialog( const wstring & initialFileName, const wstring & initialDir, const wchar_t * filter, int filterIndex, const wstring & dialogTitle)
{
    OPENFILENAME ofn ;

    ZeroMemory(&ofn, sizeof(ofn));

    wchar_t outBuffer[MAX_PATH];
    wcscpy_s( outBuffer, _countof( outBuffer ), initialFileName.c_str() );

    ofn.lStructSize     = sizeof(ofn);
    ofn.hwndOwner       = vaWindows::GetMainHWND();
    ofn.lpstrDefExt     = NULL;
    ofn.lpstrFile       = outBuffer;
    ofn.nMaxFile        = _countof( outBuffer );
    ofn.lpstrFilter     = filter;
    ofn.nFilterIndex    = filterIndex;
    ofn.lpstrInitialDir = initialDir.c_str();
    ofn.lpstrTitle      = dialogTitle.c_str();
    ofn.Flags           = OFN_FILEMUSTEXIST; // OFN_ALLOWMULTISELECT

    if( GetOpenFileName( &ofn ) )
    {
        return ofn.lpstrFile;
    }
    else
    {
        return L"";
    }
}

wstring vaFileTools::SaveFileDialog( const wstring & initialFileName, const wstring & initialDir, const wchar_t * filter, int filterIndex, const wstring & dialogTitle)
{
    OPENFILENAME ofn ;

    ZeroMemory(&ofn, sizeof(ofn));

    wchar_t outBuffer[MAX_PATH];
    wcscpy_s( outBuffer, _countof( outBuffer ), initialFileName.c_str() );

    ofn.lStructSize     = sizeof(ofn);
    ofn.hwndOwner       = vaWindows::GetMainHWND();
    ofn.lpstrDefExt     = NULL;
    ofn.lpstrFile       = outBuffer;
    ofn.nMaxFile        = _countof( outBuffer );
    ofn.lpstrFilter     = filter;
    ofn.nFilterIndex    = filterIndex;
    ofn.lpstrInitialDir = initialDir.c_str();
    ofn.lpstrTitle      = dialogTitle.c_str();
    ofn.Flags           = OFN_OVERWRITEPROMPT;

    if( GetSaveFileName( &ofn ) )
    {
        return ofn.lpstrFile;
    }
    else
    {
        return L"";
    }
}
