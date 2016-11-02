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

#include "ASSAOWrapper.h"

#include "Rendering/DirectX/vaRenderDeviceDX11.h"
#include "Rendering/DirectX/vaRenderDeviceContextDX11.h"
#include "Rendering/DirectX/vaRenderingToolsDX11.h"
#include "Rendering/DirectX/vaTextureDX11.h"

#ifdef SSAO_ALLOW_INTERNAL_SHADER_DEBUGGING
#include "Rendering/DirectX/vaRenderingGlobalsDX11.h"
#endif

#ifndef _DEBUG
// slows down compilation
#define USE_EMBEDDED_SHADER
#endif

#ifdef USE_EMBEDDED_SHADER
#include "ASSAO/ASSAO_shader.c"
#endif

namespace VertexAsylum
{

    class ASSAOWrapperDX11 : public ASSAOWrapper, VertexAsylum::vaDirectXNotifyTarget
    {
        VA_RENDERING_MODULE_MAKE_FRIENDS( );

    private:
#ifdef SSAODEMO_ENABLE_FINAL_INTEL_SSAO
        ASSAO_Effect *                      m_effect;
#endif

    public:
        ASSAOWrapperDX11( const vaConstructorParamsBase * params );
        ~ASSAOWrapperDX11( );

    private:
        virtual void                            OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain );
        virtual void                            OnDeviceDestroyed( );

    private:
        //
    private:
        // ASSAO
        virtual void                            Draw( vaDrawContext & drawContext, vaTexture & depthTexture, bool blend, vaTexture * normalmapTexture = nullptr, const vaVector4i & scissorRect = vaVector4i( 0, 0, 0, 0 ) );

    public:
    };

}

using namespace VertexAsylum;



ASSAOWrapperDX11::ASSAOWrapperDX11( const vaConstructorParamsBase * params )
{
#ifdef SSAODEMO_ENABLE_FINAL_INTEL_SSAO
    m_effect = nullptr;
#endif

    VA_DIRECTX_NOTIFY_TARGET_CONSTRUCT_PATCH( ASSAOWrapperDX11 );
}

ASSAOWrapperDX11::~ASSAOWrapperDX11( )
{
    VA_DIRECTX_NOTIFY_TARGET_DESTRUCT_PATCH( ASSAOWrapperDX11 );

#ifdef SSAODEMO_ENABLE_FINAL_INTEL_SSAO
    assert( m_effect == nullptr );
#endif
}

void ASSAOWrapperDX11::OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain )
{
#ifdef SSAODEMO_ENABLE_FINAL_INTEL_SSAO

#ifdef USE_EMBEDDED_SHADER
    m_effect = ASSAO_Effect::CreateInstance( &ASSAO_CreateDescDX11(device, c_embedded_assaoshader, sizeof(c_embedded_assaoshader) ) );
#else
    auto shaderData = vaFileTools::LoadFileToMemoryStream( vaCore::GetExecutableDirectory( ) + L"ASSAO/ASSAO.hlsl" );
    m_effect = ASSAO_Effect::CreateInstance( &ASSAO_CreateDescDX11(device, shaderData->GetBuffer(), shaderData->GetLength() ) );
#endif

#endif
}

void ASSAOWrapperDX11::OnDeviceDestroyed( )
{
#ifdef SSAODEMO_ENABLE_FINAL_INTEL_SSAO
    ASSAO_Effect::DestroyInstance( m_effect );
    m_effect = nullptr;
#endif
}

void ASSAOWrapperDX11::Draw( vaDrawContext & drawContext, vaTexture & depthTexture, bool blend, vaTexture * normalmapTexture, const vaVector4i & scissorRect )
{
#ifdef SSAODEMO_ENABLE_FINAL_INTEL_SSAO
    vaRenderDeviceContextDX11 * apiContext = drawContext.APIContext.SafeCast<vaRenderDeviceContextDX11*>( );

    ASSAO_InputsDX11 inputs;
    inputs.ScissorLeft                  = scissorRect.x;
    inputs.ScissorTop                   = scissorRect.y;
    inputs.ScissorRight                 = scissorRect.z;
    inputs.ScissorBottom                = scissorRect.w;
    inputs.DeviceContext                = apiContext->GetDXImmediateContext();
    inputs.ProjectionMatrix             = ASSAO_Float4x4( (const float*)&drawContext.Camera.GetProjMatrix()._11 );
    inputs.ViewportWidth                = drawContext.APIContext.GetViewport().Width;
    inputs.ViewportHeight               = drawContext.APIContext.GetViewport().Height;
    inputs.DepthSRV                     = depthTexture.SafeCast<vaTextureDX11*>( )->GetSRV( );
    inputs.NormalSRV                    = (normalmapTexture != nullptr)?(normalmapTexture->SafeCast<vaTextureDX11*>( )->GetSRV( )):(nullptr);
    inputs.MatricesRowMajorOrder        = true;
    inputs.DrawOpaque                   = !blend;

    m_effect->Draw( m_settings, &inputs );
#endif
}


void RegisterASSAOWrapperDX11( )
{
    VA_RENDERING_MODULE_REGISTER( ASSAOWrapper, ASSAOWrapperDX11 );
}

