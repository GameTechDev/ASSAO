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

#include "Core/System/vaThread.h"


using namespace VertexAsylum;

namespace 
{
   struct Proxy
   {
      Proxy( vaThreadProcPtr threadProc, void * userData ) : threadProc(threadProc), userData(userData) {}
      vaThreadProcPtr         threadProc;
      void *                  userData;

      static DWORD WINAPI     ThreadProc( LPVOID lpThreadParameter );
   };
}


//class vaThreadWindows : public vaThread
//{
//
//   HANDLE         m_thread;
//
//
//protected:
//   friend vaThread * vaThread::Create( uint32 stackSize, vaThreadProcPtr procPtr, void * userData );
//   friend void vaThread::Destroy( vaThread * thread );
//
//   vaThreadWindows(uint32 stackSize, vaThreadProcPtr, void * userData);
//   ~vaThreadWindows();
//
//public:
//
//   virtual void                           Resume();
//   virtual bool                           WaitExit( uint32 timeout = 0xFFFFFFFF );
//
//   virtual vaThreadingSyncObjectHandle    GetSyncObjectHandle()   { return m_thread; }
//
//   virtual void                           SetPriority( ThreadPriority priority );
//   virtual ThreadPriority                 GetPriority( )                            { return m_priority; }
//
//   
//};
//

vaThread::vaThread()
{
   m_platformObject     = NULL;
   m_priority           = TP_Normal;
   m_dontWaitOnDestroy  = true;
}

vaThread::~vaThread()
{
   if( m_platformObject != NULL )
      Destroy( );
}

bool vaThread::Create( uint32 stackSize, vaThreadProcPtr procPtr, void * userData, bool createSuspended, ThreadPriority priority, bool dontWaitOnDestroy )
{
   m_platformObject     = ::CreateThread( NULL, stackSize, Proxy::ThreadProc, new Proxy(procPtr, userData), CREATE_SUSPENDED, NULL );
   assert( m_platformObject != NULL );
   m_priority           = priority;
   m_dontWaitOnDestroy  = dontWaitOnDestroy;

   SetPriority( m_priority );

   if( !createSuspended )
      Resume();

   return true;
}

void vaThread::Destroy( )
{
   if( m_platformObject != NULL )
   {
      // maybe this isn't the best way...
      if( !m_dontWaitOnDestroy )
          WaitExit( INFINITE );
      CloseHandle( m_platformObject );
   }
   m_platformObject = NULL;
}

bool vaThread::IsCreated( )
{
   return m_platformObject != NULL;
}

bool vaThread::WaitExit( uint32 timeout )
{
   assert( IsCreated() );

   bool retVal = ::WaitForSingleObject( m_platformObject, timeout ) == WAIT_OBJECT_0;

   if( retVal )
   {
      CloseHandle( m_platformObject );
      m_platformObject = NULL;
   }
   return retVal;
}

void vaThread::SetPriority( ThreadPriority priority )
{
   m_priority = priority;
   switch( m_priority )
   {
   case( TP_BelowNormal ): ::SetThreadPriority( m_platformObject, THREAD_PRIORITY_BELOW_NORMAL );  break;
   case( TP_Normal ):      ::SetThreadPriority( m_platformObject, THREAD_PRIORITY_NORMAL );        break;
   case( TP_AboveNormal ): ::SetThreadPriority( m_platformObject, THREAD_PRIORITY_ABOVE_NORMAL );  break;
   default: assert( false );
   }
}

void vaThread::Resume( )
{
   assert( IsCreated() );
   ::ResumeThread( m_platformObject );
}

void vaThread::Sleep( uint32 milliseconds )
{
   ::Sleep( milliseconds );
}

void vaThread::YieldProcessor( )
{
    ::YieldProcessor( );
}

DWORD WINAPI Proxy::ThreadProc( LPVOID lpThreadParameter )
{
   Proxy * proxy = static_cast<Proxy*>(lpThreadParameter);

   uint32 retVal = proxy->threadProc(proxy->userData);
   delete proxy;
   return retVal;
}

typedef BOOL( WINAPI *LPFN_GLPI )(
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION,
    PDWORD );


// Helper function to count set bits in the processor mask.
DWORD CountSetBits( ULONG_PTR bitMask )
{
    DWORD LSHIFT = sizeof( ULONG_PTR ) * 8 - 1;
    DWORD bitSetCount = 0;
    ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;
    DWORD i;

    for( i = 0; i <= LSHIFT; ++i )
    {
        bitSetCount += ( ( bitMask & bitTest ) ? 1 : 0 );
        bitTest /= 2;
    }

    return bitSetCount;
}

int vaThread::GetCPULogicalCoreCount( )
{
    // how difficult could it be to get logical core count?
    // well, apparently, according to https://msdn.microsoft.com/en-us/library/windows/desktop/ms683194%28v=vs.85%29.aspx,  difficult:

    LPFN_GLPI glpi;
    BOOL done = FALSE;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
    DWORD returnLength = 0;
    DWORD logicalProcessorCount = 0;
    DWORD numaNodeCount = 0;
    DWORD processorCoreCount = 0;
    DWORD processorL1CacheCount = 0;
    DWORD processorL2CacheCount = 0;
    DWORD processorL3CacheCount = 0;
    DWORD processorPackageCount = 0;
    DWORD byteOffset = 0;
    PCACHE_DESCRIPTOR Cache;

    glpi = (LPFN_GLPI)GetProcAddress(
        GetModuleHandle( TEXT( "kernel32" ) ),
        "GetLogicalProcessorInformation" );
    if( NULL == glpi )
    {
        _tprintf( TEXT( "\nGetLogicalProcessorInformation is not supported.\n" ) );
        assert( false );
        return 1;
    }

    while( !done )
    {
        DWORD rc = glpi( buffer, &returnLength );

        if( FALSE == rc )
        {
            if( GetLastError( ) == ERROR_INSUFFICIENT_BUFFER )
            {
                if( buffer )
                    free( buffer );

                buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(
                    returnLength );

                if( NULL == buffer )
                {
                    _tprintf( TEXT( "\nError: Allocation failure\n" ) );
                    assert( false );
                    return 1;
                }
            }
            else
            {
                _tprintf( TEXT( "\nError %d\n" ), GetLastError( ) );
                assert( false );
                return 1;
            }
        }
        else
        {
            done = TRUE;
        }
    }

    ptr = buffer;

    while( byteOffset + sizeof( SYSTEM_LOGICAL_PROCESSOR_INFORMATION ) <= returnLength )
    {
        switch( ptr->Relationship )
        {
        case RelationNumaNode:
            // Non-NUMA systems report a single record of this type.
            numaNodeCount++;
            break;

        case RelationProcessorCore:
            processorCoreCount++;

            // A hyperthreaded core supplies more than one logical processor.
            logicalProcessorCount += CountSetBits( ptr->ProcessorMask );
            break;

        case RelationCache:
            // Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache. 
            Cache = &ptr->Cache;
            if( Cache->Level == 1 )
            {
                processorL1CacheCount++;
            }
            else if( Cache->Level == 2 )
            {
                processorL2CacheCount++;
            }
            else if( Cache->Level == 3 )
            {
                processorL3CacheCount++;
            }
            break;

        case RelationProcessorPackage:
            // Logical processors share a physical package.
            processorPackageCount++;
            break;

        default:
            _tprintf( TEXT( "\nError: Unsupported LOGICAL_PROCESSOR_RELATIONSHIP value.\n" ) );
            break;
        }
        byteOffset += sizeof( SYSTEM_LOGICAL_PROCESSOR_INFORMATION );
        ptr++;
    }

    //_tprintf( TEXT( "\nGetLogicalProcessorInformation results:\n" ) );
    //_tprintf( TEXT( "Number of NUMA nodes: %d\n" ),
    //    numaNodeCount );
    //_tprintf( TEXT( "Number of physical processor packages: %d\n" ),
    //    processorPackageCount );
    //_tprintf( TEXT( "Number of processor cores: %d\n" ),
    //    processorCoreCount );
    //_tprintf( TEXT( "Number of logical processors: %d\n" ),
    //    logicalProcessorCount );
    //_tprintf( TEXT( "Number of processor L1/L2/L3 caches: %d/%d/%d\n" ),
    //    processorL1CacheCount,
    //    processorL2CacheCount,
    //    processorL3CacheCount );
    //
    //
    free( buffer );

    return logicalProcessorCount;
}


