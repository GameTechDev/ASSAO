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

#include "Rendering/DirectX/vaDirectXCore.h"
#include "Rendering/DirectX/vaDirectXShader.h"
#include "Rendering/DirectX/vaDirectXTools.h"

#include "Scene/vaSceneIncludes.h"

#include "Rendering/vaRenderingIncludes.h"

#include "Rendering/Effects/vaSky.h"
#include "Rendering/DirectX/vaRenderDeviceContextDX11.h"

#include "Rendering/Media/Shaders/vaSharedTypes.h"

namespace VertexAsylum
{

    class vaSkyDX11 : public vaSky, vaDirectXNotifyTarget
    {
        VA_RENDERING_MODULE_MAKE_FRIENDS( );
    private:

        vaDirectXVertexShader               m_vertexShader;
        vaDirectXPixelShader                m_pixelShader;

        vaDirectXVertexBuffer < vaVector3 > m_screenTriangleVertexBuffer;
        vaDirectXVertexBuffer < vaVector3 > m_screenTriangleVertexBufferReversedZ;
        ID3D11DepthStencilState *           m_depthStencilState;

        vaDirectXConstantsBuffer < SimpleSkyConstants >
                                            m_constantsBuffer;

    protected:
        vaSkyDX11( const vaConstructorParamsBase * params );
        ~vaSkyDX11( );

    private:
        // vaDirectXNotifyTarget
        virtual void                    OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain );
        virtual void                    OnDeviceDestroyed( );
    private:
        // RenderingObject
        virtual void                    Draw( vaDrawContext & drawContext );
    };

}

using namespace VertexAsylum;


vaSkyDX11::vaSkyDX11( const vaConstructorParamsBase * params )
{
    m_depthStencilState = NULL;

    D3D11_INPUT_ELEMENT_DESC inputElements[] =
    {
        { "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    m_vertexShader.CreateShaderAndILFromFile( L"vaSky.hlsl", "vs_5_0", "SimpleSkyboxVS", NULL, inputElements, _countof( inputElements ) );
    m_pixelShader.CreateShaderFromFile( L"vaSky.hlsl", "ps_5_0", "SimpleSkyboxPS", NULL );

    VA_DIRECTX_NOTIFY_TARGET_CONSTRUCT_PATCH( vaSkyDX11 );
}

vaSkyDX11::~vaSkyDX11( )
{
    VA_DIRECTX_NOTIFY_TARGET_DESTRUCT_PATCH( vaSkyDX11 );
}

void vaSkyDX11::OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain )
{
    HRESULT hr;

    // Create screen triangle vertex buffer
    {
        const float skyFarZ = 1.0f;
        vaVector3 screenTriangle[3];
        screenTriangle[0] = vaVector3( -1.0f, 1.0f, skyFarZ );
        screenTriangle[1] = vaVector3( 3.0f, 1.0f, skyFarZ );
        screenTriangle[2] = vaVector3( -1.0f, -3.0f, skyFarZ );

        m_screenTriangleVertexBuffer.Create( 3, screenTriangle );
    }

    // Create screen triangle vertex buffer
    {
        const float skyFarZ = 0.0f;
        vaVector3 screenTriangle[3];
        screenTriangle[0] = vaVector3( -1.0f, 1.0f, skyFarZ );
        screenTriangle[1] = vaVector3( 3.0f, 1.0f, skyFarZ );
        screenTriangle[2] = vaVector3( -1.0f, -3.0f, skyFarZ );

        m_screenTriangleVertexBufferReversedZ.Create( 3, screenTriangle );
    }

    // Create depth stencil
   {
       CD3D11_DEPTH_STENCIL_DESC desc = CD3D11_DEPTH_STENCIL_DESC( CD3D11_DEFAULT( ) );
       desc.DepthEnable = TRUE;
       desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
       desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

       V( device->CreateDepthStencilState( &desc, &m_depthStencilState ) );
   }

   m_constantsBuffer.Create( );
}

void vaSkyDX11::OnDeviceDestroyed( )
{
    m_screenTriangleVertexBuffer.Destroy( );
    m_screenTriangleVertexBufferReversedZ.Destroy( );
    SAFE_RELEASE( m_depthStencilState );
    m_constantsBuffer.Destroy( );
}


void vaSkyDX11::Draw( vaDrawContext & drawContext )
{
    if( drawContext.PassType != vaRenderPassType::ForwardOpaque )
        return;

    ID3D11DeviceContext * dx11Context = vaSaferStaticCast< vaRenderDeviceContextDX11 * >( &drawContext.APIContext )->GetDXImmediateContext( );

    {
        const vaSky::Settings & settings = GetSettings( );
        SimpleSkyConstants consts;

        vaMatrix4x4 view = drawContext.Camera.GetViewMatrix( );
        vaMatrix4x4 proj = drawContext.Camera.GetProjMatrix( );

        view.r3 = vaVector4( 0.0f, 0.0f, 0.0f, 1.0f );

        vaMatrix4x4 viewProj = view * proj;

        consts.ProjToWorld = viewProj.Inverse( );
        consts.SunDir = vaVector4( GetSunDir( ), 1.0f );

        consts.SkyColorLowPow = settings.SkyColorLowPow;
        consts.SkyColorLowMul = settings.SkyColorLowMul;
        consts.SkyColorLow = settings.SkyColorLow;
        consts.SkyColorHigh = settings.SkyColorHigh;
        consts.SunColorPrimary = settings.SunColorPrimary;
        consts.SunColorSecondary = settings.SunColorSecondary;
        consts.SunColorPrimaryPow = settings.SunColorPrimaryPow;
        consts.SunColorPrimaryMul = settings.SunColorPrimaryMul;
        consts.SunColorSecondaryPow = settings.SunColorSecondaryPow;
        consts.SunColorSecondaryMul = settings.SunColorSecondaryMul;

        m_constantsBuffer.Update( dx11Context, consts );
    }

    // make sure we're not overwriting anything else, and set our constant buffers
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, (ID3D11Buffer*)NULL, SIMPLESKY_CONSTANTS_BUFFERSLOT );
    m_constantsBuffer.SetToD3DContextAllShaderTypes( dx11Context, SIMPLESKY_CONSTANTS_BUFFERSLOT );

    if( drawContext.Camera.GetUseReversedZ() )
        m_screenTriangleVertexBufferReversedZ.SetToD3DContext( dx11Context );
    else
        m_screenTriangleVertexBuffer.SetToD3DContext( dx11Context );

    dx11Context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    dx11Context->IASetInputLayout( m_vertexShader.GetInputLayout( ) );
    dx11Context->VSSetShader( m_vertexShader.GetShader( ), NULL, 0 );
    dx11Context->PSSetShader( m_pixelShader.GetShader( ), NULL, 0 );

    dx11Context->RSSetState( NULL );
    dx11Context->OMSetDepthStencilState( (drawContext.Camera.GetUseReversedZ())?( vaDirectXTools::GetDSS_DepthEnabledGE_NoDepthWrite( ) ):( vaDirectXTools::GetDSS_DepthEnabledLE_NoDepthWrite( ) ), 0 );
    float blendFactor[4] = { 0, 0, 0, 0 };
    dx11Context->OMSetBlendState( NULL, blendFactor, 0xFFFFFFFF );

    dx11Context->Draw( 3, 0 );

    // make sure nothing messed with our constant buffers and nothing uses them after
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, m_constantsBuffer.GetBuffer( ), SIMPLESKY_CONSTANTS_BUFFERSLOT );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, (ID3D11Buffer*)NULL, SIMPLESKY_CONSTANTS_BUFFERSLOT );
}

void RegisterSkyDX11( )
{
    VA_RENDERING_MODULE_REGISTER( vaSky, vaSkyDX11 );
}