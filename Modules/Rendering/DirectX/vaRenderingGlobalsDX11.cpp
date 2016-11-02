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

#include "Rendering/DirectX/vaRenderingGlobalsDX11.h"

#include "Rendering/DirectX/vaTextureDX11.h"

using namespace VertexAsylum;

vaRenderingGlobalsDX11::vaRenderingGlobalsDX11( const vaConstructorParamsBase * params )
{
    VA_DIRECTX_NOTIFY_TARGET_CONSTRUCT_PATCH( vaRenderingGlobalsDX11 );
}

vaRenderingGlobalsDX11::~vaRenderingGlobalsDX11()
{
    VA_DIRECTX_NOTIFY_TARGET_DESTRUCT_PATCH( vaRenderingGlobalsDX11 );
}

void vaRenderingGlobalsDX11::OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain )
{
    m_constantsBuffer.Create( );

}

void vaRenderingGlobalsDX11::OnDeviceDestroyed( )
{
    m_constantsBuffer.Destroy( );
}

const shared_ptr<vaTexture> & vaRenderingGlobalsDX11::GetCurrentShaderDebugOutput( )
{
    return m_shaderDebugOutputGPUTextures[ (int)(m_frameIndex % c_shaderDebugOutputSyncDelay) ];
}

ID3D11UnorderedAccessView * vaRenderingGlobalsDX11::GetCurrentShaderDebugOutputUAV(  )
{
    return vaSaferStaticCast< vaTextureDX11 * >( GetCurrentShaderDebugOutput().get() )->GetUAV();
}

void vaRenderingGlobalsDX11::SetAPIGlobals( vaDrawContext & drawContext )
{
    ID3D11DeviceContext * dx11Context = vaSaferStaticCast< vaRenderDeviceContextDX11 * >( &drawContext.APIContext )->GetDXImmediateContext( );

    {
        ShaderGlobalConstants consts;
        memset( &consts, 0, sizeof(consts) );

        const vaViewport & viewport = drawContext.APIContext.GetViewport();

        if( drawContext.Lighting != nullptr )
        {
            drawContext.Lighting->UpdateLightingGlobalConstants( drawContext, consts.Lighting );
        }
        else
        {
            memset( &consts.Lighting, 0, sizeof(consts.Lighting) );
        }

        consts.View                             = drawContext.Camera.GetViewMatrix( );
        consts.Proj                             = drawContext.Camera.GetProjMatrix( );

        if( drawContext.PassType == vaRenderPassType::ForwardDebugWireframe )
            drawContext.Camera.GetZOffsettedProjMatrix( consts.Proj, 1.0001f, 0.0001f );

        consts.ViewProj                         = consts.View * consts.Proj;

        {
            consts.ViewportSize                 = vaVector2( (float)viewport.Width, (float)viewport.Height );
            consts.ViewportPixelSize            = vaVector2( 1.0f / (float)viewport.Width, 1.0f / (float)viewport.Height );
            consts.ViewportHalfSize             = vaVector2( (float)viewport.Width*0.5f, (float)viewport.Height*0.5f );
            consts.ViewportPixel2xSize          = vaVector2( 2.0f / (float)viewport.Width, 2.0f / (float)viewport.Height );

            float clipNear  = drawContext.Camera.GetNearPlane();
            float clipFar   = drawContext.Camera.GetFarPlane();
            float depthLinearizeMul = ( clipFar * clipNear ) / ( clipFar - clipNear );
            float depthLinearizeAdd = clipFar / ( clipFar - clipNear );
            consts.DepthUnpackConsts = vaVector2( depthLinearizeMul, depthLinearizeAdd );

            // this will only work if xxxPerspectiveFovLH (or equivalent) is used to create projection matrix
            float tanHalfFOVY = tanf( drawContext.Camera.GetYFOV() * 0.5f );
            float tanHalfFOVX = tanHalfFOVY * drawContext.Camera.GetAspect();
            consts.CameraTanHalfFOV     = vaVector2( tanHalfFOVX, tanHalfFOVY );

            consts.CameraNearFar        = vaVector2( clipNear, clipFar );

            consts.Dummy                = vaVector2( 0.0f, 0.0f );
        }

        consts.TransparencyPass                 = ( drawContext.PassType == vaRenderPassType::ForwardTransparent )?( 1.0f ) : ( 0.0f );
        consts.WireframePass                    = ( drawContext.PassType == vaRenderPassType::ForwardDebugWireframe )?( 1.0f ) : ( 0.0f );
        consts.Dummy0                           = 0.0f;
        consts.Dummy1                           = 0.0f;

        //consts.ProjToWorld = viewProj.Inverse( );

        m_constantsBuffer.Update( dx11Context, consts );
    }

    m_constantsBuffer.SetToD3DContextAllShaderTypes( dx11Context, SHADERGLOBAL_CONSTANTSBUFFERSLOT );

    // Samplers
    ID3D11SamplerState * samplers[6] =
    {
        vaDirectXTools::GetSamplerStatePointClamp( ),
        vaDirectXTools::GetSamplerStatePointWrap( ),
        vaDirectXTools::GetSamplerStateLinearClamp( ),
        vaDirectXTools::GetSamplerStateLinearWrap( ),
        vaDirectXTools::GetSamplerStateAnisotropicClamp( ),
        vaDirectXTools::GetSamplerStateAnisotropicWrap( ),
    };
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, samplers, SHADERGLOBAL_POINTCLAMP_SAMPLERSLOT, _countof( samplers ) );
    // this fills SHADERGLOBAL_POINTCLAMP_SAMPLERSLOT, SHADERGLOBAL_POINTWRAP_SAMPLERSLOT, SHADERGLOBAL_LINEARCLAMP_SAMPLERSLOT, SHADERGLOBAL_LINEARWRAP_SAMPLERSLOT, SHADERGLOBAL_ANISOTROPICCLAMP_SAMPLERSLOT, SHADERGLOBAL_ANISOTROPICWRAP_SAMPLERSLOT
    
    MarkAPIGlobalsUpdated( drawContext );
}

void vaRenderingGlobalsDX11::UpdateDebugOutputFloats( vaDrawContext & drawContext )
{
    // 1.) queue resource GPU->CPU copy
    vaTextureDX11 * src = vaSaferStaticCast< vaTextureDX11 * >( m_shaderDebugOutputGPUTextures[ (int)( m_frameIndex % c_shaderDebugOutputSyncDelay ) ].get() );
    vaTextureDX11 * dst = vaSaferStaticCast< vaTextureDX11 * >( m_shaderDebugOutputCPUTextures[ (int)( m_frameIndex % c_shaderDebugOutputSyncDelay ) ].get() );

    ID3D11DeviceContext * dx11Context = vaSaferStaticCast< vaRenderDeviceContextDX11 * >( &drawContext.APIContext )->GetDXImmediateContext( );
    dx11Context->CopyResource( dst->GetResource(), src->GetResource() );

    // 2.) get data from oldest (next) CPU resource
    vaTextureDX11 * readTex = vaSaferStaticCast< vaTextureDX11 * >( m_shaderDebugOutputCPUTextures[(int)( (m_frameIndex + 1) % c_shaderDebugOutputSyncDelay )].get( ) );
    ID3D11Texture1D * readTexDX11 = readTex->GetTexture1D();
    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = dx11Context->Map( readTexDX11, 0, D3D11_MAP_READ, 0, &mapped );
    if( !FAILED(hr) )
    {
        memcpy( m_shaderDebugFloats, mapped.pData, sizeof(m_shaderDebugFloats) );
        dx11Context->Unmap( readTexDX11, 0 );
    }
    else
    {
        assert( false );
    }
}

void RegisterRenderingGlobals( )
{
    VA_RENDERING_MODULE_REGISTER( vaRenderingGlobals, vaRenderingGlobalsDX11 );
}
