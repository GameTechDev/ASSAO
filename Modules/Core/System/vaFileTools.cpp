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

#include "vaFileTools.h"
#include "vaFileStream.h"
#include "Core/vaMemory.h"
#include "Core/System/vaMemoryStream.h"

#include "Core/vaStringTools.h"

#include <stdio.h>

using namespace VertexAsylum;

#pragma warning ( disable: 4996 )

std::map<wstring, vaFileTools::EmbeddedFileData> vaFileTools::s_EmbeddedFiles;

bool vaFileTools::FileExists( const wstring & path )
{
   FILE * fp = _wfopen( path.c_str(), L"rb" );
   if( fp != NULL )
   {
      fclose( fp );
      return true;
   }
   return false;
}

bool vaFileTools::DeleteFile( const wchar_t * path )
{
   int ret = ::_wremove( path );
   return ret == 0;
}

bool vaFileTools::MoveFile( const wchar_t * oldPath, const wchar_t * newPath )
{
   int ret = ::_wrename( oldPath, newPath );
   return ret == 0;
}

// // This should be deleted
// std::shared_ptr<vaMemoryBuffer> vaFileTools::LoadFileToMemoryBuffer( const wchar_t * fileName )
// {
//    vaFileStream file;
//    if( !file.Open( fileName, FileCreationMode::Open ) )
//       return std::shared_ptr<vaMemoryBuffer>( NULL );
// 
//    int64 length = file.GetLength();
// 
//    if( length == 0 )
//       return std::shared_ptr<vaMemoryBuffer>( NULL );
// 
//    std::shared_ptr<vaMemoryBuffer> ret( new vaMemoryBuffer( length ) );
// 
//    if( !file.Read( ret->GetData(), ret->GetSize() ) )
//       return std::shared_ptr<vaMemoryBuffer>( NULL );
// 
//    return ret;
// }

std::shared_ptr<vaMemoryStream> vaFileTools::LoadFileToMemoryStream( const wstring & fileName )
{
   vaFileStream file;
   if( !file.Open( fileName, FileCreationMode::Open ) )
      return std::shared_ptr<vaMemoryStream>( NULL );

   int64 length = file.GetLength();

   if( length == 0 )
      return std::shared_ptr<vaMemoryStream>( NULL );

   std::shared_ptr<vaMemoryStream> ret( new vaMemoryStream( length ) );

   if( !file.Read( ret->GetBuffer(), ret->GetLength() ) )
      return std::shared_ptr<vaMemoryStream>( NULL );

   return ret;
}

wstring vaFileTools::CleanupPath( const wstring & inputPath, bool convertToLowercase )
{
   wstring ret = inputPath; 
   if( convertToLowercase )
       ret = vaStringTools::ToLower( ret );

   wstring::size_type foundPos;
   while( (foundPos = ret.find(L"/")) != wstring::npos )
      ret.replace( foundPos, 1, L"\\" );
   while( (foundPos = ret.find(L"\\\\")) != wstring::npos )
      ret.replace( foundPos, 2, L"\\" );

   // restore network path
   if( (ret.length() > 0) && (ret[0] == '\\') )
       ret = L"\\"+ret;

   return ret;
}

wstring vaFileTools::GetAbsolutePath( const wstring & path )
{
   #define BUFSIZE 4096
   DWORD  retval=0;
   TCHAR  buffer[BUFSIZE]=TEXT(""); 
   TCHAR** lppPart={NULL};

   // Retrieve the full path name for a file. 
   // The file does not need to exist.
   retval = GetFullPathName( path.c_str(), BUFSIZE, buffer, lppPart);

   if (retval == 0)
   {
      VA_ASSERT_ALWAYS( L"Failed getting absolute path to '%s'", path.c_str() );
      return L"";
   }
   else
   {
      return wstring(buffer);
   }
}

static void FindFilesRecursive( const wstring & startDirectory, const wstring & searchName, bool recursive, std::vector<wstring> & outResult )
{
   WIN32_FIND_DATA FindFileData;
   HANDLE hFind;

   wstring combined = startDirectory + searchName;

   hFind = FindFirstFileExW( combined.c_str(), FindExInfoBasic, &FindFileData, FindExSearchNameMatch, NULL, 0 );
   bool bFound = hFind != INVALID_HANDLE_VALUE;
   while( bFound )
   {
      if(   ( (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) != 0 )
         && ( (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 ) )
         outResult.push_back( startDirectory + FindFileData.cFileName );
   
      bFound = FindNextFileW( hFind, &FindFileData ) != 0;
   }

   if( recursive )
   {
      combined = startDirectory + L"*";

      hFind = FindFirstFileExW( combined.c_str(), FindExInfoBasic, &FindFileData, FindExSearchLimitToDirectories, NULL, 0 );
      bool bFound = hFind != INVALID_HANDLE_VALUE;
      while( bFound )
      {
         if( (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 )
         {
            wstring subDir = FindFileData.cFileName;
            if( subDir != L"." && subDir != L".." )
            {
               FindFilesRecursive( startDirectory + subDir + L"\\", searchName, recursive, outResult );
            }
         }

         bFound = FindNextFileW( hFind, &FindFileData ) != 0;
      }

   }
}

std::vector<wstring> vaFileTools::FindFiles( const wstring & startDirectory, const wstring & searchName, bool recursive )
{
   std::vector<wstring> result;

   FindFilesRecursive( startDirectory, searchName, recursive, result );

   return result;
}

void                      vaFileTools::EmbeddedFilesRegister( const wstring & _pathName, byte * data, int64 dataSize, int64 timeStamp )
{
   // case insensitive
   wstring pathName = vaStringTools::ToLower( _pathName );

   std::map<wstring, EmbeddedFileData>::iterator it = s_EmbeddedFiles.find( pathName );

   if( it != s_EmbeddedFiles.end() )
   {
      VA_WARN( L"Embedded file %s already registered!", pathName.c_str() )
      return;
   }

   s_EmbeddedFiles.insert( std::pair<wstring, EmbeddedFileData>( pathName, 
      EmbeddedFileData( pathName, std::shared_ptr<vaMemoryStream>( new vaMemoryStream( data, dataSize ) ), timeStamp ) ) );
}

vaFileTools::EmbeddedFileData vaFileTools::EmbeddedFilesFind( const wstring & _pathName )
{
   // case insensitive
   wstring pathName = vaStringTools::ToLower( _pathName );

   std::map<wstring, EmbeddedFileData>::iterator it = s_EmbeddedFiles.find( pathName );

   if( it != s_EmbeddedFiles.end() )
      return it->second;

   return EmbeddedFileData();
}

void vaFileTools::Initialize( )
{

}

void vaFileTools::Deinitialize( )
{
   for( std::map<wstring, EmbeddedFileData>::iterator it = s_EmbeddedFiles.begin(); it != s_EmbeddedFiles.end(); it++ )
   {
      VA_ASSERT( it->second.MemStream.unique(), L"Embedded file %s reference count not 0, stream not closed but storage no longer guaranteed!", it->first.c_str() );
   }

   s_EmbeddedFiles.clear();
}
