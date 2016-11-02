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

#include "Rendering/vaRendering.h"

#include "Rendering/vaTexture.h"

namespace VertexAsylum
{

    // vaRenderDeviceContext is used to get/set current render targets and access rendering API stuff like contexts, etc.
    class vaRenderDeviceContext : public vaRenderingModule
    {
        VA_RENDERING_MODULE_MAKE_FRIENDS( );

    public:
        static const uint32                 c_maxRTs = 8;
        static const uint32                 c_maxUAVs = 8;

        struct OutputsState
        {
            vaViewport                          Viewport;
            vaVector4i                          ScissorRect;
            bool                                ScissorRectEnabled;

            shared_ptr<vaTexture>               RenderTargets[c_maxRTs];
            shared_ptr<vaTexture>               UAVs[c_maxUAVs];
            uint32                              UAVInitialCounts[c_maxUAVs];
            std::shared_ptr<vaTexture>          DepthStencil;

            uint32                              RenderTargetCount;
            uint32                              UAVsStartSlot;
            uint32                              UAVCount;
        };

        OutputsState                            m_outputsState;

    protected:

    protected:
        vaRenderDeviceContext( ) : vaRenderingModule( ) { }
    
    public:
        virtual ~vaRenderDeviceContext( ) {}
        //
    public:
        //
        const vaViewport &                  GetViewport( ) const                                { return m_outputsState.Viewport;    }
        void                                SetViewport( const vaViewport & vp )                { m_outputsState.Viewport = vp; m_outputsState.ScissorRect = vaVector4i( 0, 0, 0, 0 ); m_outputsState.ScissorRectEnabled = false; UpdateViewport(); }

        void                                GetScissorRect( vaVector4i & outScissorRect, bool & outScissorRectEnabled ) { outScissorRect = m_outputsState.ScissorRect; outScissorRectEnabled = m_outputsState.ScissorRectEnabled; }
        void                                SetViewportAndScissorRect( const vaViewport & vp, const vaVector4i & scissorRect )  
                                                                                                { m_outputsState.Viewport = vp; m_outputsState.ScissorRect = scissorRect; m_outputsState.ScissorRectEnabled = true; UpdateViewport(); }

        const std::shared_ptr<vaTexture> &  GetRenderTarget( ) const                            { return m_outputsState.RenderTargets[0]; }
        const std::shared_ptr<vaTexture> &  GetDepthStencil( ) const                            { return m_outputsState.DepthStencil; }
        const shared_ptr<vaTexture> *       GetRenderTargets( ) const                           { return m_outputsState.RenderTargets; }
        const shared_ptr<vaTexture> *       GetUAVs( ) const                                    { return m_outputsState.RenderTargets; }
        
        uint32                              GetrenderTargetCount( ) const                       { return m_outputsState.RenderTargetCount; }
        uint32                              GetUAVsStartSlot( ) const                           { return m_outputsState.UAVsStartSlot; }
        uint32                              GetUAVCount( ) const                                { return m_outputsState.UAVCount; }

        const OutputsState &                GetOutputs( ) const                                 { return m_outputsState; }
        void                                SetOutputs( const OutputsState & state )            { m_outputsState = state; UpdateRenderTargetsDepthStencilUAVs(); UpdateViewport(); }

        void                                SetRenderTarget( const std::shared_ptr<vaTexture> & renderTarget, const std::shared_ptr<vaTexture> & depthStencil, bool updateViewport );

        void                                SetRenderTargets( uint32 numRTs, const std::shared_ptr<vaTexture> * renderTargets, const std::shared_ptr<vaTexture> & depthStencil, bool updateViewport );

        void                                SetRenderTargetsAndUnorderedAccessViews( uint32 numRTs, const std::shared_ptr<vaTexture> * renderTargets, const std::shared_ptr<vaTexture> & depthStencil, 
                                                                                        uint32 UAVStartSlot, uint32 numUAVs, const std::shared_ptr<vaTexture> * UAVs, bool updateViewport, const uint32 * UAVInitialCounts = nullptr );

    protected:
        virtual void                        UpdateViewport( ) = 0;
        virtual void                        UpdateRenderTargetsDepthStencilUAVs( ) = 0;

    public:
        virtual void *                      GetAPIImmediateContextPtr( ) = 0;

    };


}