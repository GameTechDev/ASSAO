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


#include "Core/vaCoreIncludes.h"

#include <vector>

// maybe use/integrate with http://brofiler.com/ ? 
// https://github.com/bombomby/brofiler/wiki

namespace VertexAsylum
{
    class vaGPUTimer;

    enum class vaProfilerTimingsDisplayType : int32
    {
        LastFrame,
        LastXFramesAverage,
        LastXFramesMax,
    };

    class vaNestedProfilerNode
    {
    public:
        static const int                c_historyFrameCount     = 96;

    private:
        string const                    m_name;
        bool                            m_selected;

        vaNestedProfilerNode * const    m_parentNode;
        map< string, vaNestedProfilerNode * >
                                        m_childNodes;
        int                             m_childNodeUsageFrameCounter;
        static const int                c_totalMaxChildNodes    = 1000;     // not designed for more than this for performance reasons

        double                          m_startTimeCPU;        // 0 if not started
        double                          m_totalTimeCPU;
        double                          m_exclusiveTimeCPU;

        unique_ptr<vaGPUTimer>          m_GPUProfiler;
        double                          m_totalTimeGPU;
        double                          m_exclusiveTimeGPU;

        int64                           m_lastUsedProfilerFrameIndex;
        int                             m_nodeUsageIndex;

        int                             m_framesUntouched;

        static int                      s_totalNodes;
        static const int                c_totalMaxNodes         = 10000;    // not designed for more than this, would become too slow

        bool                            m_hasGPUTimings;

        int                             m_historyLastIndex;
        double                          m_historyTotalTimeCPU[c_historyFrameCount];
        double                          m_historyTotalTimeGPU[c_historyFrameCount];
        double                          m_historyExclusiveTimeCPU[c_historyFrameCount];
        double                          m_historyExclusiveTimeGPU[c_historyFrameCount];
        double                          m_averageTotalTimeCPU;
        double                          m_averageTotalTimeGPU;
        double                          m_averageExclusiveTimeCPU;
        double                          m_averageExclusiveTimeGPU;
        double                          m_maxTotalTimeCPU;
        double                          m_maxTotalTimeGPU;
        double                          m_maxExclusiveTimeCPU;
        double                          m_maxExclusiveTimeGPU;
            
        static const int                c_untouchedFramesKeep = c_historyFrameCount * 3;

    protected:
        friend class vaProfiler;
        vaNestedProfilerNode( vaNestedProfilerNode * parentNode, const string & name, void * gpuAPIContext, bool selected );
        ~vaNestedProfilerNode( );

    protected:
        vaNestedProfilerNode *          StartScope( const string & name, void * gpuAPIContext, double currentTime, int64 profilerFrameIndex, bool newNodeSelectedDefault );
        void                            StopScope( double currentTime );
        //
    protected:
        void                            RemoveOrphans( );
        void                            Proccess( );
        void                            Display( const string & namePath, int depth, bool cpu, vaProfilerTimingsDisplayType displayType );
        //
    public:
        // Warning: node returned here is only guaranteed to remain valud until next vaProfiler::NewFrame( ) gets called
        const vaNestedProfilerNode *    FindSubNode( const string & name ) const;

        int                             GetFrameHistoryLength( ) const              { return c_historyFrameCount;       }

        // Warning: memory returned here is only guaranteed to remain valud until next vaProfiler::NewFrame( ) gets called
        const double *                  GetFrameHistoryTotalTileCPU( ) const        { return m_historyTotalTimeCPU;     }
        const double *                  GetFrameHistoryTotalTimeGPU( ) const        { return m_historyTotalTimeGPU;     }
        const double *                  GetFrameHistoryExclusiveTimeCPU( ) const    { return m_historyExclusiveTimeCPU; }
        const double *                  GetFrameHistoryExclusiveTimeGPU( ) const    { return m_historyExclusiveTimeGPU; }
        
        // last valid index value in the circular buffer 
        int                             GetFrameHistoryLastIndex( ) const           { return m_historyLastIndex; }

        double                          GetFrameLastTotalTimeCPU( ) const           { return m_historyTotalTimeCPU[     (m_historyLastIndex + c_historyFrameCount - 1) % c_historyFrameCount ]; }
        double                          GetFrameLastTotalTimeGPU( ) const           { return m_historyTotalTimeGPU[     (m_historyLastIndex + c_historyFrameCount - 1) % c_historyFrameCount ]; }
        double                          GetFrameLastExclusiveTimeCPU( ) const       { return m_historyExclusiveTimeCPU[ (m_historyLastIndex + c_historyFrameCount - 1) % c_historyFrameCount ]; }
        double                          GetFrameLastExclusiveTimeGPU( ) const       { return m_historyExclusiveTimeGPU[ (m_historyLastIndex + c_historyFrameCount - 1) % c_historyFrameCount ]; }
        double                          GetFrameAverageTotalTimeCPU( ) const        { return m_averageTotalTimeCPU;     }
        double                          GetFrameAverageTotalTimeGPU( ) const        { return m_averageTotalTimeGPU;     }
        double                          GetFrameAverageExclusiveTimeCPU( ) const    { return m_averageExclusiveTimeCPU; }
        double                          GetFrameAverageExclusiveTimeGPU( ) const    { return m_averageExclusiveTimeGPU; }
        double                          GetFrameMaxTotalTimeCPU( ) const            { return m_maxTotalTimeCPU;         }
        double                          GetFrameMaxTotalTimeGPU( ) const            { return m_maxTotalTimeGPU;         }
        double                          GetFrameMaxExclusiveTimeCPU( ) const        { return m_maxExclusiveTimeCPU;     }
        double                          GetFrameMaxExclusiveTimeGPU( ) const        { return m_maxExclusiveTimeGPU;     }
    };

    class vaProfiler : public vaSingletonBase<vaProfiler>
    {
    public:

    private:
        vaNestedProfilerNode            m_root;
        vaNestedProfilerNode *          m_currentScope;

        vaSystemTimer                   m_timer;

        vaProfilerTimingsDisplayType    m_displayType;

        int64                           m_profilerFrameIndex;
    
    protected:
        friend class vaCore;
        vaProfiler( );
        ~vaProfiler( );

    protected:
        friend class vaScopeTimer;

        vaNestedProfilerNode *          StartScope( const string & name, void * gpuAPIContext, bool newNodeSelectedDefault );
        void                            StopScope( vaNestedProfilerNode * node );

    public:
        void                            NewFrame( );
        void                            DisplayImGui( bool showCPU );
        const vaNestedProfilerNode *    FindNode( const string & name ) const;
    };

    class vaScopeTimer
    {
        vaNestedProfilerNode * const    m_node;

        static bool                     s_disableScopeTimer;

    private:
        //
    public:
        vaScopeTimer( const string & name, void * gpuAPIContext = nullptr, bool newNodeSelectedDefault = false )
            : m_node( (!s_disableScopeTimer)?(vaProfiler::GetInstance().StartScope( name, gpuAPIContext, newNodeSelectedDefault ) ) : ( nullptr ) ) 
        { 
        }
        virtual ~vaScopeTimer( )
        {
            if( !s_disableScopeTimer )
                vaProfiler::GetInstance().StopScope( m_node );
        }

        static bool                 IsEnabled( )                { return !s_disableScopeTimer; }
        static void                 SetEnabled( bool enabled )  { s_disableScopeTimer = !enabled; }
    };

#define VA_SCOPE_PROFILER_ENABLED

#ifdef VA_SCOPE_PROFILER_ENABLED
    #define VA_SCOPE_CPU_TIMER( name )                                          vaScopeTimer scope_##name( #name )
    #define VA_SCOPE_CPU_TIMER_NAMED( nameVar, nameScope )                      vaScopeTimer scope_##name( nameScope )
    #define VA_SCOPE_CPU_TIMER_NAMED_DEFAULTSELECTED( nameVar, nameScope )      vaScopeTimer scope_##name( nameScope, nullptr, true )
#else
    #define VA_SCOPE_CPU_TIMER( name )                      
    #define VA_SCOPE_CPU_TIMER_NAMED( nameVar, nameScope )  
    #define VA_SCOPE_CPU_TIMER_NAMED_DEFAULTSELECTED( nameVar, nameScope )
#endif

}
