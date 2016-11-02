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

#include "Rendering/vaRenderDevice.h"

#include "Rendering/DirectX/vaDirectXCore.h"
#include "Rendering/DirectX/vaDebugCanvas2DDX11.h"
#include "Rendering/DirectX/vaDebugCanvas3DDX11.h"

namespace VertexAsylum
{
    class vaApplication;

    class vaRenderDeviceDX11 : public vaRenderDevice
    {
    private:

        IDXGIFactory1 *             m_DXGIFactory;
        ID3D11Device *              m_device;
        ID3D11DeviceContext *       m_deviceImmediateContext;
        IDXGISwapChain *            m_swapChain;
        IDXGIOutput *               m_mainOutput;

        ID3D11RenderTargetView *    m_mainRenderTargetView;
        D3D11_TEXTURE2D_DESC        m_backbufferTextureDesc;

        ID3D11Texture2D *           m_mainDepthStencil;
        ID3D11DepthStencilView *    m_mainDepthStencilView;
        ID3D11ShaderResourceView *  m_mainDepthSRV;

        int64                       m_renderFrameCounter;

        vaApplication *             m_application;

    private:
        vaDirectXCore               m_directXCore;
        vaDebugCanvas2DDX11         m_canvas2D;
        vaDebugCanvas3DDX11         m_canvas3D;

    private:
        //static vaRenderDeviceDX11 *    s_mainDevice;

    public:
        vaRenderDeviceDX11( const vaConstructorParamsBase * params );
        virtual ~vaRenderDeviceDX11( void );

    private:
        bool                        Initialize( );
        void                        Deinitialize( );

    protected:
        // vaRenderDevice implementation
        virtual void                OnAppAboutToQuit( )                                                         { Deinitialize(); }

        virtual void                CreateSwapChain( int width, int height, bool windowed, HWND hwnd );
        virtual bool                ResizeSwapChain( int width, int height, bool windowed );                    // returns true if actually resized

        virtual bool                IsFullscreen( );
        virtual bool                IsSwapChainCreated( ) const                                                 { return m_swapChain != NULL; }

        virtual void                FindClosestFullscreenMode( int & width, int & height );

        virtual void                BeginFrame( float deltaTime );
        virtual void                EndAndPresentFrame( int vsyncInterval = 0 );

        virtual void                DrawDebugCanvas2D( );
        virtual void                DrawDebugCanvas3D( vaDrawContext & drawContext );

        virtual vaDebugCanvas2DBase *    GetCanvas2D( )                                                         { return (vaDebugCanvas2DBase *)&m_canvas2D; }
        virtual vaDebugCanvas3DBase *    GetCanvas3D( )                                                         { return (vaDebugCanvas3DBase *)&m_canvas3D; }

        virtual void                RecompileFileLoadedShaders( );

        virtual wstring             GetAdapterNameShort( );
        virtual wstring             GetAdapterNameID( );

    public:

        ID3D11DeviceContext *       GetImmediateRenderContext( ) const                                          { return m_deviceImmediateContext; }

        void                        SetMainRenderTargetToImmediateContext( );

        //static vaRenderDeviceDX11 &    GetMainInstance( )                                                     { assert( s_mainDevice != NULL ); return *s_mainDevice; }

    public:
        ID3D11Device *              GetPlatformDevice( ) const                                                  { m_device; }

    private:
        void                        CreateSwapChainRelatedObjects( );
        void                        ReleaseSwapChainRelatedObjects( );
    };

}