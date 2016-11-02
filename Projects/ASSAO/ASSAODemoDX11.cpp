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
// 2016-09-07: filip.strugar@intel.com: first commit
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Rendering/DirectX/vaRenderDeviceDX11.h"
#include "Rendering/DirectX/vaRenderDeviceContextDX11.h"

#include "ASSAODemo.h"

#include "Rendering/DirectX/vaTriangleMeshDX11.h"
#include "Rendering/DirectX/vaRenderingToolsDX11.h"

#include "Media/Shaders/SSAODemoGlobals.hlsl"

namespace VertexAsylum
{

    class SSAODemoDX11 : public ASSAODemo, VertexAsylum::vaDirectXNotifyTarget
    {
        VA_RENDERING_MODULE_MAKE_FRIENDS( );

    private:
        vaDirectXConstantsBuffer < SSAODemoGlobalConstants >    m_constantsBuffer;

        vaDirectXPixelShader                                    m_overlayPS;
        vaDirectXPixelShader                                    m_opacityGraphPS;
        std::vector< std::pair< std::string, std::string > >    m_staticShaderMacros;
        bool                                                    m_shadersDirty;

    private:

        SSAODemoDX11( const vaConstructorParamsBase * params );
        ~SSAODemoDX11( );
    private:
        // vaDirectXNotifyTarget
        virtual void                    OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain );
        virtual void                    OnDeviceDestroyed( );

        //   private:
        //       // RenderingObject
        //       virtual void                    Draw( vaDrawContext & drawContext );

        void                            UpdateShadersIfDirty( vaDrawContext & drawContext );
        void                            UpdateConstants( vaDrawContext & drawContext );

    private:
        // ASSAODemo
        virtual void                    DrawDebugOverlay( vaDrawContext & drawContext );
    };

}

using namespace VertexAsylum;


SSAODemoDX11::SSAODemoDX11( const vaConstructorParamsBase * params )
{
    m_shadersDirty = true;

    VA_DIRECTX_NOTIFY_TARGET_CONSTRUCT_PATCH( SSAODemoDX11 );
}

SSAODemoDX11::~SSAODemoDX11( )
{
    VA_DIRECTX_NOTIFY_TARGET_DESTRUCT_PATCH( SSAODemoDX11 );
}

void SSAODemoDX11::OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain )
{
    m_constantsBuffer.Create( );
}

void SSAODemoDX11::OnDeviceDestroyed( )
{
    m_constantsBuffer.Destroy( );
}

void SSAODemoDX11::UpdateConstants( vaDrawContext & drawContext )
{
    ID3D11DeviceContext * dx11Context = vaDirectXCore::GetImmediateContext( );
    vaRenderDeviceContextDX11 * canvasDX11 = vaSaferStaticCast< vaRenderDeviceContextDX11 * >( &drawContext.APIContext );

    {
        SSAODemoGlobalConstants consts;

        consts.World = vaMatrix4x4::Identity;
        consts.WorldView = vaMatrix4x4::Identity;

        // consts.DebugTexelLocation               = m_VSMDebuggingSettings.DebugTexelLocation;    
        // consts.DebugOverlayDepth                = m_VSMDebuggingSettings.DebugOverlayDepth;
        // consts.DebugOverlayScale                = m_VSMDebuggingSettings.DebugOverlayScale;

        m_constantsBuffer.Update( dx11Context, consts );
    }
}

void SSAODemoDX11::UpdateShadersIfDirty( vaDrawContext & drawContext )
{
    ID3D11DeviceContext * dx11Context = vaDirectXCore::GetImmediateContext( );

    std::vector< std::pair< std::string, std::string > > newStaticShaderMacros;

    if( ( drawContext.SimpleShadowMap != NULL ) && ( drawContext.SimpleShadowMap->GetVolumeShadowMapPlugin( ) != NULL ) )
        newStaticShaderMacros.insert( newStaticShaderMacros.end( ), drawContext.SimpleShadowMap->GetVolumeShadowMapPlugin( )->GetShaderMacros( ).begin( ), drawContext.SimpleShadowMap->GetVolumeShadowMapPlugin( )->GetShaderMacros( ).end( ) );

    if( newStaticShaderMacros != m_staticShaderMacros )
    {
        m_staticShaderMacros = newStaticShaderMacros;
        m_shadersDirty = true;
    }

    if( m_shadersDirty )
    {
        m_shadersDirty = false;

        std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements = vaVertexInputLayoutsDX11::BillboardSpriteVertexDecl( );

        m_overlayPS.CreateShaderFromFile( L"VSMGlobals.hlsl", "ps_5_0", "AVSMDebugOverlayPS", m_staticShaderMacros );
        m_opacityGraphPS.CreateShaderFromFile( L"VSMGlobals.hlsl", "ps_5_0", "AVSMDebugOpacityGraphPS", m_staticShaderMacros );
    }
}

void SSAODemoDX11::DrawDebugOverlay( vaDrawContext & drawContext )
{
    UpdateShadersIfDirty( drawContext );
    UpdateConstants( drawContext );

    ID3D11DeviceContext * dx11Context = vaDirectXCore::GetImmediateContext( );
    vaRenderDeviceContextDX11 * canvasDX11 = vaSaferStaticCast< vaRenderDeviceContextDX11 * >( &drawContext.APIContext );

    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, (ID3D11Buffer*)NULL, SSAODEMOGLOBAL_CONSTANTS_BUFFERSLOT );
    m_constantsBuffer.SetToD3DContextAllShaderTypes( dx11Context, SSAODEMOGLOBAL_CONSTANTS_BUFFERSLOT );

    canvasDX11->FullscreenPassDraw( dx11Context, m_overlayPS.GetShader( ) );

    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, (ID3D11Buffer*)NULL, SSAODEMOGLOBAL_CONSTANTS_BUFFERSLOT );
}


void RegisterASSAOWrapperDX11( );
void RegisterExternalSSAOWrapperDX11( );

void RegisterSSAODemoDX11( )
{
    VA_RENDERING_MODULE_REGISTER( ASSAODemo, SSAODemoDX11 );
    RegisterASSAOWrapperDX11( );
    RegisterExternalSSAOWrapperDX11( );
}
