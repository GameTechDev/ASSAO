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

#include "Rendering/vaRenderDeviceContext.h"

#include "Rendering/vaDebugCanvas.h"

#include "Rendering/vaTexture.h"

namespace VertexAsylum
{
    class vaRenderDevice
    {
    protected:

        int64                                   m_renderFrameCounter;

        // vaDebugCanvas2DDX11          m_canvas2D;
        // vaDebugCanvas3DDX11          m_canvas3D;
        //vaMatrix4x4                     m_mainViewProj;      // needed by Canvas3D

        std::shared_ptr<vaTexture>              m_mainColor;    // aka m_swapChainColor
        std::shared_ptr<vaTexture>              m_mainDepth;    // aka m_swapChainDepth
        std::shared_ptr<vaRenderDeviceContext>  m_mainDeviceContext;

        bool                                    m_profilingEnabled;

    public:
        vaRenderDevice( );
        virtual ~vaRenderDevice( );

    public:
//        virtual bool                    Initialize( )                                                               = 0;
//        virtual void                    Deinitialize( )                                                             = 0;
        virtual void                    OnAppAboutToQuit( )                                                         = 0; 

        virtual void                    CreateSwapChain( int width, int height, bool windowed, HWND hwnd )          = 0;
        virtual bool                    ResizeSwapChain( int width, int height, bool windowed )                     = 0;    // return true if actually resized (for further handling)

        std::shared_ptr<vaTexture>      GetMainChainColor( ) const                                                  { return m_mainColor; }
        std::shared_ptr<vaTexture>      GetMainChainDepth( ) const                                                  { return m_mainDepth; }

        virtual bool                    IsFullscreen( )                                                             = 0;
        virtual bool                    IsSwapChainCreated( ) const                                                 = 0;
        
        virtual void                    FindClosestFullscreenMode( int & width, int & height )                      = 0;

        // main 
        virtual void                    BeginFrame( float deltaTime )                                               = 0;
        virtual void                    EndAndPresentFrame( int vsyncInterval = 0 )                                 = 0;

        int64                           GetFrameCount( ) const                                                      { return m_renderFrameCounter; }
//        const vaViewport &              GetMainViewport( ) const                                                    { assert( m_mainCanvas != NULL ); return m_mainCanvas->GetViewport(); }

        // void                           SetMainRenderTargetToImmediateContext( );
        // void                           SetMainViewProjTransform( vaMatrix4x4 & mainViewProj );   // necessary for Canvas3D to work

        virtual void                    DrawDebugCanvas2D( )                                                        = 0;
        virtual void                    DrawDebugCanvas3D( vaDrawContext & drawContext )                            = 0;

        vaRenderDeviceContext *         GetMainContext( )                                                            { return m_mainDeviceContext.get(); }
                
        virtual vaDebugCanvas2DBase *   GetCanvas2D( )                                                              = 0;
        virtual vaDebugCanvas3DBase *   GetCanvas3D( )                                                              = 0;


        bool                            IsProfilingEnabled( )                                                       { return m_profilingEnabled; }

        virtual void                    RecompileFileLoadedShaders( )                                               = 0;

        virtual wstring                 GetAdapterNameShort( )                                                      = 0;
        virtual wstring                 GetAdapterNameID( )                                                         = 0;

    protected:
        void                            CleanupAPIDependencies( );

        // ID3D11Device *             GetPlatformDevice( ) const                                  { m_device; }
    };

}