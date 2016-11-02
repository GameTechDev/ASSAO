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

#include "Rendering/DirectX/vaDirectXCore.h"
#include "Rendering/DirectX/vaDirectXTools.h"
#include "Rendering/DirectX/vaDirectXShader.h"

#include "Rendering/vaRenderingIncludes.h"


namespace VertexAsylum
{

    class vaRenderDeviceContextDX11 : public vaRenderDeviceContext, private VertexAsylum::vaDirectXNotifyTarget
    {
        VA_RENDERING_MODULE_MAKE_FRIENDS( );

    private:
        ID3D11DeviceContext *       m_deviceImmediateContext;

    private:
        // fullscreen pass stuff
        vaDirectXVertexShader       m_fullscreenVS;
        ID3D11Buffer *              m_fullscreenVB;
        ID3D11Buffer *              m_fullscreenVBDynamic;


    protected:
                                    vaRenderDeviceContextDX11( const vaConstructorParamsBase * params );
        virtual                     ~vaRenderDeviceContextDX11( );

    public:
        static vaRenderDeviceContext *           Create( ID3D11DeviceContext * deviceContext );
        ID3D11DeviceContext *       GetDXImmediateContext( ) const                               { return m_deviceImmediateContext; }

    public:
        void                        FullscreenPassDraw( ID3D11DeviceContext * context, ID3D11PixelShader * pixelShader, ID3D11BlendState * blendState = vaDirectXTools::GetBS_Opaque(), ID3D11DepthStencilState * depthStencilState = vaDirectXTools::GetDSS_DepthDisabled_NoDepthWrite(), UINT stencilRef = 0, ID3D11VertexShader * vertexShader = NULL, float ZDistance = 0.0f );

    private:
        void                        Initialize( ID3D11DeviceContext * deviceContext );
        void                        Destroy( );

    private:
        // vaDirectXNotifyTarget
        virtual void                OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain );
        virtual void                OnDeviceDestroyed( );

    private:
        // vaRenderDeviceContext
        virtual void                UpdateViewport( );
        virtual void                UpdateRenderTargetsDepthStencilUAVs( );
        virtual void *              GetAPIImmediateContextPtr( )                                 { return m_deviceImmediateContext; }
    };


}