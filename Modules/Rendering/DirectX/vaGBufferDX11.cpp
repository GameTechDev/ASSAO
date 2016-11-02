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

#include "Core/vaCoreIncludes.h"

#include "Rendering/vaGBuffer.h"

#include "Rendering/DirectX/vaDirectXCore.h"
#include "Rendering/DirectX/vaDirectXShader.h"
#include "Rendering/DirectX/vaDirectXTools.h"
#include "Rendering/DirectX/vaRenderDeviceContextDX11.h"
#include "Rendering/DirectX/vaTextureDX11.h"

#define GBUFFER_SLOT0   0

// 
// #include "Scene/vaSceneIncludes.h"
// 
// #include "Rendering/vaRenderMesh.h"
// 
// #include "Rendering/vaRenderingIncludes.h"
// 
// #include "Rendering/DirectX/vaTriangleMeshDX11.h"
// #include "Rendering/DirectX/vaRenderingToolsDX11.h"
// 
// 
// #include "Rendering/DirectX/Effects/vaSimpleShadowMapDX11.h"
// 
// #include "Rendering/DirectX/vaTriangleMeshDX11.h"
// 
// #include "Rendering/Media/Shaders/vaSharedTypes.h"


namespace VertexAsylum
{

    class vaGBufferDX11 : public vaGBuffer, VertexAsylum::vaDirectXNotifyTarget
    {
        VA_RENDERING_MODULE_MAKE_FRIENDS( );

    private:
//        vaDirectXVertexShader                   m_vertexShader;
        vaDirectXPixelShader                    m_pixelShader;

        vaDirectXPixelShader                    m_depthToViewspaceLinearPS;
        vaDirectXPixelShader                    m_debugDrawDepthPS;
        vaDirectXPixelShader                    m_debugDrawDepthViewspaceLinearPS;
        vaDirectXPixelShader                    m_debugDrawNormalMapPS;
        vaDirectXPixelShader                    m_debugDrawAlbedoPS;
        vaDirectXPixelShader                    m_debugDrawRadiancePS;

        bool                                    m_shadersDirty;

//        vaDirectXConstantsBuffer< GBufferConstants >
//                                                m_constantsBuffer;

        vector< pair< string, string > >        m_staticShaderMacros;

        wstring                                 m_shaderFileToUse;

    protected:
        vaGBufferDX11( const vaConstructorParamsBase * params );
        ~vaGBufferDX11( );

    private:
        // vaDirectXNotifyTarget
        virtual void                            OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain );
        virtual void                            OnDeviceDestroyed( );

    private:
        const wchar_t *                         GetShaderFilePath( ) const      { return m_shaderFileToUse.c_str();  };

    private:
        // vaGBuffer
        virtual void                            UpdateResources( vaDrawContext & drawContext, int width, int height );

        virtual void                            RenderDebugDraw( vaDrawContext & drawContext );

        virtual void                            DepthToViewspaceLinear( vaDrawContext & drawContext, vaTexture & depthTexture );

    public:
    };

}

using namespace VertexAsylum;


vaGBufferDX11::vaGBufferDX11( const vaConstructorParamsBase * params )
{
    m_shadersDirty = true;

    m_shaderFileToUse                   = L"vaGBuffer.hlsl";

//    m_staticShaderMacros.push_back( std::pair<std::string, std::string>( "INVALID", "INVALID" ) );

    VA_DIRECTX_NOTIFY_TARGET_CONSTRUCT_PATCH( vaGBufferDX11 );
}

vaGBufferDX11::~vaGBufferDX11( )
{
    VA_DIRECTX_NOTIFY_TARGET_DESTRUCT_PATCH( vaGBufferDX11 );
}

void vaGBufferDX11::OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain )
{
//    m_constantsBuffer.Create( );
}

void vaGBufferDX11::OnDeviceDestroyed( )
{
//    m_constantsBuffer.Destroy( );
}

static void ReCreateIfNeeded( shared_ptr<vaTexture> & inoutTex, int width, int height, vaTextureFormat format, float & inoutTotalSizeSum )
{
    inoutTotalSizeSum += width * height * vaTextureFormatHelpers::GetPixelSizeInBytes( format );

    vaTextureBindSupportFlags bindFlags = vaTextureBindSupportFlags::RenderTarget | vaTextureBindSupportFlags::UnorderedAccess | vaTextureBindSupportFlags::ShaderResource;

    if( (width == 0) || (height == 0) || (format == vaTextureFormat::Unknown ) )
    {
        inoutTex = nullptr;
    }
    else
    {
        vaTextureFormat resourceFormat  = format;
        vaTextureFormat srvFormat       = format;
        vaTextureFormat rtvFormat       = format;
        vaTextureFormat dsvFormat       = vaTextureFormat::Unknown;
        vaTextureFormat uavFormat       = format;

        // handle special cases
        if( format == vaTextureFormat::D32_FLOAT )
        {
            bindFlags = (bindFlags & ~(vaTextureBindSupportFlags::RenderTarget | vaTextureBindSupportFlags::UnorderedAccess)) | vaTextureBindSupportFlags::DepthStencil;
            resourceFormat  = vaTextureFormat::R32_TYPELESS;
            srvFormat       = vaTextureFormat::R32_FLOAT;
            rtvFormat       = vaTextureFormat::Unknown;
            dsvFormat       = vaTextureFormat::D32_FLOAT;
            uavFormat       = vaTextureFormat::Unknown;
        }
        else if( format == vaTextureFormat::D24_UNORM_S8_UINT )
        {
            bindFlags = ( bindFlags & ~( vaTextureBindSupportFlags::RenderTarget | vaTextureBindSupportFlags::UnorderedAccess ) ) | vaTextureBindSupportFlags::DepthStencil;
            resourceFormat = vaTextureFormat::R24G8_TYPELESS;
            srvFormat = vaTextureFormat::R24_UNORM_X8_TYPELESS;
            rtvFormat = vaTextureFormat::Unknown;
            dsvFormat = vaTextureFormat::D24_UNORM_S8_UINT;
            uavFormat = vaTextureFormat::Unknown;
        }
        else if( format == vaTextureFormat::R8G8B8A8_UNORM_SRGB )
        {
            //resourceFormat  = vaTextureFormat::R8G8B8A8_TYPELESS;
            //srvFormat       = vaTextureFormat::R8G8B8A8_UNORM_SRGB;
            //rtvFormat       = vaTextureFormat::R8G8B8A8_UNORM_SRGB;
            //uavFormat       = vaTextureFormat::R8G8B8A8_UNORM;
            uavFormat  = vaTextureFormat::Unknown;
            bindFlags &= ~vaTextureBindSupportFlags::UnorderedAccess;
        }

        if( (inoutTex != nullptr) && (inoutTex->GetSizeX() == width) && (inoutTex->GetSizeY()==height) &&
            (inoutTex->GetResourceFormat()==resourceFormat) && (inoutTex->GetSRVFormat()==srvFormat) && (inoutTex->GetRTVFormat()==rtvFormat) && (inoutTex->GetDSVFormat()==dsvFormat) && (inoutTex->GetUAVFormat()==uavFormat) )
            return;

        inoutTex = shared_ptr<vaTexture>( vaTexture::Create2D( resourceFormat, width, height, 1, 1, 1, bindFlags, vaTextureAccessFlags::None, nullptr, 0, srvFormat, rtvFormat, dsvFormat, uavFormat ) );
    }
}

void vaGBufferDX11::UpdateResources( vaDrawContext & drawContext, int width, int height )
{
    if( width == -1 )   width     = drawContext.APIContext.GetViewport().Width;
    if( height == -1 )  height    = drawContext.APIContext.GetViewport().Height;

    m_resolution = vaVector2i( width, height );

    float totalSizeInMB = 0.0f;

    ReCreateIfNeeded( m_depthBuffer,                width, height, m_formats.DepthBuffer,                   totalSizeInMB );    
    ReCreateIfNeeded( m_depthBufferViewspaceLinear, width, height, m_formats.DepthBufferViewspaceLinear,    totalSizeInMB );
    ReCreateIfNeeded( m_normalMap                 , width, height, m_formats.NormalMap,                     totalSizeInMB );
    ReCreateIfNeeded( m_albedo                    , width, height, m_formats.Albedo,                        totalSizeInMB );
    ReCreateIfNeeded( m_radiance                  , width, height, m_formats.Radiance,                      totalSizeInMB );
    ReCreateIfNeeded( m_outputColor               , width, height, m_formats.OutputColor,                   totalSizeInMB );

    totalSizeInMB /= 1024 * 1024;

    m_debugInfo = vaStringTools::Format( "GBuffer (approx. %.2fMB) ", totalSizeInMB );

    //    vaSimpleShadowMapDX11 * simpleShadowMapDX11 = NULL;
    //    if( drawContext.SimpleShadowMap != NULL )
    //        simpleShadowMapDX11 = vaSaferStaticCast<vaSimpleShadowMapDX11*>( drawContext.SimpleShadowMap );
    //
    //    std::vector< std::pair< std::string, std::string > > newStaticShaderMacros;
    //
    //    if( drawContext.SimpleShadowMap != NULL && drawContext.SimpleShadowMap->GetVolumeShadowMapPlugin( ) != NULL )
    //        newStaticShaderMacros.insert( newStaticShaderMacros.end( ), drawContext.SimpleShadowMap->GetVolumeShadowMapPlugin( )->GetShaderMacros( ).begin( ), drawContext.SimpleShadowMap->GetVolumeShadowMapPlugin( )->GetShaderMacros( ).end( ) );
    //
    //    if( newStaticShaderMacros != m_staticShaderMacros )
    //    {
    //        m_staticShaderMacros = newStaticShaderMacros;
    //
    //        m_shadersDirty = true;
    //    }
    //
    if( m_shadersDirty )
    {
        m_shadersDirty = false;
    
        m_depthToViewspaceLinearPS.CreateShaderFromFile(        GetShaderFilePath( ), "ps_5_0", "DepthToViewspaceLinearPS",         m_staticShaderMacros );
        m_debugDrawDepthPS.CreateShaderFromFile(                GetShaderFilePath( ), "ps_5_0", "DebugDrawDepthPS",                 m_staticShaderMacros );
        m_debugDrawDepthViewspaceLinearPS.CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "DebugDrawDepthViewspaceLinearPS",  m_staticShaderMacros );
        m_debugDrawNormalMapPS.CreateShaderFromFile(            GetShaderFilePath( ), "ps_5_0", "DebugDrawNormalMapPS",             m_staticShaderMacros );
        m_debugDrawAlbedoPS.CreateShaderFromFile(               GetShaderFilePath( ), "ps_5_0", "DebugDrawAlbedoPS",                m_staticShaderMacros );
        m_debugDrawRadiancePS.CreateShaderFromFile(             GetShaderFilePath( ), "ps_5_0", "DebugDrawRadiancePS",              m_staticShaderMacros );
    }
}

// draws provided depthTexture (can be the one obtained using GetDepthBuffer( )) into currently selected RT; relies on settings set in vaRenderingGlobals and will assert and return without doing anything if those are not present
void vaGBufferDX11::DepthToViewspaceLinear( vaDrawContext & drawContext, vaTexture & depthTexture )
{
    assert( drawContext.GetRenderingGlobalsUpdated( ) );    if( !drawContext.GetRenderingGlobalsUpdated( ) ) return;
    assert( !m_shadersDirty );                              if( m_shadersDirty ) return;

    vaRenderDeviceContextDX11 * apiContext = drawContext.APIContext.SafeCast<vaRenderDeviceContextDX11*>( );
    ID3D11DeviceContext * dx11Context = apiContext->GetDXImmediateContext( );

    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, GBUFFER_SLOT0 );

    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, depthTexture.SafeCast<vaTextureDX11*>( )->GetSRV( ), GBUFFER_SLOT0 );
    apiContext->FullscreenPassDraw( dx11Context, m_depthToViewspaceLinearPS );

    // Reset
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, GBUFFER_SLOT0 );
}

void vaGBufferDX11::RenderDebugDraw( vaDrawContext & drawContext )
{
    assert( drawContext.GetRenderingGlobalsUpdated( ) );    if( !drawContext.GetRenderingGlobalsUpdated( ) ) return;
    assert( !m_shadersDirty );                              if( m_shadersDirty ) return;

    if( m_debugSelectedTexture == -1 )
        return;

    vaRenderDeviceContextDX11 * apiContext = drawContext.APIContext.SafeCast<vaRenderDeviceContextDX11*>( );
    ID3D11DeviceContext * dx11Context = apiContext->GetDXImmediateContext( );

    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, (ID3D11ShaderResourceView*)nullptr, GBUFFER_SLOT0 );

    if( m_debugSelectedTexture == 0 )
    {
        vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, m_depthBuffer->SafeCast<vaTextureDX11*>()->GetSRV(), GBUFFER_SLOT0 );
        apiContext->FullscreenPassDraw( dx11Context, m_debugDrawDepthPS );
    }
    else if( m_debugSelectedTexture == 1 )
    {
        vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, m_depthBufferViewspaceLinear->SafeCast<vaTextureDX11*>( )->GetSRV( ), GBUFFER_SLOT0 );
        apiContext->FullscreenPassDraw( dx11Context, m_debugDrawDepthViewspaceLinearPS );
    }
    else if( m_debugSelectedTexture == 2 )
    {
        vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, m_normalMap->SafeCast<vaTextureDX11*>( )->GetSRV( ), GBUFFER_SLOT0 );
        apiContext->FullscreenPassDraw( dx11Context, m_debugDrawNormalMapPS );
    }
    else if( m_debugSelectedTexture == 3 )
    {
        vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, m_albedo->SafeCast<vaTextureDX11*>( )->GetSRV( ), GBUFFER_SLOT0 );
        apiContext->FullscreenPassDraw( dx11Context, m_debugDrawAlbedoPS );
    }
    else if( m_debugSelectedTexture == 4 )
    {
        vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, m_radiance->SafeCast<vaTextureDX11*>( )->GetSRV( ), GBUFFER_SLOT0 );
        apiContext->FullscreenPassDraw( dx11Context, m_debugDrawRadiancePS );
    }

    // Reset
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, (ID3D11ShaderResourceView*)nullptr, GBUFFER_SLOT0 );

    // make sure nothing messed with our constant buffers and nothing uses them after
    // vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, m_constantsBuffer.GetBuffer( ), RENDERMESH_CONSTANTS_BUFFERSLOT );
    //vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, (ID3D11Buffer*)NULL, RENDERMESH_CONSTANTS_BUFFERSLOT );
}

void RegisterGBufferDX11( )
{
    VA_RENDERING_MODULE_REGISTER( vaGBuffer, vaGBufferDX11 );
}
