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

#include "vaPoissonDiskGenerator.h"

using namespace VertexAsylum;

int  vaPoissonDiskGenerator::s_lastRandomSeed = 0;

void vaPoissonDiskGenerator::PoissonThreadProc( void * threadParam )
{
    SearchThreadState * ptsPtr = static_cast<SearchThreadState*>( threadParam );
    SearchThreadState & pts = *ptsPtr;

    {
        vaCriticalSectionScopeLock scopeLock( pts.GlobalState.CriticalSection );
   
        if( pts.GlobalState.Break )
        {
            pts.GlobalState.RemainingJobCount--;
            delete ptsPtr;
            return;
        }
    }

    vector<vaVector2> points;
    
    vaPoissonDiskGenerator::SampleCircle( pts.Center, pts.Radius, pts.MinimumDistance, pts.PointsPerIteration, pts.MaxDecimals, pts.FirstPointAtCenter, points );

    {
        vaCriticalSectionScopeLock scopeLock( pts.GlobalState.CriticalSection );

        if( pts.GlobalState.Break )
        {
            pts.GlobalState.RemainingJobCount--;
            delete ptsPtr;
            return;
        }

        if( pts.GlobalState.SearchTarget == -1 )
        {
            if( points.size() > pts.GlobalState.GlobalBest.size() )
                pts.GlobalState.GlobalBest = points;
        }
        else
        {
            if( points.size() == pts.GlobalState.SearchTarget )
            {
                pts.GlobalState.Break = true;
                pts.GlobalState.GlobalBest = points;
            }
        }

        pts.GlobalState.AllCountsSoFar.push_back( (int)points.size() );
        
        pts.GlobalState.RemainingJobCount--;
    }

    delete ptsPtr;
}


void vaPoissonDiskGenerator::SearchCircleByParams( vaVector2 center, float radius, int searchTarget, bool firstPointAtCenter, bool deleteCenterPoint, vector<vaVector2> & outResults, float & outMinDistance )
{
    const int jobThreadCount = (int)vaMath::Min( vaThreadPool::GetInstance().GetThreadCount(), vaThreadPool::c_maxPossibleThreads );

    //vaThreadPool::GetInstance().AddJob( doChunk, j );

    if( !firstPointAtCenter )
        deleteCenterPoint = false;

    if( deleteCenterPoint )
        searchTarget++;

    int finalPointsPerIteration     = 0;

    int         pointsPerIteration  = (int)searchTarget / 3 + 1;
    float       currentMinDistance  = 0.4f;
    float       minDistModifier     = 0.3f;

    bool        lastDirectionUp     = false;

    int failsafeSearchIterationCount = 1000;

    for( int ll = 0; ll < failsafeSearchIterationCount; ll++ )
    {
        const int parallelIterationsPerStep = 32;

        SearchGlobalThreadsState globalThreadsState( parallelIterationsPerStep );

        globalThreadsState.SearchTarget = searchTarget;

        for( int i = 0; i < parallelIterationsPerStep; i++ )
        {
            SearchThreadState * sts = new SearchThreadState( globalThreadsState, center, radius, currentMinDistance, pointsPerIteration, 7, firstPointAtCenter );
            vaThreadPool::GetInstance().AddJob( PoissonThreadProc, (void*) sts );
        }

        for( ;; )
        {
            globalThreadsState.CriticalSection.Enter();
            if( globalThreadsState.RemainingJobCount == 0 )
            {
                globalThreadsState.CriticalSection.Leave();
                break;
            }
            globalThreadsState.CriticalSection.Leave();

            vaThread::YieldProcessor();
        }

        if( (int)globalThreadsState.GlobalBest.size() == globalThreadsState.SearchTarget )
        {
            // found it, exit!
            outResults                      = globalThreadsState.GlobalBest;
            outMinDistance                  = currentMinDistance;
            finalPointsPerIteration         = pointsPerIteration;
            break;
        }
        else
        {
            int countBelow = 0;
            int countAbove = 0;
            int totalCount = (int)globalThreadsState.AllCountsSoFar.size();
            assert( totalCount == parallelIterationsPerStep );
            for( int i = 0; i < totalCount; i++ )
            {
                if( globalThreadsState.AllCountsSoFar[i] > globalThreadsState.SearchTarget )
                    countAbove++;
                else
                    countBelow++;
                assert( globalThreadsState.AllCountsSoFar[i] != globalThreadsState.SearchTarget );
            }
            float ratio = (float)(countAbove - countBelow) / (float)totalCount;

            // if wildly changing direction of search, slightly reduce the distance modifier
            bool newDirectionUp = ratio > 0;
            if( lastDirectionUp != newDirectionUp )
            {
                lastDirectionUp = newDirectionUp;
                if( vaMath::Abs( ratio ) > 0.9f )
                    minDistModifier *= 0.7f;
            }

            float finalModifier = 1.0f + minDistModifier * ratio;
            currentMinDistance *= finalModifier;
        }

    }
    
    if( deleteCenterPoint && outResults.size() > 0 )
    {
        outResults.erase( outResults.begin() + 0 );
    }
    // if( checkBoxDeleteCenterPoint.Checked )
    // {
    //     CurrentPoints.RemoveAll( x => ( x.LengthSq( ) < 0.00001f ) );
    // }
    // 
    // if( checkBoxSortCW.Checked )
    // {
    //     CurrentPoints.Sort( ( x, y ) => ( AngleAroundZero( x ).CompareTo( AngleAroundZero( y ) ) ) );
    // }

}
