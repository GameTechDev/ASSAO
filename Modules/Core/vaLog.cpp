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

#include "vaLog.h"

#include "vaCoreIncludes.h"

using namespace VertexAsylum;

vaLog::vaLog( )
{
    m_lastAddedTime = 0;

    m_timer.Start();

    if( !m_outStream.Open( vaCore::GetExecutableDirectory( ) + L"log.txt", FileCreationMode::Create, FileAccessMode::Write, FileShareMode::Read ) )
    {
        // where does one log failure to open the log file?
        vaCore::DebugOutput( L"Unable to open log output file" );
    }
    else
    {
        // Using byte order marks: https://msdn.microsoft.com/en-us/library/windows/desktop/dd374101
        uint16 utf16LE = 0xFEFF;
        m_outStream.WriteValue<uint16>(utf16LE);
    }
}

vaLog::~vaLog( )
{
    m_timer.Stop();

    m_outStream.Close();
}

void vaLog::Add( const vaVector4 & color, const std::wstring & text )
{
    vaCore::DebugOutput( L"vaLog: " + text + L"\n" );

    {
        vaCriticalSectionScopeLock lock( m_criticalSection );

        time_t locTime = time( NULL );
        double now = m_timer.GetCurrentTimeDouble( );
        assert( now >= m_lastAddedTime );
        if( now > m_lastAddedTime )
            m_lastAddedTime = now;
        m_logEntries.push_back( Entry( color, text, locTime, m_lastAddedTime ) );

        if( m_outStream.IsOpen() )
        {
            char buff[64];
#pragma warning ( suppress: 4996 )
            strftime( buff, sizeof( buff ), "%H:%M:%S: ", localtime( &locTime ) );

            m_outStream.WriteTXT( vaStringTools::SimpleWiden(buff) + text + L"\r\n" );
        }
    }
}

void vaLog::Add( const char * messageFormat, ... )
{
    va_list args;
    va_start( args, messageFormat );
    std::string txt = vaStringTools::Format( messageFormat, args );
    va_end( args );

    Add( vaVector4( 0.0, 0.0, 0.0, 1.0 ), vaStringTools::SimpleWiden(txt) );
}

void vaLog::Add( const wchar_t * messageFormat, ... )
{
    va_list args;
    va_start( args, messageFormat );
    std::wstring txt = vaStringTools::Format( messageFormat, args );
    va_end( args );

    Add( vaVector4( 0.0, 0.0, 0.0, 1.0 ), txt );
}

void vaLog::Add( const vaVector4 & color, const char * messageFormat, ... )
{
    va_list args;
    va_start( args, messageFormat );
    std::string txt = vaStringTools::Format( messageFormat, args );
    va_end( args );

    Add( color, vaStringTools::SimpleWiden( txt ) );
}

void vaLog::Add( const vaVector4 & color, const wchar_t * messageFormat, ... )
{
    va_list args;
    va_start( args, messageFormat );
    std::wstring txt = vaStringTools::Format( messageFormat, args );
    va_end( args );

    Add( color, txt );
}

int vaLog::FindNewest( float maxAgeSeconds )
{
    vaCriticalSectionScopeLock lock( m_criticalSection );

    double now = m_timer.GetCurrentTimeDouble( );
    double searchTime = now - (double)maxAgeSeconds;

    assert( now >= m_lastAddedTime );

    // nothing to return?
    if( m_logEntries.back().SystemTime < searchTime )
        return (int)m_logEntries.size();

    // return all?
    if( m_logEntries.front( ).SystemTime >= searchTime )
        return 0;

    int currentIndex    = (int)m_logEntries.size()-1;
    int prevStepIndex   = currentIndex;
    int stepSize        = vaMath::Max( 1, currentIndex / 100 );
    while( true )
    {
        assert( m_logEntries[prevStepIndex].SystemTime >= searchTime );

        currentIndex = vaMath::Max( 0, prevStepIndex - stepSize );

        if( m_logEntries[currentIndex].SystemTime < searchTime )
        {
            if( stepSize == 1 )
                return currentIndex+1;
            currentIndex = prevStepIndex + stepSize;
            stepSize = (stepSize+1) / 2;
        }
        else
        {
            prevStepIndex = currentIndex;
        }
    }

    // shouldn't ever happen!
    assert( false );
    return -1;
}
