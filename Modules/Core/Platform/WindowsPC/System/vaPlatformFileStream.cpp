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
#include "Core/vaStringTools.h"
#include "Core/vaLog.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////////////
// vaFileStream
//////////////////////////////////////////////////////////////////////////////
using namespace VertexAsylum;
//
vaFileStream::vaFileStream( )
{
    m_file = 0;
    m_accessMode = FileAccessMode::Default;
}
vaFileStream::~vaFileStream( void )
{
    Close( );
}
//
#pragma warning ( disable: 4996 )
//

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
static std::string GetLastErrorAsStringA( )
{
    //Get the error message, if any.
    DWORD errorMessageID = ::GetLastError( );
    if( errorMessageID == 0 )
        return std::string( ); //No error message has been recorded

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPSTR)&messageBuffer, 0, NULL );

    std::string message( messageBuffer, size );

    //Free the buffer.
    LocalFree( messageBuffer );

    return message;
}
//
//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
static std::wstring GetLastErrorAsStringW( )
{
    //Get the error message, if any.
    DWORD errorMessageID = ::GetLastError( );
    if( errorMessageID == 0 )
        return std::wstring( ); //No error message has been recorded

    LPWSTR messageBuffer = nullptr;
    size_t size = FormatMessageW( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPWSTR)&messageBuffer, 0, NULL );

    std::wstring message( messageBuffer, size );

    //Free the buffer.
    LocalFree( messageBuffer );

    return message;
}
//
bool vaFileStream::Open( const wchar_t * file_path, FileCreationMode::Enum creationMode, FileAccessMode::Enum accessMode, FileShareMode::Enum shareMode )
{
    if( IsOpen( ) ) return false;

    DWORD dwCreationDisposition = 0;

    if( creationMode == FileCreationMode::CreateNew )
    {
        if( vaFileTools::FileExists( file_path ) )
            return false;
        creationMode = FileCreationMode::Create;
    }

    if( creationMode == FileCreationMode::Create )
    {
        if( vaFileTools::FileExists( file_path ) )
            creationMode = FileCreationMode::Truncate;
    }

    if( ( accessMode == FileAccessMode::Read ) && ( creationMode == FileCreationMode::Create || creationMode == FileCreationMode::OpenOrCreate || creationMode == FileCreationMode::Truncate || creationMode == FileCreationMode::Append ) )
    {
        VA_ASSERT_ALWAYS( L"vaFileStream::Open - access mode and creation mode mismatch" );
        return false;
    }

    if( creationMode == FileCreationMode::OpenOrCreate )
    {
        if( vaFileTools::FileExists( file_path ) )
        {
            creationMode = FileCreationMode::Open;
            if( accessMode == FileAccessMode::Default )   accessMode = FileAccessMode::ReadWrite;
        }
        else
        {
            creationMode = FileCreationMode::Create;
        }
    }

    if( ( creationMode == FileCreationMode::Open ) && ( accessMode == FileAccessMode::Default ) )
        accessMode = FileAccessMode::Read;


    switch( creationMode )
    {
    case( FileCreationMode::Create ) : dwCreationDisposition = CREATE_ALWAYS;      break;
    case( FileCreationMode::Open ) : dwCreationDisposition = OPEN_EXISTING;      break;
    case( FileCreationMode::Append ) : dwCreationDisposition = OPEN_ALWAYS;        break;
    case( FileCreationMode::Truncate ) : dwCreationDisposition = TRUNCATE_EXISTING;  break;
    default: VA_ASSERT_ALWAYS( L"Incorrect creationMode parameter" );  return false;
    }

    DWORD dwDesiredAccess = 0;
    dwDesiredAccess |= ( ( accessMode & FileAccessMode::Read ) != 0 ) ? ( GENERIC_READ ) : ( 0 );
    dwDesiredAccess |= ( ( accessMode & FileAccessMode::Write ) != 0 ) ? ( GENERIC_WRITE ) : ( 0 );

    DWORD dwShareMode = 0;
    dwShareMode |= ( ( shareMode & FileShareMode::Read ) != 0 ) ? ( FILE_SHARE_READ ) : ( 0 );
    dwShareMode |= ( ( shareMode & FileShareMode::Write ) != 0 ) ? ( FILE_SHARE_WRITE ) : ( 0 );
    dwShareMode |= ( ( shareMode & FileShareMode::Delete ) != 0 ) ? ( FILE_SHARE_DELETE ) : ( 0 );


    m_file = ::CreateFileW( file_path, dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL );

    if( m_file == INVALID_HANDLE_VALUE )
    {
        wstring errorStr = GetLastErrorAsStringW( );
        VA_LOG( L"vaFileStream::Open( ""%s"", ... ): %s", file_path, errorStr.c_str( ) );
        return false;
    }

    m_accessMode = accessMode;

    if( creationMode == FileCreationMode::Append )
    {
        Seek( GetLength( ) );
    }

    return true;
}
//
bool vaFileStream::Open( const char * file_path, FileCreationMode::Enum creationMode, FileAccessMode::Enum accessMode, FileShareMode::Enum shareMode )
{
    wstring filePath = vaStringTools::SimpleWiden( file_path );
    return Open( filePath.c_str( ), creationMode, accessMode, shareMode );
}
//
void vaFileStream::Truncate( )
{
    VA_ASSERT( ( m_accessMode & FileAccessMode::Write ) != 0, L"File not opened for writing" );

    ::SetEndOfFile( m_file );
}
//
bool vaFileStream::Read( void * buffer, int64 count, int64 * outCountRead )
{
    VA_ASSERT( count > 0, L"count parameter must be > 0" );
    VA_ASSERT( ( m_accessMode & FileAccessMode::Read ) != 0, L"File not opened for reading" );
    VA_ASSERT( count < INT_MAX, L"File system current doesn't support reads bigger than INT_MAX" );

    DWORD dwRead;
    if( !::ReadFile( m_file, buffer, (DWORD)count, &dwRead, NULL ) )
        return false;

    if( outCountRead == NULL )
    {
        return count == (int)dwRead;
    }
    else
    {
        *outCountRead = dwRead;
        return dwRead > 0;
    }
}
//
bool vaFileStream::Write( const void * buffer, int64 count, int64 * outCountWritten )
{
    VA_ASSERT( count > 0, L"count parameter must be > 0" );
    VA_ASSERT( ( m_accessMode & FileAccessMode::Write ) != 0, L"File not opened for writing" );
    VA_ASSERT( count < INT_MAX, L"File system current doesn't support writes bigger than INT_MAX" );

    DWORD dwWritten;
    if( !::WriteFile( m_file, buffer, (DWORD)count, &dwWritten, NULL ) )
        return false;

    if( outCountWritten == NULL )
    {
        return (int)dwWritten == count;
    }
    else
    {
        *outCountWritten = dwWritten;
        return dwWritten > 0;
    }
}
//
void vaFileStream::Seek( int64 position )
{
    assert( position >= 0 );

    LARGE_INTEGER pos;
    pos.QuadPart = (LONGLONG)position;
    LARGE_INTEGER outPos;
    ::SetFilePointerEx( m_file, pos, &outPos, FILE_BEGIN );
}
//
void vaFileStream::Close( )
{
    if( m_file == NULL ) return;

    ::CloseHandle( m_file );
    m_file = NULL;
    m_accessMode = FileAccessMode::Default;
}
//
bool vaFileStream::IsOpen( )
{
    return m_file != NULL;
}
//
int64 vaFileStream::GetLength( )
{
    DWORD sizeHigh = 0;
    DWORD sizeLow = GetFileSize( m_file, &sizeHigh );

    return ( (int64)sizeHigh << 32 ) | sizeLow;
}
//
int64 vaFileStream::GetPosition( )
{
    LARGE_INTEGER pos;
    pos.QuadPart = 0;
    LARGE_INTEGER outPos;
    ::SetFilePointerEx( m_file, pos, &outPos, FILE_CURRENT );
    return (int64)outPos.QuadPart;
}
//////////////////////////////////////////////////////////////////////////////
