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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// this is a very weird and incomplete implementation of a thread pool that needs a complete rework...
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core/vaCore.h"
#include "Core/vaSingleton.h"

#include "vaThread.h"
#include "vaCriticalSection.h"
#include "vaSemaphore.h"

#include <stack>
#include <deque>
#include <vector>
#include <set>
#include <algorithm>
#include <functional>

namespace VertexAsylum
{
    class vaThreadPool;

    struct vaThreadPoolJobInfo
    {
        vaThreadPool * ThreadPool;
        void *         UserData0;
        void *         UserData1;
        uint32         JobID;
    };

//    typedef void( *vaThreadPoolJobProcPtr )( const vaThreadPoolJobInfo & jobInfo );

    class vaThreadPool : public vaSingletonBase < vaThreadPool >
    {
    public:
        static const uint32           c_invalidJobID = (uint32)( -1 );

        static const uint32           c_maxPossibleThreads  = 64;

    private:
        static const int32            c_maxJobs             = 4096;

        enum JobFlag
        {
            JF_Pending = ( 1 << 0 ),
            JF_Active = ( 1 << 1 ),
        };

        //struct WaveInfo
        //{
        //    bool                    ItemFinishedTable[c_maxWaveSize];
        //    int                     ItemCount;
        //};

        struct vaThreadPoolJobPrivateInfo : public vaThreadPoolJobInfo
        {
            uint32                          Flags;
            std::function < void( int ) >
                                            JobProcFunctI;
            std::function< void( void * ) > 
                                            JobProcFunctV;
            std::function< void( int, int ) > 
                                            JobProcFunctII;
            std::function< void( int, void * ) > 
                                            JobProcFunctIV;
            std::function< void( void *, void * ) > 
                                            JobProcFunctVV;

            inline void                     Execute( )
            {
                // Do the job! (handle the different function callback types)
                if( this->JobProcFunctI )
#pragma warning( suppress : 4311 4302 )
                    this->JobProcFunctI( (int)this->UserData0 );
                else if( this->JobProcFunctV )
#pragma warning( suppress : 4311 4302 )
                    this->JobProcFunctV( this->UserData0 );
                else if( this->JobProcFunctII )
#pragma warning( suppress : 4311 4302 )
                    this->JobProcFunctII( (int)this->UserData0, (int)this->UserData1 );
                else if( this->JobProcFunctIV )
#pragma warning( suppress : 4311 4302 )
                    this->JobProcFunctIV( (int)this->UserData0, this->UserData1 );
                else if( this->JobProcFunctVV )
#pragma warning( suppress : 4311 4302 )
                    this->JobProcFunctVV( this->UserData0, this->UserData1 );
                else
                {
                    assert( false );
                }
            }
        };

        vaThread                      m_threads[c_maxPossibleThreads];
        uint32                        m_threadCount;

        vaThreadPoolJobPrivateInfo    m_jobs[c_maxJobs];
        int                           m_lastUsedJobID;              // starts at -1

        std::stack<uint32>            m_freeJobIDs;
        std::deque<uint32>            m_pendingJobs;

        int                           m_activeJobCount;
        //std::set<uint32>              m_activeJobs;

        vaSemaphore                   m_availableJobs;
        vaSemaphore                   m_doneJobs;
        vaCriticalSection             m_containersAccessMutex;      // for m_pendingJobs, m_activeJobs, m_jobs, m_lastUsedJobID, m_freeJobIDs, m_shutdownSignal

        bool                          m_shutdownSignal;

    public:
        vaThreadPool( uint32 workerThreadCount, vaThread::ThreadPriority threadsPriority, uint32 stackSize = 256 * 1024 );
        virtual ~vaThreadPool( );

        // just an easy way to cover most callback needs
        void                          AddJob( const std::function< void( int ) > & callback, int userData, uint32 * outJobID = NULL );
        void                          AddJob( const std::function< void( void * ) > & callback, void * userData, uint32 * outJobID = NULL );
        void                          AddJob( const std::function< void( int, int ) > & callback, int userData0, int userData1, uint32 * outJobID = NULL );
        void                          AddJob( const std::function< void( int, void * ) > & callback, int userData0, void * userData1, uint32 * outJobID = NULL );
        void                          AddJob( const std::function< void( void *, void * ) > & callback, void * userData0, void * userData1, uint32 * outJobID = NULL );

        void                          ExecuteArrayAndWait( std::function< void( int ) > functions[], int count );

        void                          WaitFinishAllTasks( bool joinWorking = false );

        void                          GetCurrentJobCount( int & outPendingJobs, int & outActiveJobs );

        //uint32                        GetLocalThreadScratchBufferSize( ) const      { return m_localThreadScratchBufferSize; }
        uint32                        GetThreadCount( ) const                       { return m_threadCount; }

    private:
        void                            InternalAddJob( const std::function < void( int ) > & jobProcFunctI, const std::function < void( void * ) > & jobProcFunctV, const std::function< void( int, int ) > & jobProcFunctII, const std::function < void( int, void * ) > & jobProcFunctIV, const std::function < void( void *, void * ) > & jobProcFunctVV, void * userData0, void * userData1, uint32 * outJobID );


        bool                            RemoveQueuedJob( uint32 jobID );

        inline uint32                   GetNewJobID( );
        inline void                     ReleaseJobID( uint32 jobID );

        void                            ThreadProc( );
        static uint32                   StaticThreadProc( void * pUserData );


        void                            PopLeaveExecuteReenterRelease( );

    };

    uint32 vaThreadPool::GetNewJobID( )
    {
        uint32 newJobID;
        if( m_freeJobIDs.size( ) != 0 )
        {
            newJobID = m_freeJobIDs.top( );
            m_freeJobIDs.pop( );
        }
        else
        {
            if( m_lastUsedJobID >= ( c_maxJobs - 1 ) )
            {
                assert( false ); // no more jobs available!
                return c_invalidJobID;
            }
            m_lastUsedJobID++;
            newJobID = m_lastUsedJobID;
        }

        // must be unused as we're reusing it
        assert( ( m_jobs[newJobID].Flags & ( JF_Active | JF_Pending ) ) == 0 );
        assert( newJobID != c_invalidJobID );
        return newJobID;
    }

    void vaThreadPool::ReleaseJobID( uint32 jobID )
    {
        // must be finished and not pending as we're releasing it
        assert( ( m_jobs[jobID].Flags & ( JF_Active | JF_Pending ) ) == 0 );

        if( (int32)jobID == m_lastUsedJobID )
        {
            do
            {
                if( m_lastUsedJobID < 0 )
                    break;

                m_lastUsedJobID--;
            } while( ( m_jobs[m_lastUsedJobID].Flags & JF_Active | JF_Pending ) == 0 );
        }
        else
        {
            m_freeJobIDs.push( jobID );
        }
    }

}
