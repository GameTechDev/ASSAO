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

#include "vaRenderDeviceContext.h"

using namespace VertexAsylum;

void vaRenderDeviceContext::SetRenderTarget( const std::shared_ptr<vaTexture> & renderTarget, const std::shared_ptr<vaTexture> & depthStencil, bool updateViewport )
{
    for( int i = 0; i < c_maxRTs; i++ )     m_outputsState.RenderTargets[i]      = nullptr;
    for( int i = 0; i < c_maxUAVs; i++ )    m_outputsState.UAVs[i]               = nullptr;
    for( int i = 0; i < c_maxUAVs; i++ )    m_outputsState.UAVInitialCounts[i]   = -1;
    m_outputsState.RenderTargets[0]     = renderTarget;
    m_outputsState.DepthStencil         = depthStencil;
    m_outputsState.RenderTargetCount    = 1;
    m_outputsState.UAVsStartSlot        = 0;
    m_outputsState.UAVCount             = 0;
    m_outputsState.ScissorRect          = vaVector4i( 0, 0, 0, 0 );
    m_outputsState.ScissorRectEnabled   = false;

    const std::shared_ptr<vaTexture> & anyRT = ( renderTarget != NULL ) ? ( renderTarget ) : ( depthStencil );

    vaViewport vp = m_outputsState.Viewport;

    if( anyRT != NULL )
    {
        assert( ( anyRT->GetType( ) == vaTextureType::Texture2D ) || ( anyRT->GetType( ) == vaTextureType::Texture2DMS ) );   // others not supported yet
        vp.X = 0;
        vp.Y = 0;
        vp.Width = anyRT->GetViewedSliceSizeX( );
        vp.Height = anyRT->GetViewedSliceSizeY( );
    }

    if( renderTarget != NULL )
    {
        assert( ( renderTarget->GetBindSupportFlags( ) & vaTextureBindSupportFlags::RenderTarget ) != 0 );
    }
    if( depthStencil != NULL )
    {
        assert( ( depthStencil->GetBindSupportFlags( ) & vaTextureBindSupportFlags::DepthStencil ) != 0 );
    }

    UpdateRenderTargetsDepthStencilUAVs( );

    if( updateViewport )
        SetViewport( vp );
}

void vaRenderDeviceContext::SetRenderTargetsAndUnorderedAccessViews( uint32 numRTs, const std::shared_ptr<vaTexture> * renderTargets, const std::shared_ptr<vaTexture> & depthStencil,
    uint32 UAVStartSlot, uint32 numUAVs, const std::shared_ptr<vaTexture> * UAVs, bool updateViewport, const uint32 * UAVInitialCounts )
{
    assert( numRTs <= c_maxRTs );
    assert( numUAVs <= c_maxUAVs );
    m_outputsState.RenderTargetCount = numRTs  = vaMath::Min( numRTs , c_maxRTs  );
    m_outputsState.UAVCount          = numUAVs = vaMath::Min( numUAVs, c_maxUAVs );

    for( size_t i = 0; i < c_maxRTs; i++ )     
        m_outputsState.RenderTargets[i]      = (i < m_outputsState.RenderTargetCount)?(renderTargets[i]):(nullptr);
    for( size_t i = 0; i < c_maxUAVs; i++ )    
    {
        m_outputsState.UAVs[i]               = (i < m_outputsState.UAVCount)?(UAVs[i]):(nullptr);
        m_outputsState.UAVInitialCounts[i]   = ( (i < m_outputsState.UAVCount) && (UAVInitialCounts != nullptr) )?( UAVInitialCounts[i] ):( -1 );
    }
    m_outputsState.DepthStencil = depthStencil;
    m_outputsState.UAVsStartSlot = UAVStartSlot;

    const std::shared_ptr<vaTexture> & anyRT = ( m_outputsState.RenderTargets[0] != NULL ) ? ( m_outputsState.RenderTargets[0] ) : ( depthStencil );

    vaViewport vp = m_outputsState.Viewport;

    if( anyRT != NULL )
    {
        assert( ( anyRT->GetType( ) == vaTextureType::Texture2D ) || ( anyRT->GetType( ) == vaTextureType::Texture2DMS ) || ( anyRT->GetType( ) == vaTextureType::Texture2DArray ) || ( anyRT->GetType( ) == vaTextureType::Texture2DMSArray ) );   // others not supported yet
        vp.X = 0;
        vp.Y = 0;
        vp.Width = anyRT->GetViewedSliceSizeX( );
        vp.Height = anyRT->GetViewedSliceSizeY( );
    }

    for( size_t i = 0; i < m_outputsState.RenderTargetCount; i++ )
    {
        if( m_outputsState.RenderTargets[i] != nullptr )
        {
            assert( ( m_outputsState.RenderTargets[i]->GetBindSupportFlags( ) & vaTextureBindSupportFlags::RenderTarget ) != 0 );
        }
    }
    for( size_t i = 0; i < m_outputsState.UAVCount; i++ )
    {
        if( m_outputsState.UAVs[i] != nullptr )
        {
            assert( ( m_outputsState.UAVs[i]->GetBindSupportFlags( ) & vaTextureBindSupportFlags::UnorderedAccess ) != 0 );
        }
    }
    if( depthStencil != NULL )
    {
        assert( ( depthStencil->GetBindSupportFlags( ) & vaTextureBindSupportFlags::DepthStencil ) != 0 );
    }

    UpdateRenderTargetsDepthStencilUAVs( );

    if( updateViewport )
    {
        // can't update viewport if no RTs
        assert(anyRT != NULL);
        if( anyRT != NULL )
            SetViewport( vp );
    }
}

void vaRenderDeviceContext::SetRenderTargets( uint32 numRTs, const std::shared_ptr<vaTexture> * renderTargets, const std::shared_ptr<vaTexture> & depthStencil, bool updateViewport )
{
    SetRenderTargetsAndUnorderedAccessViews( numRTs, renderTargets, depthStencil, 0, 0, nullptr, updateViewport );
}
