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

#include "Rendering/DirectX/vaGPUTimerDX11.h"

#include <algorithm>

#include <pix.h>

using namespace VertexAsylum;

namespace
{
    struct vaGPUTimerStaticPartDX11 : public vaSingletonBase< vaGPUTimerStaticPartDX11 >
    {
        __int64                             s_lastFrameID;
        bool                                s_frameActive;

        ID3D11Device *                      s_device;
        ID3D11DeviceContext *               s_immediateContext;

        std::vector<ID3D11Query*>           s_disjointQueries;
        std::vector<ID3D11Query*>           s_timerQueries;

        std::vector<vaGPUTimerDX11*>        s_instances;

        vaGPUTimerStaticPartDX11( )
        {
            s_immediateContext = NULL;
            s_device = NULL;

            s_lastFrameID = 0;
            s_frameActive = false;
        }

        ~vaGPUTimerStaticPartDX11( )
        {
            assert( s_disjointQueries.size( ) == 0 );
            assert( s_timerQueries.size( ) == 0 );
            assert( s_instances.size( ) == 0 );
        }
    };
}

void vaGPUTimerDX11::OnDeviceAndContextCreated( ID3D11Device* device )
{
    if( vaGPUTimerStaticPartDX11::GetInstancePtr( ) == NULL )
        new vaGPUTimerStaticPartDX11( );

    assert( vaGPUTimerStaticPartDX11::GetInstance( ).s_immediateContext == NULL );
    vaGPUTimerStaticPartDX11::GetInstance( ).s_device = device;
    device->AddRef( );
    
    {
        device->GetImmediateContext( &vaGPUTimerStaticPartDX11::GetInstance( ).s_immediateContext );

        if( vaGPUTimerStaticPartDX11::GetInstance( ).s_immediateContext == nullptr )
        {
            // this isn't going to work
            assert( false );
        }
    }

    vaGPUTimerStaticPartDX11::GetInstance( ).s_lastFrameID = 0;
    vaGPUTimerStaticPartDX11::GetInstance( ).s_frameActive = false;
}

void vaGPUTimerDX11::OnDeviceAboutToBeDestroyed( )
{
    FrameFinishQueries( true );

    assert( vaGPUTimerStaticPartDX11::GetInstance( ).s_immediateContext != NULL );
    SAFE_RELEASE( vaGPUTimerStaticPartDX11::GetInstance( ).s_device );
    SAFE_RELEASE( vaGPUTimerStaticPartDX11::GetInstance( ).s_immediateContext );

    for( int i = 0; i < (int)vaGPUTimerStaticPartDX11::GetInstance( ).s_disjointQueries.size( ); i++ )
        SAFE_RELEASE( vaGPUTimerStaticPartDX11::GetInstance( ).s_disjointQueries[i] );
    vaGPUTimerStaticPartDX11::GetInstance( ).s_disjointQueries.clear( );

    for( int i = 0; i < (int)vaGPUTimerStaticPartDX11::GetInstance( ).s_timerQueries.size( ); i++ )
        SAFE_RELEASE( vaGPUTimerStaticPartDX11::GetInstance( ).s_timerQueries[i] );
    vaGPUTimerStaticPartDX11::GetInstance( ).s_timerQueries.clear( );

    for( int i = 0; i < (int)vaGPUTimerStaticPartDX11::GetInstance( ).s_instances.size( ); i++ )
    {
        vaGPUTimerStaticPartDX11::GetInstance( ).s_instances[i]->m_orphaned = true;
        SAFE_RELEASE( vaGPUTimerStaticPartDX11::GetInstance( ).s_instances[i]->m_deviceContext2 );  // need to get rid of these before device is destroyed
    }
    vaGPUTimerStaticPartDX11::GetInstance( ).s_instances.clear( );

    delete vaGPUTimerStaticPartDX11::GetInstancePtr( );
}

void vaGPUTimerDX11::FrameFinishQueries( bool forceAll )
{
    assert( vaGPUTimerStaticPartDX11::GetInstance( ).s_immediateContext != NULL );
    assert( !vaGPUTimerStaticPartDX11::GetInstance( ).s_frameActive );

    for( int i = 0; i < (int)vaGPUTimerStaticPartDX11::GetInstance( ).s_instances.size( ); i++ )
        vaGPUTimerStaticPartDX11::GetInstance( ).s_instances[i]->FinishQueries( forceAll );
}

void vaGPUTimerDX11::OnFrameStart( )
{
    FrameFinishQueries( false );

    assert( vaGPUTimerStaticPartDX11::GetInstance( ).s_immediateContext != NULL );
    assert( !vaGPUTimerStaticPartDX11::GetInstance( ).s_frameActive );

    vaGPUTimerStaticPartDX11::GetInstance( ).s_frameActive = true;
    vaGPUTimerStaticPartDX11::GetInstance( ).s_lastFrameID++;
}

void vaGPUTimerDX11::OnFrameEnd( )
{
    assert( vaGPUTimerStaticPartDX11::GetInstance( ).s_immediateContext != NULL );
    assert( vaGPUTimerStaticPartDX11::GetInstance( ).s_frameActive );
    vaGPUTimerStaticPartDX11::GetInstance( ).s_frameActive = false;
}

ID3D11Query * vaGPUTimerDX11::GetDisjointQuery( )
{
    ID3D11Query * ret = NULL;
    if( vaGPUTimerStaticPartDX11::GetInstance( ).s_disjointQueries.size( ) == 0 )
    {
        CD3D11_QUERY_DESC tqd( D3D11_QUERY_TIMESTAMP_DISJOINT, 0 );
        if( FAILED( vaGPUTimerStaticPartDX11::GetInstance( ).s_device->CreateQuery( &tqd, &ret ) ) )
        {
            assert( false );
            return NULL;
        }
    }
    else
    {
        ret = vaGPUTimerStaticPartDX11::GetInstance( ).s_disjointQueries.back( );
        vaGPUTimerStaticPartDX11::GetInstance( ).s_disjointQueries.pop_back( );
    }
    return ret;
}

void vaGPUTimerDX11::ReleaseDisjointQuery( ID3D11Query * q )
{
    vaGPUTimerStaticPartDX11::GetInstance( ).s_disjointQueries.push_back( q );
}

ID3D11Query * vaGPUTimerDX11::GetTimerQuery( )
{
    ID3D11Query * ret = NULL;
    if( vaGPUTimerStaticPartDX11::GetInstance( ).s_timerQueries.size( ) == 0 )
    {
        CD3D11_QUERY_DESC tqd( D3D11_QUERY_TIMESTAMP, 0 );
        if( FAILED( vaGPUTimerStaticPartDX11::GetInstance( ).s_device->CreateQuery( &tqd, &ret ) ) )
        {
            assert( false );
            return NULL;
        }
    }
    else
    {
        ret = vaGPUTimerStaticPartDX11::GetInstance( ).s_timerQueries.back( );
        vaGPUTimerStaticPartDX11::GetInstance( ).s_timerQueries.pop_back( );
    }
    return ret;
}

void vaGPUTimerDX11::ReleaseTimerQuery( ID3D11Query * q )
{
    vaGPUTimerStaticPartDX11::GetInstance( ).s_timerQueries.push_back( q );
}


vaGPUTimerDX11::vaGPUTimerDX11( const vaConstructorParamsBase * params )
{
    if( vaGPUTimerStaticPartDX11::GetInstancePtr( ) == NULL )
        new vaGPUTimerStaticPartDX11( );

    vaGPUTimerStaticPartDX11::GetInstance( ).s_instances.push_back( this );

    memset( m_history, 0, sizeof( m_history ) );
    for( int i = 0; i < (int)c_historyLength; i++ )
        m_history[i].TimingResult = -1.0;

    m_historyLastIndex = 0;
    m_active = false;
    m_lastTime = 0.0f;
    m_avgTime = 0.0f;
    m_orphaned = false;
    m_deviceContext = nullptr;
    m_deviceContext2 = nullptr;
}

vaGPUTimerDX11::~vaGPUTimerDX11( )
{
    // m_deviceContext is not reference tracked, so don't release it
    SAFE_RELEASE( m_deviceContext2 ); // but m_deviceContext2 is

    if( vaGPUTimerStaticPartDX11::GetInstancePtr( ) == NULL )
    {
        assert( m_orphaned );
        return;
    }
    if( m_orphaned )
        return;

    std::vector<vaGPUTimerDX11*>::iterator it = std::find( vaGPUTimerStaticPartDX11::GetInstance( ).s_instances.begin( ), vaGPUTimerStaticPartDX11::GetInstance( ).s_instances.end( ), this );
    if( it != vaGPUTimerStaticPartDX11::GetInstance( ).s_instances.end( ) )
    {
        vaGPUTimerStaticPartDX11::GetInstance( ).s_instances.erase( it );
    }
    else
    {
        // this instance not found in the global list?
        assert( false );
    }
}

void vaGPUTimerDX11::SetAPIContext( void * gpuAPIContext )
{
    // m_deviceContext is not reference tracked, so don't release it
    SAFE_RELEASE( m_deviceContext2 ); // but m_deviceContext2 is

    m_deviceContext = reinterpret_cast<ID3D11DeviceContext2 *>( gpuAPIContext ); 

    if( !SUCCEEDED( m_deviceContext->QueryInterface( IID_ID3D11DeviceContext2, (void**)&m_deviceContext2 ) ) )
    {
        m_deviceContext2 = nullptr;
        // well, not a big deal
    }
}

void vaGPUTimerDX11::Start( )
{
    assert( !m_orphaned );
    if( m_orphaned )
        return;

    if( m_deviceContext == nullptr )
        m_deviceContext = vaGPUTimerStaticPartDX11::GetInstance( ).s_immediateContext;

    assert( vaGPUTimerStaticPartDX11::GetInstancePtr( ) != NULL );

    assert( vaGPUTimerStaticPartDX11::GetInstance( ).s_frameActive );
    assert( vaGPUTimerStaticPartDX11::GetInstance( ).s_device != NULL );
    assert( !m_active );

    m_historyLastIndex = ( m_historyLastIndex + 1 ) % c_historyLength;

    assert( !m_history[m_historyLastIndex].QueryActive );

    m_history[m_historyLastIndex].DisjointData.Frequency = 0;
    m_history[m_historyLastIndex].DisjointData.Disjoint = 1;
    m_history[m_historyLastIndex].StartData = -1;
    m_history[m_historyLastIndex].StopData = -1;
    m_history[m_historyLastIndex].TimingResult = -1.0;
    m_history[m_historyLastIndex].DisjointQuery = GetDisjointQuery( );
    m_history[m_historyLastIndex].StartQuery = GetTimerQuery( );
    m_history[m_historyLastIndex].StopQuery = GetTimerQuery( );
    m_history[m_historyLastIndex].QueryActive = true;
    m_history[m_historyLastIndex].FrameID = vaGPUTimerStaticPartDX11::GetInstance( ).s_lastFrameID;
    m_active = true;
    m_deviceContext->Begin( m_history[m_historyLastIndex].DisjointQuery );
    m_deviceContext->End( m_history[m_historyLastIndex].StartQuery );

 #if /*defined(RELEASE) ||*/ _MSC_VER < 1800
     ( label );
 #else
     if( m_name != "" )
         ::PIXBeginEvent( m_deviceContext2, 0, m_name.c_str( ) );
 #endif

}

void vaGPUTimerDX11::Stop( )
{
    assert( !m_orphaned );
    if( m_orphaned )
        return;

    assert( vaGPUTimerStaticPartDX11::GetInstancePtr( ) != NULL );

    assert( vaGPUTimerStaticPartDX11::GetInstance( ).s_device != NULL );
    assert( vaGPUTimerStaticPartDX11::GetInstance( ).s_frameActive );
    assert( m_active );
    assert( m_history[m_historyLastIndex].QueryActive );

    m_deviceContext->End( m_history[m_historyLastIndex].StopQuery );
    m_deviceContext->End( m_history[m_historyLastIndex].DisjointQuery );
    m_active = false;

#if /*!defined(RELEASE) &&*/ _MSC_VER >= 1800
    if( m_name != "" )
        ::PIXEndEvent( m_deviceContext2 );
#endif
}

static bool GetQueryDataHelper( ID3D11DeviceContext * context, bool loopUntilDone, ID3D11Query * query, void * data, int dataSize )
{
    if( query == NULL )
        return false;

    HRESULT hr = 0;
    int attempts = 0;
    do
    {
        hr = context->GetData( query, data, dataSize, (loopUntilDone)?(0):(D3D11_ASYNC_GETDATA_DONOTFLUSH) );
        if( hr == S_OK )
        {
            return true;
        }
        attempts++;
        if( attempts > 100 )
            Sleep( 1 );
        if( attempts > 1000 )
        {
            VA_ASSERT( false, L"vaGPUTimerDX11.cpp - Infinite loop while doing m_deviceContext->GetData() - this shouldn't happen. " _CRT_WIDE( __FILE__ ) L", line: " _CRT_WIDE( _CRT_STRINGIZE( __LINE__ ) ) ); return false;
        }
    } while( loopUntilDone && ( hr == S_FALSE ) );
    return false;
}

void vaGPUTimerDX11::FinishQueries( bool forceAll )
{
    assert( !m_orphaned );
    assert( !m_active );
    assert( vaGPUTimerStaticPartDX11::GetInstance( ).s_device != NULL );

    int dataGathered = 0;
    m_avgTime = 0.0;

    // get data from previous frames queries if available
    for( int i = 0; i < (int)c_historyLength; i++ )
    {
        int safeIndex = ( i % c_historyLength );

        GPUTimerInfo & item = m_history[safeIndex];

        bool tryGather = true;

        if( item.QueryActive )
        {
            bool loopUntilDone = ( ( vaGPUTimerStaticPartDX11::GetInstance( ).s_lastFrameID - item.FrameID ) >= c_dataQueryMaxLag ) || forceAll;

            if( GetQueryDataHelper( m_deviceContext, loopUntilDone, item.DisjointQuery, &item.DisjointData, sizeof( item.DisjointData ) ) )
            {
                ReleaseDisjointQuery( item.DisjointQuery );
                item.DisjointQuery = NULL;
            }

            if( GetQueryDataHelper( m_deviceContext, loopUntilDone, item.StartQuery, &item.StartData, sizeof( item.StartData ) ) )
            {
                ReleaseTimerQuery( item.StartQuery );
                item.StartQuery = NULL;
            }

            if( GetQueryDataHelper( m_deviceContext, loopUntilDone, item.StopQuery, &item.StopData, sizeof( item.StopData ) ) )
            {
                ReleaseTimerQuery( item.StopQuery );
                item.StopQuery = NULL;
            }

            if( ( item.StartQuery == NULL ) && ( item.StopQuery == NULL ) && ( item.DisjointQuery == NULL ) )
            {
                if( ( item.DisjointData.Disjoint ) || ( ( item.StartData & 0xFFFFFFFF ) == 0xFFFFFFFF ) || ( ( item.StopData & 0xFFFFFFFF ) == 0xFFFFFFFF ) )
                {
                    // discard data
                    item.TimingResult = -1.0;
                }
                else
                {
                    item.TimingResult = ( item.StopData - item.StartData ) / (double)item.DisjointData.Frequency;
                }

                item.QueryActive = false;
            }
        }

        if( ( !item.QueryActive ) && ( item.TimingResult != -1.0 ) )
        {
            dataGathered++;
            m_lastTime = item.TimingResult;
            m_avgTime += item.TimingResult;
        }
    }

    if( dataGathered == 0 )
        m_avgTime = 0.0f;
    else
        m_avgTime /= (double)dataGathered;
}

