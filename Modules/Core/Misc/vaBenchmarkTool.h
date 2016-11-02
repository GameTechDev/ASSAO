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

#include "..\vaCore.h"
#include "..\vaSingleton.h"

#include <functional>

#include <ctime>

namespace VertexAsylum
{

    class vaBenchmarkTool : public vaSingletonBase< vaBenchmarkTool >
    {
    public:
        struct AverageMinMax
        {
            float                           Average;
            float                           Minimum;
            float                           Maximum;
        };

        struct RunDefinition
        {
            std::string                                                         Name;
            std::string                                                         LongInfo;

            float                                                               SamplingPeriod;
            int                                                                 SamplingTotalCount;
            float                                                               DelayStartTime;
            std::vector<std::string>                                            MetricNames;

            std::function< void( const RunDefinition & ) >                      SettingsSetupCallback;
            std::function< void( const RunDefinition &, std::vector<float> & ) >  
                                                                                CollectSamplesCallback;
            // finished run info, finished run index, total run count, finished run samples, finished run averaged samples
            std::function< void( const RunDefinition &, int, int, const std::vector< std::vector<float> > &, const std::vector<AverageMinMax> & ) > 
                                                                                FinishedCallback;

            RunDefinition( ) { SamplingPeriod = 0.0f; SamplingTotalCount = 0; DelayStartTime = 1.0f; }

        };

    private:
        bool                                m_active;

        std::vector<RunDefinition>          m_benchmarkRuns;
        int                                 m_currentRunIndex;

        RunDefinition                       m_currentRun;
        bool                                m_currentRunSetupDone;
        std::vector< float >                m_sampleCache;
        std::vector< std::vector<float> >   m_currentMetricsSampleLog;
        std::vector< AverageMinMax >        m_avgMinMaxCache;

        std::time_t                         m_runStartTime;

        float                               m_timeFromStart;
        int                                 m_currentSampleCount;

    private:

    public:
        vaBenchmarkTool( );
        ~vaBenchmarkTool( );

        // to be expanded with Run( const vector< RunDefinition > & )
        bool                                                Run( const std::vector<RunDefinition> & benchmarks );
        void                                                Tick( float deltaTime );
        void                                                Stop( );

        bool                                                IsRunning( ) const                          { return m_active; }
        int                                                 GetCurrentRunIndex( ) const                 { return m_currentRunIndex; }
        int                                                 GetTotalRunCount( ) const                   { return (int)m_benchmarkRuns.size(); }
        time_t                                              GetRunStartTime( ) const                    { return m_runStartTime; }

        float                                               GetRemainingBenchmarkTime( )                { return m_currentRun.SamplingPeriod * m_currentRun.SamplingTotalCount - m_timeFromStart; }

        static void                                         WriteResultsCSV( const wstring & fileName, bool append, const RunDefinition & runDef, int currentIndex, int totalCount, const std::vector< std::vector<float> > & metricsSamples, const std::vector<AverageMinMax> & metricsAverages );

    protected:
        void                                                StartNextOrStop( );
        void                                                FinishCurrent( );
    };
}