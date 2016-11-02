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

namespace VertexAsylum
{
    inline vaSemaphore::vaSemaphore( )
    {
        m_semaphore = NULL;
    }

    inline vaSemaphore::vaSemaphore( const int32 initialCount, const int32 maxCount )
    {
        m_semaphore = NULL;
        Initialize( initialCount, maxCount );
    }

    inline vaSemaphore::~vaSemaphore( )
    {
        BOOL ret = ::CloseHandle( m_semaphore );
        assert( ret ); // GetLastError()
        ret;
    }

    inline void vaSemaphore::Initialize( const int32 initialCount, const int32 maxCount )
    {
        if( IsInitialized( ) )
        {
            assert( false );
            return;
        }
        m_semaphore = CreateSemaphore( NULL, initialCount, maxCount, NULL );
        assert( m_semaphore != NULL ); // GetLastError()
    }

    inline bool vaSemaphore::IsInitialized( ) const
    {
        return m_semaphore != NULL;
    }

    inline void vaSemaphore::Release( uint32 releaseCount, uint32 * releasedCount )
    {
        assert( IsInitialized() );
        if( !ReleaseSemaphore( m_semaphore, releaseCount, (LPLONG)&releasedCount ) )
        {
            assert( false ); // GetLastError()
        }
    }

    inline ThreadingWaitResponseType vaSemaphore::Wait( uint32 waitTimeout )
    {
        assert( IsInitialized() );

        DWORD dwWaitResult = WaitForSingleObject( m_semaphore, waitTimeout );          // zero-second time-out interval
        switch( dwWaitResult )
        {
        case( WAIT_FAILED ): return TWRT_Failed;
        case( WAIT_OBJECT_0 ): return TWRT_Signaled_0;
        case( WAIT_TIMEOUT ): return TWRT_Timeout;
        }
        return TWRT_Failed;
    }

    inline ThreadingWaitResponseType vaSemaphore::WaitAll( vaPlatformSemaphoreType semaphores[], const int semaphoresCount, uint32 waitTimeout )
    {
        //assert( IsInitialized() );

        DWORD dwWaitResult = WaitForMultipleObjectsEx( semaphoresCount, semaphores, true, waitTimeout, false );
        if( dwWaitResult == WAIT_FAILED )   return TWRT_Failed;
        if( dwWaitResult == WAIT_TIMEOUT )  return TWRT_Timeout;
        return (ThreadingWaitResponseType)( ( dwWaitResult - WAIT_OBJECT_0 ) + TWRT_Signaled_0 );
    }

    inline ThreadingWaitResponseType vaSemaphore::WaitAny( vaPlatformSemaphoreType semaphores[], const int semaphoresCount, uint32 waitTimeout )
    {
        //assert( IsInitialized() );

        DWORD dwWaitResult = WaitForMultipleObjectsEx( semaphoresCount, semaphores, false, waitTimeout, false );
        if( dwWaitResult == WAIT_FAILED )   return TWRT_Failed;
        if( dwWaitResult == WAIT_TIMEOUT )  return TWRT_Timeout;
        return (ThreadingWaitResponseType)( ( dwWaitResult - WAIT_OBJECT_0 ) + TWRT_Signaled_0 );
    }

}