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

#include "vaRendering.h"

#include "vaTexture.h"

//#include "Rendering/Media/Shaders/vaSharedTypes.h"

#include "IntegratedExternals/vaImguiIntegration.h"

namespace VertexAsylum
{
    // A helper for GBuffer rendering - does creation and updating of render target textures and provides debug views of them.
    class vaGBuffer : public vaRenderingModule, public vaImguiHierarchyObject
    {
    public:

        // to disable a texture, use vaTextureFormat::Unknown
        struct BufferFormats
        {
            vaTextureFormat         DepthBuffer;
            vaTextureFormat         DepthBufferViewspaceLinear;
            vaTextureFormat         Albedo;
            vaTextureFormat         NormalMap;
            vaTextureFormat         Radiance;
            vaTextureFormat         OutputColor;

            BufferFormats( )
            {
                DepthBuffer                 = vaTextureFormat::D32_FLOAT;
                DepthBufferViewspaceLinear  = vaTextureFormat::R16_FLOAT;
                Albedo                      = vaTextureFormat::R8G8B8A8_UNORM_SRGB;
                NormalMap                   = vaTextureFormat::R8G8B8A8_UNORM;          // improve encoding, drop to R8G8?
                Radiance                    = vaTextureFormat::R11G11B10_FLOAT;         // is this enough? R16G16B16A16_FLOAT as alternative
                OutputColor                 = vaTextureFormat::R8G8B8A8_UNORM_SRGB;     // for HDR displays use something higher
            }
        };

    protected:
        string                                          m_debugInfo;
        mutable int                                     m_debugSelectedTexture;

        BufferFormats                                   m_formats;

        shared_ptr<vaTexture>                           m_depthBuffer;                      // just a regular depth buffer
        shared_ptr<vaTexture>                           m_depthBufferViewspaceLinear;       // regular depth buffer converted to viewspace 
        shared_ptr<vaTexture>                           m_normalMap;                        // screen space normal map
        shared_ptr<vaTexture>                           m_albedo;                           // material color plus whatever else
        shared_ptr<vaTexture>                           m_radiance;                         // a.k.a. light accumulation, a.k.a. screen color - final lighting output goes here as well as emissive materials
        shared_ptr<vaTexture>                           m_outputColor;                      // final output color

        // Light Pre-Pass
        // shared_ptr<vaTexture>                           m_diffuseIrradiance;        // placeholder for Light Pre-Pass
        // shared_ptr<vaTexture>                           m_specularIrradiance;       // placeholder for Light Pre-Pass

        vaVector2i                                      m_resolution;


    protected:
        vaGBuffer( );
    public:
        virtual ~vaGBuffer( );

    private:
        friend class vaGBufferDX11;

    public:
        void                                            SetFormats( const BufferFormats & newFormats );
        const BufferFormats &                           GetFormats( ) const                     { return m_formats; }

        const shared_ptr<vaTexture> &                   GetDepthBuffer( ) const                 { return m_depthBuffer;              ; } 
        const shared_ptr<vaTexture> &                   GetDepthBufferViewspaceLinear( ) const  { return m_depthBufferViewspaceLinear; } 
        const shared_ptr<vaTexture> &                   GetNormalMap( ) const                   { return m_normalMap;                ; } 
        const shared_ptr<vaTexture> &                   GetAlbedo( ) const                      { return m_albedo;                   ; } 
        const shared_ptr<vaTexture> &                   GetRadiance( ) const                    { return m_radiance;                 ; } 
        const shared_ptr<vaTexture> &                   GetOutputColor( ) const                 { return m_outputColor;              ; } 

        const vaVector2i &                              GetResolution( ) const                  { return m_resolution; }

    public:
        // if viewportWidth/Height are -1, take them from drawContext.Canvas.GetViewport
        virtual void                                    UpdateResources( vaDrawContext & drawContext, int width = -1, int height = -1 )   = 0;

        virtual void                                    RenderDebugDraw( vaDrawContext & drawContext )                                          = 0;

        // draws provided depthTexture (can be the one obtained using GetDepthBuffer( )) into currently selected RT; relies on settings set in vaRenderingGlobals and will assert and return without doing anything if those are not present
        virtual void                                    DepthToViewspaceLinear( vaDrawContext & drawContext, vaTexture & depthTexture )         = 0;

    private:
        virtual string                                  IHO_GetInstanceInfo( ) const { return m_debugInfo; }
        virtual void                                    IHO_Draw( );

    };

}