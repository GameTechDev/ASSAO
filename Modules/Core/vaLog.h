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

#include "vaCore.h"
#include "vaSingleton.h"
#include "vaGeometry.h"
#include "vaSTL.h"
#include <time.h>
#include <chrono>

#include "Core/System/vaSystemTimer.h"
#include "Core/System/vaCriticalSection.h"
#include "Core/System/vaFileStream.h"

namespace VertexAsylum
{

    // system-wide logging colors, can't think of a better place to put them
#define LOG_COLORS_NEUTRAL  (vaVector4( 0.8f, 0.8f, 0.8f, 1.0f ) )
#define LOG_COLORS_SUCCESS  (vaVector4( 0.0f, 0.8f, 0.2f, 1.0f ) )
#define LOG_COLORS_WARNING  (vaVector4( 0.8f, 0.8f, 0.1f, 1.0f ) )
#define LOG_COLORS_ERROR    (vaVector4( 1.0f, 0.1f, 0.1f, 1.0f ) )

    class vaLog : public vaSingletonBase< vaLog >
    {
    public:
        struct Entry
        {
            vaVector4           Color;
            wstring             Text;
            time_t              LocalTime;
            double              SystemTime;

            Entry( const vaVector4 & color, const std::wstring & text, time_t localTime, double appTime ) : Color( color ), Text( text ), LocalTime( localTime ), SystemTime( appTime ) { }
        };

        vector<Entry>           m_logEntries;
        double                  m_lastAddedTime;

        vaSystemTimer           m_timer;
        vaCriticalSection       m_criticalSection;

        vaFileStream            m_outStream;
        

    private:
        friend class vaCore;
        vaLog( );
        ~vaLog( );

    public:
        const vector<Entry> &   Entries( ) { return m_logEntries; }

        int                     FindNewest( float maxAgeSeconds );

        void                    Add( const char * messageFormat, ... );
        void                    Add( const wchar_t * messageFormat, ... );

        void                    Add( const vaVector4 & color, const char * messageFormat, ... );
        void                    Add( const vaVector4 & color, const wchar_t * messageFormat, ... );

#pragma warning ( suppress: 4996 )
        void                    Add( const vaVector4 & color, const std::wstring & text );

        vaCriticalSection &     CriticalSection( ) { return m_criticalSection; }

    private:
    };
    ////////////////////////////////////////////////////////////////////////////////////////////////

    // For measuring & logging occasional long taking tasks like loading of a level or similar. 
    // Not precise for short tasks and not useful for something that happens every frame (would 
    // spam the log)!
    class vaSimpleScopeTimerLog
    {
    private:
         clock_t        m_begin;
         vaVector4      m_color;
         wstring        m_info;
    public:
        vaSimpleScopeTimerLog( const wstring & info, const vaVector4 & color = LOG_COLORS_NEUTRAL )
        {
            m_info  = info;
            m_color = color;
            m_begin = clock();
            vaLog::GetInstance().Add( m_color, L"%s : starting...", m_info.c_str() );
        }
        ~vaSimpleScopeTimerLog( )
        {
            clock_t end = clock( );
            float elapsed_secs = (float)(double( end - m_begin ) / (double)CLOCKS_PER_SEC);
            vaLog::GetInstance().Add( m_color, L"%s : done, time taken %.3f seconds.", m_info.c_str(), elapsed_secs );
        }
    };


#define VA_LOG( format, ... )                   { vaLog::GetInstance().Add( LOG_COLORS_NEUTRAL, format, __VA_ARGS__ ); } 
#define VA_LOG_SUCCESS( format, ... )           { vaLog::GetInstance().Add( LOG_COLORS_SUCCESS, format, __VA_ARGS__ ); } 
#define VA_LOG_WARNING( format, ... )           { vaLog::GetInstance().Add( LOG_COLORS_WARNING, format, __VA_ARGS__ ); } 
#define VA_LOG_ERROR( format, ... )             { vaLog::GetInstance().Add( LOG_COLORS_ERROR, format, __VA_ARGS__ ); } 

#define VA_LOG_STACKINFO( format, ... )         { vaLog::GetInstance().Add( L"%s:%d : " format , VA_T(__FILE__), __LINE__, __VA_ARGS__); } 
#define VA_LOG_WARNING_STACKINFO( format, ... ) { vaLog::GetInstance().Add( LOG_COLORS_WARNING, L"%s:%d : " format , VA_T(__FILE__), __LINE__, __VA_ARGS__); } 
#define VA_LOG_ERROR_STACKINFO( format, ... )   { vaLog::GetInstance().Add( LOG_COLORS_ERROR, L"%s:%d : " format , VA_T(__FILE__), __LINE__, __VA_ARGS__); } 
}