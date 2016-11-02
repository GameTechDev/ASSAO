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

// Types
#include "vaThreadPool.h"

using namespace VertexAsylum;


vaThreadPool::vaThreadPool( uint32 workerThreadCount, vaThread::ThreadPriority threadsPriority, uint32 stackSize )
 : m_availableJobs( 0, c_maxJobs*2 ), m_doneJobs( 0, c_maxJobs*2 ), m_activeJobCount( 0 )
{
   m_lastUsedJobID   = -1;
   for( int i = 0; i < _countof(m_jobs); i++ )
   {
      m_jobs[i].Flags = 0;
      m_jobs[i].JobID = i;
      m_jobs[i].ThreadPool = this;
      m_jobs[i].UserData0 = NULL;
      m_jobs[i].UserData1 = NULL;
   }

   {
      vaCriticalSectionScopeLock lock(m_containersAccessMutex);
      m_shutdownSignal = false;
   }

   m_threadCount = workerThreadCount;
   for( uint32 i = 0; i < m_threadCount; i++ )
   {
      m_threads[i].Create( stackSize, StaticThreadProc, this, false, threadsPriority );
   }
}

vaThreadPool::~vaThreadPool()
{
   WaitFinishAllTasks();
   
   {
      vaCriticalSectionScopeLock lock(m_containersAccessMutex);
      m_shutdownSignal = true;
   }

   m_availableJobs.Release(m_threadCount);

   for( uint32 i = 0; i < m_threadCount; i++ )
   {
        m_threads[i].WaitExit();
        m_threads[i].Destroy();
   } 
}

void vaThreadPool::InternalAddJob( const std::function < void( int ) > & jobProcFunctI, const std::function < void( void * ) > & jobProcFunctV, const std::function< void( int, int ) > & jobProcFunctII, const std::function < void( int, void * ) > & jobProcFunctIV, const std::function < void( void *, void * ) > & jobProcFunctVV, void * userData0, void * userData1, uint32 * poutJobID )
{
    int outJobID;
    {
        vaCriticalSectionScopeLock lock( m_containersAccessMutex );

        outJobID = GetNewJobID( );
        if( poutJobID != NULL )
            *poutJobID = outJobID;

        vaThreadPoolJobPrivateInfo newJob;
        newJob.JobID                    = outJobID;
        newJob.Flags                    = 0;
        newJob.JobProcFunctI            = jobProcFunctI;
        newJob.JobProcFunctV            = jobProcFunctV;
        newJob.JobProcFunctII           = jobProcFunctII;
        newJob.JobProcFunctIV           = jobProcFunctIV;
        newJob.JobProcFunctVV           = jobProcFunctVV;
        newJob.UserData0                = userData0;
        newJob.UserData1                = userData1;

        if( outJobID == c_invalidJobID )
        {
            // not enough storage for concurrent execution: execute in-place
            assert( false );
            newJob.Execute();
            return;
        }

        m_jobs[outJobID] = newJob;

        assert( ( m_jobs[outJobID].Flags & ( JF_Pending | JF_Active ) ) == 0 );
        m_jobs[outJobID].Flags |= JF_Pending;

        m_pendingJobs.push_back( outJobID );
    }

    // Added new job, release a thread from sleep
    m_availableJobs.Release( );
}

void vaThreadPool::AddJob( const std::function< void( int ) > & callback, int userData, uint32 * poutJobID )
{
#pragma warning( suppress : 4311 4312 4302 )
    InternalAddJob( callback, nullptr, nullptr, nullptr, nullptr, (void*)userData, NULL, poutJobID );
}

void vaThreadPool::AddJob( const std::function< void( void * ) > & callback, void * userData, uint32 * poutJobID  )
{
    InternalAddJob( nullptr, callback, nullptr, nullptr, nullptr, userData, NULL, poutJobID );
}

void vaThreadPool::AddJob( const std::function< void( int, int ) > & callback, int userData0, int userData1, uint32 * poutJobID )
{
#pragma warning( suppress : 4311 4312 4302 )
    InternalAddJob( nullptr, nullptr, callback, nullptr, nullptr, (void*)userData0, (void*)userData1, poutJobID );
}

void vaThreadPool::AddJob( const std::function< void( int, void * ) > & callback, int userData0, void * userData1, uint32 * poutJobID )
{
#pragma warning( suppress : 4311 4312 4302 )
    InternalAddJob( nullptr, nullptr, nullptr, callback, nullptr, (void*)userData0, userData1, poutJobID );
}

void vaThreadPool::AddJob( const std::function< void( void *, void * ) > & callback, void * userData0, void * userData1, uint32 * poutJobID )
{
    InternalAddJob( nullptr, nullptr, nullptr, nullptr, callback, userData0, userData1, poutJobID );
}

bool vaThreadPool::RemoveQueuedJob( uint32 jobID )
{
   assert( jobID != c_invalidJobID );

   vaCriticalSectionScopeLock lock(m_containersAccessMutex);

   std::deque<uint32>::iterator it = std::find( m_pendingJobs.begin(), m_pendingJobs.end(), jobID );
   if( it == m_pendingJobs.end() )
   {
      // not found
      return false;
   }
   else
   {
      // found, remove it
      *it = c_invalidJobID;
      return true;
   }
}

void vaThreadPool::PopLeaveExecuteReenterRelease( )
{
    uint32 jobID;
    jobID = m_pendingJobs.front( );
    m_pendingJobs.pop_front( );

    // Allowed to have c_invalidJobID-s in the queue (for example, a job was canceled)
    if( jobID == c_invalidJobID )
        return;

    vaThreadPoolJobPrivateInfo * jobInfo;

    //m_activeJobs.insert(jobID);
    //m_activeJobs.push_back(jobID);
    m_activeJobCount++;

    jobInfo = &m_jobs[jobID];
    assert( jobID == jobInfo->JobID );

    assert( ( jobInfo->Flags & JF_Pending ) != 0 );
    assert( ( jobInfo->Flags & JF_Active ) == 0 );
    jobInfo->Flags &= ~JF_Pending;
    jobInfo->Flags |= JF_Active;

    vaThreadPoolJobInfo localJobInfo = *static_cast<vaThreadPoolJobInfo*>( jobInfo );

    m_containersAccessMutex.Leave( );

    jobInfo->Execute( );

    m_containersAccessMutex.Enter( );

    // // std::vector<uint32>::iterator it = std::find( m_activeJobs.begin(), m_activeJobs.end(), jobInfo->JobID );
    // std::set<uint32>::iterator it = m_activeJobs.find(jobInfo->JobID);
    // assert( it != m_activeJobs.end() ); // job must be in, we've just finished it?
    // if( it != m_activeJobs.end() )
    // {
    //    m_activeJobs.erase( it );
    // }
    m_activeJobCount--;

    assert( ( jobInfo->Flags & JF_Active ) != 0 );
    jobInfo->Flags &= ~JF_Active;
    ReleaseJobID( jobInfo->JobID );
}

void vaThreadPool::WaitFinishAllTasks( bool joinWorking )
{
   for( ;; )
   {
      {
          m_containersAccessMutex.Enter( );
         if( (m_pendingJobs.size() == 0) && (m_activeJobCount == 0) ) //m_activeJobs.size() == 0 )
         {
             m_containersAccessMutex.Leave( );
             return;
         }
         if( joinWorking && (m_pendingJobs.size() > 0) )
             PopLeaveExecuteReenterRelease( );

         m_containersAccessMutex.Leave( );
      }

      vaThread::YieldProcessor();
   }
}

void vaThreadPool::ThreadProc()
{
   //byte * threadScratchBuffer = new byte[m_localThreadScratchBufferSize];

   for( ;; )
   {
      m_availableJobs.Wait();

      m_containersAccessMutex.Enter();
      
      if( m_shutdownSignal )
      {
         m_containersAccessMutex.Leave();
         break; // exits the thread
      }

      while( m_pendingJobs.size() != 0 )
      {
          PopLeaveExecuteReenterRelease( );
      }
      m_containersAccessMutex.Leave();
   }

//   delete[] threadScratchBuffer;
}

uint32 vaThreadPool::StaticThreadProc(void * pUserData)
{
   vaThreadPool * thisPtr = static_cast<vaThreadPool*>(pUserData);
   thisPtr->ThreadProc();
   return 0;
}

void vaThreadPool::GetCurrentJobCount( int & outPendingJobs, int & outActiveJobs )
{
   vaCriticalSectionScopeLock lock(m_containersAccessMutex);

   outPendingJobs = (int)m_pendingJobs.size();
   outActiveJobs  = m_activeJobCount; //(int)m_activeJobs.size();
}


struct ExecuteArrayAndWaitInfo
{
    std::function< void( int ) > *  Functions;

    vaSemaphore                     AllDoneSemaphore;

    long                            RemainingJobCount;

    ExecuteArrayAndWaitInfo( std::function< void( int ) > functions[], int jobCount ) : AllDoneSemaphore( 0, 1 ), Functions( functions ), RemainingJobCount( jobCount ) { }
};

static void ExecuteArrayAndWaitProc( int index, ExecuteArrayAndWaitInfo & info )
{
    std::function< void( int ) > & fptr = info.Functions[index];
    fptr( index );

    // gah, not platform independent - need to make it!
    long remainingJobs = ::InterlockedDecrement( &info.RemainingJobCount );

//    if( remainingJobs == 0 )
//        info.AllDoneSemaphore.Release( );
}

static void ExecuteArrayAndWaitProcInternal( void * param0, void * param1 )
{
#pragma warning( suppress : 4311 4312 4302 )
    int index = (int)param0;
    ExecuteArrayAndWaitInfo & info = *((ExecuteArrayAndWaitInfo*)param1);
    ExecuteArrayAndWaitProc( index, info );
}

void vaThreadPool::ExecuteArrayAndWait( std::function< void( int ) > functions[], int jobCount )
{
    if( jobCount == 0 )
        return;
    assert( jobCount > 0 );

    ExecuteArrayAndWaitInfo info( functions, jobCount );

    // dispatch jobCount-1
    for( size_t i = 0; i < jobCount-1; i++ )
    {
        AddJob( std::function<void(void*, void*)>(ExecuteArrayAndWaitProcInternal), (void*)i, (void*)&info );
    }
    // ...and do the last one ourselves
    ExecuteArrayAndWaitProc( jobCount-1, info );

    while( true )
    {
        if( ::InterlockedCompareExchange( &info.RemainingJobCount, 0, 0 ) == 0 )
            return;

        vaThread::YieldProcessor();
    }
}
