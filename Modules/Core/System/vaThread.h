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

#include "../vaCore.h"

#include "System\vaPlatformThreading.h"


namespace VertexAsylum
{
    // Enums
    enum ThreadingWaitResponseType
    {
        TWRT_Failed = 0xFFFFFFFF,
        TWRT_Signaled_0 = 0,
        TWRT_Timeout = 258L,
    };

    // Typedefs
    typedef uint32( *vaThreadProcPtr )( void * pUserData );

    class vaThread
    {
    public:
        enum ThreadPriority
        {
            // left room for more granular priority control...
            TP_AboveNormal = 50,
            TP_Normal = 100,
            TP_BelowNormal = 150
        };

    protected:
        vaPlatformThreadType                m_platformObject;

        ThreadPriority                      m_priority;

        bool                                m_dontWaitOnDestroy;

    public:
        vaThread( );
        ~vaThread( );

    public:

        bool                                Create( uint32 stackSize, vaThreadProcPtr, void * userData, bool createSuspended = true, ThreadPriority priority = TP_Normal, bool dontWaitOnDestroy = true );
        void                                Destroy( );
        bool                                IsCreated( );

        void                                Resume( );
        bool                                WaitExit( uint32 timeout = 0xFFFFFFFF );
        bool                                Join( ) { return WaitExit(); }

        void                                SetPriority( ThreadPriority priority );
        ThreadPriority                      GetPriority( )                                  { return m_priority; }

        static void                         Sleep( uint32 milliseconds );
        static void                         YieldProcessor( );
        static int                          GetCPULogicalCoreCount(  );

        static int32                        Interlocked_Increment( int32 volatile * _Addend );
    };
}

#include "System\vaPlatformThreading.inl"
