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

#include "Rendering/vaLighting.h"

#include "Rendering/DirectX/vaDirectXCore.h"
#include "Rendering/DirectX/vaDirectXShader.h"
#include "Rendering/DirectX/vaDirectXTools.h"
#include "Rendering/DirectX/vaRenderDeviceContextDX11.h"
#include "Rendering/DirectX/vaTextureDX11.h"

#include "Rendering/DirectX/Effects/vaSimpleShadowMapDX11.h"

#define LIGHTING_SLOT0   0
#define LIGHTING_SLOT1   1
#define LIGHTING_SLOT2   2

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

    class vaLightingDX11 : public vaLighting, VertexAsylum::vaDirectXNotifyTarget
    {
        VA_RENDERING_MODULE_MAKE_FRIENDS( );

    private:
//        vaDirectXVertexShader                   m_vertexShader;

        vaDirectXPixelShader                    m_applyDirectionalAmbientPS;
        vaDirectXPixelShader                    m_applyDirectionalAmbientShadowedPS;

        //vaDirectXPixelShader                    m_applyTonemapPS;

        bool                                    m_shadersDirty;

//        vaDirectXConstantsBuffer< GBufferConstants >
//                                                m_constantsBuffer;

        vector< pair< string, string > >        m_staticShaderMacros;

        wstring                                 m_shaderFileToUse;

    protected:
        vaLightingDX11( const vaConstructorParamsBase * params );
        ~vaLightingDX11( );

    private:
        // vaDirectXNotifyTarget
        virtual void                            OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain );
        virtual void                            OnDeviceDestroyed( );

    private:
        const wchar_t *                         GetShaderFilePath( ) const      { return m_shaderFileToUse.c_str();  };

    private:
        void                                    UpdateResourcesIfNeeded( vaDrawContext & drawContext );
        virtual void                            ApplyDirectionalAmbientLighting( vaDrawContext & drawContext, vaGBuffer & GBuffer );
        virtual void                            ApplyDynamicLighting( vaDrawContext & drawContext, vaGBuffer & GBuffer );
        //virtual void                            ApplyTonemap( vaDrawContext & drawContext, vaGBuffer & GBuffer );

    public:
    };

}

using namespace VertexAsylum;


vaLightingDX11::vaLightingDX11( const vaConstructorParamsBase * params )
{
    m_shadersDirty = true;

    m_shaderFileToUse                   = L"vaLighting.hlsl";

//    m_staticShaderMacros.push_back( std::pair<std::string, std::string>( "INVALID", "INVALID" ) );

    VA_DIRECTX_NOTIFY_TARGET_CONSTRUCT_PATCH( vaLightingDX11 );
}

vaLightingDX11::~vaLightingDX11( )
{
    VA_DIRECTX_NOTIFY_TARGET_DESTRUCT_PATCH( vaLightingDX11 );
}

void vaLightingDX11::OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain )
{
//    m_constantsBuffer.Create( );
}

void vaLightingDX11::OnDeviceDestroyed( )
{
//    m_constantsBuffer.Destroy( );
}


void vaLightingDX11::UpdateResourcesIfNeeded( vaDrawContext & drawContext )
{
    if( m_shadersDirty )
    {
        m_shadersDirty = false;

        m_applyDirectionalAmbientPS.CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "ApplyDirectionalAmbientPS", m_staticShaderMacros );
        m_applyDirectionalAmbientShadowedPS.CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "ApplyDirectionalAmbientShadowedPS", m_staticShaderMacros );

        //m_applyTonemapPS.CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "ApplyTonemapPS", m_staticShaderMacros );
    }
}

// draws provided depthTexture (can be the one obtained using GetDepthBuffer( )) into currently selected RT; relies on settings set in vaRenderingGlobals and will assert and return without doing anything if those are not present
void vaLightingDX11::ApplyDirectionalAmbientLighting( vaDrawContext & drawContext, vaGBuffer & GBuffer )
{
    assert( drawContext.GetRenderingGlobalsUpdated( ) );    if( !drawContext.GetRenderingGlobalsUpdated( ) ) return;
    UpdateResourcesIfNeeded( drawContext );
    assert( !m_shadersDirty );                              if( m_shadersDirty ) return;
    
    vaDirectXPixelShader    * pixelShader = &m_applyDirectionalAmbientPS;

    // Shadows?
    vaSimpleShadowMapDX11 * simpleShadowMapDX11 = NULL;
    if( drawContext.SimpleShadowMap != NULL )
    {
        simpleShadowMapDX11 = vaSaferStaticCast<vaSimpleShadowMapDX11*>( drawContext.SimpleShadowMap );
        pixelShader = &m_applyDirectionalAmbientShadowedPS;
    }

    vaRenderDeviceContextDX11 * apiContext = drawContext.APIContext.SafeCast<vaRenderDeviceContextDX11*>( );
    ID3D11DeviceContext * dx11Context = apiContext->GetDXImmediateContext( );

    // make sure we're not overwriting someone's stuff
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, LIGHTING_SLOT0 );
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, LIGHTING_SLOT1 );
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, LIGHTING_SLOT2 );

    // gbuffer stuff
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, GBuffer.GetDepthBufferViewspaceLinear( )->SafeCast<vaTextureDX11*>( )->GetSRV( ),   LIGHTING_SLOT0 );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, GBuffer.GetAlbedo( )->SafeCast<vaTextureDX11*>( )->GetSRV( ),                       LIGHTING_SLOT1 );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, GBuffer.GetNormalMap( )->SafeCast<vaTextureDX11*>( )->GetSRV( ),                    LIGHTING_SLOT2 );

    // draw but only on things that are already in the zbuffer
    apiContext->FullscreenPassDraw( dx11Context, pixelShader->GetShader(), vaDirectXTools::GetBS_Additive(), vaDirectXTools::GetDSS_DepthEnabledG_NoDepthWrite(), 0, nullptr, 1.0f );

    //    Reset, leave stuff clean
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, (ID3D11ShaderResourceView*) nullptr,  LIGHTING_SLOT0 );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, (ID3D11ShaderResourceView*) nullptr,  LIGHTING_SLOT1 );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, (ID3D11ShaderResourceView*) nullptr,  LIGHTING_SLOT2 );
}

void vaLightingDX11::ApplyDynamicLighting( vaDrawContext & drawContext, vaGBuffer & GBuffer )
{

}

//void vaLightingDX11::ApplyTonemap( vaDrawContext & drawContext, vaGBuffer & GBuffer )
//{
//    assert( drawContext.GetRenderingGlobalsUpdated( ) );    if( !drawContext.GetRenderingGlobalsUpdated( ) ) return;
//    UpdateResourcesIfNeeded( drawContext );
//    assert( !m_shadersDirty );                              if( m_shadersDirty ) return;
//
//    vaRenderDeviceContextDX11 * apiContext = drawContext.APIContext.SafeCast<vaRenderDeviceContextDX11*>( );
//    ID3D11DeviceContext * dx11Context = apiContext->GetDXImmediateContext( );
//
//    // make sure we're not overwriting someone's stuff
//    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, LIGHTING_SLOT0 );
//
//    // source is accumulated radiance
//    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, GBuffer.GetRadiance( )->SafeCast<vaTextureDX11*>( )->GetSRV( ), LIGHTING_SLOT0 );
//
//    // draw but only on things that are already in the zbuffer
//    apiContext->FullscreenPassDraw( dx11Context, m_applyTonemapPS, vaDirectXTools::GetBS_Additive( ), vaDirectXTools::GetDSS_DepthEnabledG_NoDepthWrite( ), 0, nullptr, 1.0f );
//
//    //    Reset, leave stuff clean
//    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* ) nullptr, LIGHTING_SLOT0 );
//}

void RegisterLightingDX11( )
{
    VA_RENDERING_MODULE_REGISTER( vaLighting, vaLightingDX11 );
}
