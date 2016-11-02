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

#include "Rendering/DirectX/Effects/vaSimpleShadowMapDX11.h"

#include "Scene/vaSceneIncludes.h"

#include "Rendering/vaRenderingIncludes.h"

#include "Rendering/DirectX/vaRenderDeviceContextDX11.h"

using namespace VertexAsylum;

vaSimpleShadowMapDX11::vaSimpleShadowMapDX11( const vaConstructorParamsBase * params )
{
    m_prevCanvasRT = NULL;
    m_prevCanvasDS = NULL;

    m_shadowCmpSamplerState = NULL;

    m_depthSlopeDirty = true;
    m_rasterizerStateCullNone = NULL;
    m_rasterizerStateCullCW   = NULL;
    m_rasterizerStateCullCCW  = NULL;

    VA_DIRECTX_NOTIFY_TARGET_CONSTRUCT_PATCH( vaSimpleShadowMapDX11 );
}

vaSimpleShadowMapDX11::~vaSimpleShadowMapDX11( )
{
    assert( m_prevCanvasRT == NULL );
    assert( m_prevCanvasDS == NULL );

    VA_DIRECTX_NOTIFY_TARGET_DESTRUCT_PATCH( vaSimpleShadowMapDX11 );
}

void vaSimpleShadowMapDX11::InternalResolutionOrTexelWorldSizeChanged( )                
{ 
    m_depthSlopeDirty = true;
    SAFE_RELEASE( m_rasterizerStateCullNone );
    SAFE_RELEASE( m_rasterizerStateCullCW );
    SAFE_RELEASE( m_rasterizerStateCullCCW );
}

void vaSimpleShadowMapDX11::OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain )
{
    HRESULT hr;

    // samplers
    {
        CD3D11_SAMPLER_DESC desc = CD3D11_SAMPLER_DESC( CD3D11_DEFAULT( ) );

        desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;   // D3D11_FILTER_COMPARISON_ANISOTROPIC
        desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;

        V( device->CreateSamplerState( &desc, &m_shadowCmpSamplerState ) );
    }

   m_constantsBuffer.Create( );

   assert( m_prevCanvasRT == NULL );
   assert( m_prevCanvasDS == NULL );
}

void vaSimpleShadowMapDX11::OnDeviceDestroyed( )
{
//    m_screenTriangleVertexBuffer.Destroy( );
//    SAFE_RELEASE( m_depthStencilState );
    m_constantsBuffer.Destroy( );

    assert( m_prevCanvasRT == NULL );
    assert( m_prevCanvasDS == NULL );

    SAFE_RELEASE( m_shadowCmpSamplerState );

    m_depthSlopeDirty = true;
    SAFE_RELEASE( m_rasterizerStateCullNone );
    SAFE_RELEASE( m_rasterizerStateCullCW   );
    SAFE_RELEASE( m_rasterizerStateCullCCW  );
}

void vaSimpleShadowMapDX11::InternalStartGenerating( const vaDrawContext & drawContext )
{
    ID3D11DeviceContext * dx11Context = vaSaferStaticCast< vaRenderDeviceContextDX11 * >( &drawContext.APIContext )->GetDXImmediateContext( );

    if( m_depthSlopeDirty )
    {
        m_depthSlopeDirty = false;

        ID3D11Device * device = NULL;
        dx11Context->GetDevice( &device );

        CD3D11_RASTERIZER_DESC desc = CD3D11_RASTERIZER_DESC( CD3D11_DEFAULT( ) );

        float biasBase  = GetTexelSize().Length() / GetVolume().Extents.z;

        if( SHADERSIMPLESHADOWSGLOBAL_QUALITY == 1 )
            biasBase *= 3.0f;
        else if( SHADERSIMPLESHADOWSGLOBAL_QUALITY == 2 )
            biasBase *= 4.5f;

        desc.DepthBias              = (int)(biasBase * 15000.0f);
        desc.SlopeScaledDepthBias   = biasBase * 1500.0f;
        desc.DepthBiasClamp         = biasBase * 3.0f;

        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_NONE;

        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_NONE;
        device->CreateRasterizerState( &desc, &m_rasterizerStateCullNone );

        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_BACK;
        desc.FrontCounterClockwise = true;
        device->CreateRasterizerState( &desc, &m_rasterizerStateCullCW );

        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_BACK;
        desc.FrontCounterClockwise = false;
        device->CreateRasterizerState( &desc, &m_rasterizerStateCullCCW );

        SAFE_RELEASE( device );
    }

    {
        ShaderSimpleShadowsGlobalConstants consts;

        consts.View         = GetViewMatrix();
        consts.Proj         = GetProjMatrix( );
        consts.ViewProj     = GetViewProjMatrix( );

        consts.CameraViewToShadowView               = drawContext.Camera.GetInvViewMatrix( )  * GetViewMatrix( );
        consts.CameraViewToShadowViewProj           = drawContext.Camera.GetInvViewMatrix( )  * GetViewProjMatrix( );

        vaMatrix4x4 toUVNormalizedSpace             = vaMatrix4x4::Scaling( 0.5f, -0.5f, 1.0f ) * vaMatrix4x4::Translation( +0.5f, +0.5f, 0.0f );

        consts.CameraViewToShadowUVNormalizedSpace  = consts.CameraViewToShadowViewProj * toUVNormalizedSpace;

        consts.ShadowMapRes                         = (float)GetResolution( );
        consts.OneOverShadowMapRes                  = 1.0f / (float)GetResolution();
        consts.Dummy2                               = 0.0f;
        consts.Dummy3                               = 0.0f;

        m_constantsBuffer.Update( dx11Context, consts );

        // make sure we're not overwriting anything else, and set our constant buffers
        vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, (ID3D11Buffer*)NULL, SHADERSIMPLESHADOWSGLOBAL_CONSTANTSBUFFERSLOT );
        m_constantsBuffer.SetToD3DContextAllShaderTypes( dx11Context, SHADERSIMPLESHADOWSGLOBAL_CONSTANTSBUFFERSLOT );
    }

    if( drawContext.PassType != vaRenderPassType::GenerateVolumeShadowmap )
    {

        vaTextureDX11 * shadowMapTextureDX11   = vaSaferStaticCast<vaTextureDX11*>( GetShadowMapTexture( ).get() );

        assert( m_prevCanvasRT == NULL );
        assert( m_prevCanvasDS == NULL );

        m_prevCanvasVP = drawContext.APIContext.GetViewport();
        m_prevCanvasRT = drawContext.APIContext.GetRenderTarget();
        m_prevCanvasDS = drawContext.APIContext.GetDepthStencil();

        drawContext.APIContext.SetRenderTarget( NULL, GetShadowMapTexture( ), true );

        dx11Context->ClearDepthStencilView( shadowMapTextureDX11->GetDSV(), D3D11_CLEAR_DEPTH, 1.0f, 0 );
    }
}

void vaSimpleShadowMapDX11::InternalStopGenerating( const vaDrawContext & drawContext )
{
    ID3D11DeviceContext * dx11Context = vaSaferStaticCast< vaRenderDeviceContextDX11 * >( &drawContext.APIContext )->GetDXImmediateContext( );

    // make sure nothing messed with our constant buffers and nothing uses them after
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, m_constantsBuffer.GetBuffer(), SHADERSIMPLESHADOWSGLOBAL_CONSTANTSBUFFERSLOT );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, (ID3D11Buffer*)NULL, SHADERSIMPLESHADOWSGLOBAL_CONSTANTSBUFFERSLOT );

    if( drawContext.PassType != vaRenderPassType::GenerateVolumeShadowmap )
    {
        assert( drawContext.APIContext.GetRenderTarget( ) == NULL );
        assert( drawContext.APIContext.GetDepthStencil( ) == GetShadowMapTexture( ) );

        drawContext.APIContext.SetRenderTarget( m_prevCanvasRT, m_prevCanvasDS, false );
        drawContext.APIContext.SetViewport( m_prevCanvasVP );
        m_prevCanvasRT = NULL;
        m_prevCanvasDS = NULL;
        m_prevCanvasVP = vaViewport();
    }
}

void vaSimpleShadowMapDX11::InternalStartUsing( const vaDrawContext & drawContext )
{ 
    ID3D11DeviceContext * dx11Context = vaSaferStaticCast< vaRenderDeviceContextDX11 * >( &drawContext.APIContext )->GetDXImmediateContext( );

    // make sure we're not overwriting anything
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, (ID3D11Buffer*)NULL, SHADERSIMPLESHADOWSGLOBAL_CONSTANTSBUFFERSLOT );
    m_constantsBuffer.SetToD3DContextAllShaderTypes( dx11Context, SHADERSIMPLESHADOWSGLOBAL_CONSTANTSBUFFERSLOT );

    vaTextureDX11 * shadowMapTextureDX11 = vaSaferStaticCast<vaTextureDX11*>( GetShadowMapTexture( ).get( ) );

    // make sure we're not overwriting anything else, and set our sampler and shadow map texture
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, (ID3D11SamplerState*)NULL, SHADERSIMPLESHADOWSGLOBAL_CMPSAMPLERSLOT );
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, (ID3D11ShaderResourceView*)NULL, SHADERSIMPLESHADOWSGLOBAL_TEXTURESLOT );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, m_shadowCmpSamplerState, SHADERSIMPLESHADOWSGLOBAL_CMPSAMPLERSLOT );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, shadowMapTextureDX11->GetSRV(), SHADERSIMPLESHADOWSGLOBAL_TEXTURESLOT );

    assert( m_prevCanvasRT == NULL );
    assert( m_prevCanvasDS == NULL );
}

void vaSimpleShadowMapDX11::InternalStopUsing( const vaDrawContext & drawContext )
{ 
    ID3D11DeviceContext * dx11Context = vaSaferStaticCast< vaRenderDeviceContextDX11 * >( &drawContext.APIContext )->GetDXImmediateContext( );

    vaTextureDX11 * shadowMapTextureDX11 = vaSaferStaticCast<vaTextureDX11*>( GetShadowMapTexture( ).get( ) );

    // make sure nothing messed with our sampler and shadow map texture and nothing uses them after
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, m_shadowCmpSamplerState, SHADERSIMPLESHADOWSGLOBAL_CMPSAMPLERSLOT );
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, shadowMapTextureDX11->GetSRV( ), SHADERSIMPLESHADOWSGLOBAL_TEXTURESLOT );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, (ID3D11SamplerState*)NULL, SHADERSIMPLESHADOWSGLOBAL_CMPSAMPLERSLOT );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, (ID3D11ShaderResourceView*)NULL, SHADERSIMPLESHADOWSGLOBAL_TEXTURESLOT );

    // make sure nothing messed with our constant buffers and nothing uses them after
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, m_constantsBuffer.GetBuffer( ), SHADERSIMPLESHADOWSGLOBAL_CONSTANTSBUFFERSLOT );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, (ID3D11Buffer*)NULL, SHADERSIMPLESHADOWSGLOBAL_CONSTANTSBUFFERSLOT );

}

void RegisterSimpleShadowMapDX11( )
{
    VA_RENDERING_MODULE_REGISTER( vaSimpleShadowMap, vaSimpleShadowMapDX11 );
}

