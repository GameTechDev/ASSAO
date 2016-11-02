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

#include "Core/Misc/vaProfiler.h"

#include "Rendering/vaGPUTimer.h"

#include "IntegratedExternals/vaImguiIntegration.h"

using namespace VertexAsylum;

int vaNestedProfilerNode::s_totalNodes = 0;

bool vaScopeTimer::s_disableScopeTimer = false;

vaNestedProfilerNode::vaNestedProfilerNode( vaNestedProfilerNode * parentNode, const string & name, void * gpuAPIContext, bool selected )
    : m_name( name ), m_parentNode( parentNode ), m_selected( selected )
{
    if( gpuAPIContext != nullptr )
    {
        m_GPUProfiler = VA_RENDERING_MODULE_CREATE_UNIQUE( vaGPUTimer );
        m_GPUProfiler->SetName( name );
        m_GPUProfiler->SetAPIContext( gpuAPIContext );
    }

    m_startTimeCPU = 0.0;

    s_totalNodes++;
    assert( s_totalNodes < c_totalMaxNodes ); // arbitrary limit - makes no sense to have more nodes than this, too granular & slow

    m_lastUsedProfilerFrameIndex = -1;
    m_nodeUsageIndex = -1;

    m_framesUntouched = 0;
    m_childNodeUsageFrameCounter = 0;

    m_totalTimeCPU = 0.0;
    m_totalTimeGPU = 0.0;

    m_exclusiveTimeCPU = 0.0;
    m_exclusiveTimeGPU = 0.0;

    m_historyLastIndex = 0;
    m_hasGPUTimings = false;

    for( int i = 0; i < c_historyFrameCount; i++ )
    {
        m_historyTotalTimeCPU[i] = 0.0;
        m_historyTotalTimeGPU[i] = 0.0;
        m_historyExclusiveTimeCPU[i] = 0.0;
        m_historyExclusiveTimeGPU[i] = 0.0;
    }
    m_averageTotalTimeCPU       = 0.0;
    m_averageTotalTimeGPU       = 0.0;
    m_averageExclusiveTimeCPU   = 0.0;
    m_averageExclusiveTimeGPU   = 0.0;
    m_maxTotalTimeCPU           = 0.0;
    m_maxTotalTimeGPU           = 0.0;
    m_maxExclusiveTimeCPU       = 0.0;
    m_maxExclusiveTimeGPU       = 0.0;
}

vaNestedProfilerNode::~vaNestedProfilerNode( )
{
    for( auto it = m_childNodes.begin(); it != m_childNodes.end(); it++ )
    {
        assert( it->second->m_parentNode == this );
        delete it->second;
    }

    s_totalNodes--;
    assert( s_totalNodes >= 0 );
}

const vaNestedProfilerNode * vaNestedProfilerNode::FindSubNode( const string & name ) const
{
    if( m_name == name )
        return this;

    for( auto it = m_childNodes.cbegin( ); it != m_childNodes.cend( ); it++ )
    {
        assert( it->second->m_parentNode == this );
     
        const vaNestedProfilerNode * ret = it->second->FindSubNode( name );
        if( ret != nullptr )
            return ret;
    }
    return nullptr;
}

vaNestedProfilerNode * vaNestedProfilerNode::StartScope( const string & name, void * gpuAPIContext, double currentTime, int64 profilerFrameIndex, bool newNodeSelectedDefault )
{
    vaNestedProfilerNode * node = nullptr;
    auto it = m_childNodes.find( name );
    if( it != m_childNodes.end( ) )
    {
        if( it->second->m_lastUsedProfilerFrameIndex == profilerFrameIndex )
        {
            // ouch, name already exists, pick another name
            int counter = 0;
            int passIDOffset = (int)name.length()-4;
            if( ( passIDOffset >= 0 ) && (name[passIDOffset] == '_') && (name[passIDOffset+1] == 'p') &&
                (name[passIDOffset+2] >= '0') && (name[passIDOffset+2] <= '9') &&
                (name[passIDOffset+3] >= '0') && (name[passIDOffset+3] <= '9') )
                counter = (name[passIDOffset+2]-'0') * 10 + (name[passIDOffset+3]-'0');
            else
                passIDOffset = (int)name.length();
            counter++;
            if( counter >= 100 )
            {
                // you either have more than 99 attempts to profile a scope with the same name per frame (not supported), or 
                // you've probably forgot to call vaProfiler::GetInstance().NewFrame() every frame
                assert( false );
                return nullptr;
            }
            else
                return StartScope( name.substr( 0, passIDOffset ) + '_' + 'p' + (char)('0'+counter/10) + (char)('0'+counter%10), gpuAPIContext, currentTime, profilerFrameIndex, newNodeSelectedDefault );
        }

        node = it->second;
        assert( node->m_parentNode == this );
    }
    else
    {
        if( m_childNodes.size( ) >= c_totalMaxChildNodes )
        {
            // too many child nodes
            assert( false );
            return nullptr;
        }

        node = new vaNestedProfilerNode( this, name, gpuAPIContext, newNodeSelectedDefault );
        m_childNodes.insert( std::pair<string, vaNestedProfilerNode*>(name, node) );
    }

    node->m_nodeUsageIndex = m_childNodeUsageFrameCounter;
    m_childNodeUsageFrameCounter++;

    assert( node->m_startTimeCPU == 0.0 );

    node->m_lastUsedProfilerFrameIndex = profilerFrameIndex;
    node->m_startTimeCPU = currentTime;
    node->m_framesUntouched = 0;
    node->m_childNodeUsageFrameCounter = 0;

    if( node->m_GPUProfiler )
        node->m_GPUProfiler->Start( );

    return node;
}

void vaNestedProfilerNode::StopScope( double currentTime )
{
    assert( m_startTimeCPU != 0.0 );
    
    m_totalTimeCPU = currentTime - m_startTimeCPU;
    m_startTimeCPU = 0.0;

    if( m_GPUProfiler )
        m_GPUProfiler->Stop( );

    assert( m_framesUntouched == 0 );
}

void vaNestedProfilerNode::RemoveOrphans( )
{
    assert( m_startTimeCPU == 0.0 );

    for( auto it = m_childNodes.cbegin( ); it != m_childNodes.cend( ); )
    {
        assert( it->second->m_parentNode == this );

        if( it->second->m_framesUntouched > c_untouchedFramesKeep )
        {
            delete it->second;
            m_childNodes.erase( it++ );
        }
        else
        {
            it->second->RemoveOrphans();
            ++it;
        }
    }
}

void vaNestedProfilerNode::Proccess( )
{
    double totalChildrenTimeCPU = 0.0;
    double totalChildrenTimeGPU = 0.0;
    m_hasGPUTimings = m_GPUProfiler != nullptr;
    for( auto it = m_childNodes.begin( ); it != m_childNodes.end( ); it++ )
    {
        totalChildrenTimeCPU += vaMath::Abs( it->second->m_totalTimeCPU );
        totalChildrenTimeGPU += vaMath::Abs( it->second->m_totalTimeGPU );
        it->second->Proccess( );
        m_hasGPUTimings |= it->second->m_hasGPUTimings;
    }
    m_totalTimeGPU = ( m_GPUProfiler != nullptr ) ? ( m_GPUProfiler->GetLastTime( ) ) : ( totalChildrenTimeGPU );
    m_exclusiveTimeCPU = m_totalTimeCPU - totalChildrenTimeCPU;
    m_exclusiveTimeGPU = m_totalTimeGPU - totalChildrenTimeGPU;
    m_framesUntouched++;
    m_nodeUsageIndex = -1;

    m_historyTotalTimeCPU[m_historyLastIndex]       = m_totalTimeCPU;
    m_historyTotalTimeGPU[m_historyLastIndex]       = m_totalTimeGPU;
    m_historyExclusiveTimeCPU[m_historyLastIndex]   = m_exclusiveTimeCPU;
    m_historyExclusiveTimeGPU[m_historyLastIndex]   = m_exclusiveTimeGPU;
    m_historyLastIndex++;
    if( m_historyLastIndex >= c_historyFrameCount )
    {
        m_historyLastIndex = 0;

        m_averageTotalTimeCPU       = 0.0;
        m_averageTotalTimeGPU       = 0.0;
        m_averageExclusiveTimeCPU   = 0.0;
        m_averageExclusiveTimeGPU   = 0.0;
        m_maxTotalTimeCPU           = 0.0;
        m_maxTotalTimeGPU           = 0.0;
        m_maxExclusiveTimeCPU       = 0.0;
        m_maxExclusiveTimeGPU       = 0.0;
        for( int i = 0; i < c_historyFrameCount; i++ )
        {
            m_averageTotalTimeCPU       += m_historyTotalTimeCPU[i];
            m_averageTotalTimeGPU       += m_historyTotalTimeGPU[i];
            m_averageExclusiveTimeCPU   += m_historyExclusiveTimeCPU[i];
            m_averageExclusiveTimeGPU   += m_historyExclusiveTimeGPU[i];
            m_maxTotalTimeCPU           = vaMath::Max( m_maxTotalTimeCPU    , m_historyTotalTimeCPU[i]     );
            m_maxTotalTimeGPU           = vaMath::Max( m_maxTotalTimeGPU    , m_historyTotalTimeGPU[i]     );
            m_maxExclusiveTimeCPU       = vaMath::Max( m_maxExclusiveTimeCPU, m_historyExclusiveTimeCPU[i] );
            m_maxExclusiveTimeGPU       = vaMath::Max( m_maxExclusiveTimeGPU, m_historyExclusiveTimeGPU[i] );
        }
        m_averageTotalTimeCPU     /= (double)c_historyFrameCount;
        m_averageTotalTimeGPU     /= (double)c_historyFrameCount;
        m_averageExclusiveTimeCPU /= (double)c_historyFrameCount;
        m_averageExclusiveTimeGPU /= (double)c_historyFrameCount;
    }
}

void vaNestedProfilerNode::Display( const string & namePath, int depth, bool cpu, vaProfilerTimingsDisplayType displayType )
{
#ifdef VA_IMGUI_INTEGRATION_ENABLED
    string newNamePath = m_name;

    if( depth == 0 )
        newNamePath = (cpu)?("CPU"):("GPU");
    else
        newNamePath = namePath + "/" + newNamePath ;

    bool skipZero = true;

    const int indentCharCount = 2;

    string info = m_name;
    if( depth == 0 )
        info = ( cpu ) ? ( "CPU Times" ) : ( "GPU Times" );

    if( m_childNodes.size( ) == 0 )
        info = "  " + info;
    else
        info = "- " + info;

    int totalIndentCharCount = (skipZero)?( (depth-1) * indentCharCount ) : ( depth * indentCharCount );
    totalIndentCharCount = vaMath::Max( 0, totalIndentCharCount );

    info.insert( info.begin( ), totalIndentCharCount, ' ' );

    const int maxPath   = 43;
    int whiteSpaceCount = vaMath::Max( 0, maxPath - (int)info.size() );
    // if( m_childNodes.size( ) == 0 )
    //     whiteSpaceCount += 3;

    float displayTimeTotalCPU;
    float displayTimeTotalGPU;
    float displayTimeExclusiveGPU;
    float displayTimeExclusiveCPU;
    switch( displayType )
    {
    case vaProfilerTimingsDisplayType::LastFrame:
        displayTimeTotalCPU     = (float)m_totalTimeCPU;
        displayTimeTotalGPU     = (float)m_totalTimeGPU;
        displayTimeExclusiveCPU = (float)m_exclusiveTimeCPU;
        displayTimeExclusiveGPU = (float)m_exclusiveTimeGPU;
        break;
    case vaProfilerTimingsDisplayType::LastXFramesAverage:
        displayTimeTotalCPU     = (float)m_averageTotalTimeCPU;
        displayTimeTotalGPU     = (float)m_averageTotalTimeGPU;
        displayTimeExclusiveCPU = (float)m_averageExclusiveTimeCPU;
        displayTimeExclusiveGPU = (float)m_averageExclusiveTimeGPU;
        break;
    case vaProfilerTimingsDisplayType::LastXFramesMax:
        displayTimeTotalCPU     = (float)m_maxTotalTimeCPU;
        displayTimeTotalGPU     = (float)m_maxTotalTimeGPU;
        displayTimeExclusiveCPU = (float)m_maxExclusiveTimeCPU;
        displayTimeExclusiveGPU = (float)m_maxExclusiveTimeGPU;
        break;
    default:
        break;
    }
    displayTimeTotalCPU     = vaMath::Max( 0.0f, displayTimeTotalCPU     );
    displayTimeTotalGPU     = vaMath::Max( 0.0f, displayTimeTotalGPU     );
    displayTimeExclusiveCPU = vaMath::Max( 0.0f, displayTimeExclusiveCPU );
    displayTimeExclusiveGPU = vaMath::Max( 0.0f, displayTimeExclusiveGPU );

    info.insert( info.end(), whiteSpaceCount, ' ' );
    if( cpu )
        info += vaStringTools::Format( "(%5.2f) %5.2f", displayTimeExclusiveCPU*1000.0f, displayTimeTotalCPU*1000.0f );
    else
    {
        if( m_hasGPUTimings )
            info += vaStringTools::Format( "(%5.2f) %5.2f", displayTimeExclusiveGPU*1000.0f, displayTimeTotalGPU*1000.0f );
    }

    bool skipThis = skipZero && (depth == 0);

    if( cpu || m_hasGPUTimings )
    {
        if( m_childNodes.size( ) == 0 )
        {
            if( !skipThis )
                ImGui::Selectable( info.c_str(), &m_selected );
        }
        else
        {
            //if( !skipThis && ImGui::CollapsingHeader( info.c_str(), newNamePath.c_str(), false, !cpu ) )
            if( !skipThis )
                ImGui::Selectable( info.c_str(), &m_selected );
            {
                //ImGui::Indent( );

                vaNestedProfilerNode * sortedNodes[c_totalMaxChildNodes];
                for( int i = 0; i < m_childNodes.size( ); i++ )
                    sortedNodes[i] = nullptr;

                // dirty but quick sorting by m_nodeUsageIndex
                for( auto it = m_childNodes.begin( ); it != m_childNodes.end( ); it++ )
                {
                    if( ( it->second->m_nodeUsageIndex >= 0 ) && ( it->second->m_nodeUsageIndex < vaMath::Min( (int)m_childNodes.size(), c_totalMaxChildNodes ) ) )
                    {
                        if( sortedNodes[it->second->m_nodeUsageIndex] == nullptr )
                        {
                            sortedNodes[it->second->m_nodeUsageIndex] = it->second;
                        }
                        else
                        {
                            // shouldn't happen but if it does, fix the algorithm (and it's not going to cause any catastrophic damage, just not display some of the nodes)
                            assert( false );
                        }
                    }
                    else
                    { 
                        if( it->second->m_nodeUsageIndex != -1 )
                        {
                            // it->second->m_nodeUsageIndex has a broken value? algorithm bug
                            assert( false );
                        }
                    }
                }
                for( int i = 0; i < m_childNodes.size(); i++ )
                    if( sortedNodes[i] != nullptr )
                        sortedNodes[i]->Display( newNamePath, depth + 1, cpu, displayType );

                //ImGui::Unindent( );
            }
        }
    }
#endif
}

vaProfiler::vaProfiler( )
    : m_root( nullptr, "groot", nullptr, false )
{
    m_currentScope = nullptr;
    m_displayType = vaProfilerTimingsDisplayType::LastXFramesAverage;
    m_profilerFrameIndex = 0;
}

vaProfiler::~vaProfiler( )
{
}

vaNestedProfilerNode * vaProfiler::StartScope( const string & name, void * gpuAPIContext, bool newNodeSelectedDefault )
{
    assert( m_currentScope != nullptr );    // have you called vaProfiler::GetInstance().NewFrame() at the beginning of the frame?
    return m_currentScope = m_currentScope->StartScope( name, gpuAPIContext, m_timer.GetCurrentTimeDouble( ), m_profilerFrameIndex, newNodeSelectedDefault );
}

void vaProfiler::StopScope( vaNestedProfilerNode * node )
{
    assert( node == m_currentScope );
    node->StopScope( m_timer.GetCurrentTimeDouble( ) );
    m_currentScope = node->m_parentNode;
}

const vaNestedProfilerNode * vaProfiler::FindNode( const string & name ) const
{
    return m_root.FindSubNode( name );
}

void vaProfiler::NewFrame( )
{
    if( m_currentScope != nullptr )
    {
        assert( m_currentScope == &m_root );
        StopScope( m_currentScope );
        assert( m_currentScope == nullptr );
    }
    
    m_root.RemoveOrphans( );
    m_root.Proccess( );
    
    // start root
    assert( m_root.m_startTimeCPU == 0.0 );
    m_root.m_startTimeCPU = m_timer.GetCurrentTimeDouble( );
    m_root.m_framesUntouched = 0;
    m_root.m_childNodeUsageFrameCounter = 0;

    m_currentScope = &m_root;

    m_profilerFrameIndex++;
}
//void vaProfiler::Proccess( )
//{
//    assert( m_frameStarted );
//    m_root.Proccess();
//}

void vaProfiler::DisplayImGui( bool showCPU )
{
#ifdef VA_IMGUI_INTEGRATION_ENABLED
    bool showGPU = !showCPU;
    //auto imguiStateStorage = ImGui::GetStateStorage( );

    int displayTypeIndex = (int)m_displayType;

    //auto displayTypeID = ImGui::GetID( "DisplayType" );
    //displayTypeIndex = imguiStateStorage->GetInt( displayTypeID, displayTypeIndex );
    ImGui::PushItemWidth( 150.0f );

    if( ImGui::Combo( "Timings display type", &displayTypeIndex, "Last frame\0N frame average\0N frame max\0\0" ) )   // Combo using values packed in a single constant string (for really quick combo)
    {
        //imguiStateStorage->SetInt( displayTypeID, displayTypeIndex );
    }
    m_displayType = (vaProfilerTimingsDisplayType)displayTypeIndex;

    assert( m_currentScope != nullptr );    // have you called vaProfiler::GetInstance().NewFrame() at the beginning of the frame?
    //ImGui::Text( "" );
    if( showCPU )
        m_root.Display( "", 0, true, m_displayType );
    if( showGPU )
        m_root.Display( "", 0, false, m_displayType );
    //ImGui::Text( "" );
    ImGui::PopItemWidth( );
#endif
}

