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

#include "Rendering/Effects/vaPostProcessBlur.h"

#include "Rendering/DirectX/vaDirectXCore.h"
#include "Rendering/DirectX/vaDirectXShader.h"
#include "Rendering/DirectX/vaDirectXTools.h"
#include "Rendering/DirectX/vaRenderDeviceContextDX11.h"
#include "Rendering/DirectX/vaRenderDeviceDX11.h"
#include "Rendering/DirectX/vaTextureDX11.h"

namespace VertexAsylum
{

    class vaPostProcessBlurDX11 : public vaPostProcessBlur, VertexAsylum::vaDirectXNotifyTarget
    {
        VA_RENDERING_MODULE_MAKE_FRIENDS( );

    private:
        vaDirectXPixelShader                    m_pixelShaderBlur;

        bool                                    m_shadersDirty;

        vaDirectXConstantsBuffer< PostProcessConstants >
                                                m_constantsBuffer;

        vector< pair< string, string > >        m_staticShaderMacros;

    protected:
        vaPostProcessBlurDX11( const vaConstructorParamsBase * params );
        ~vaPostProcessBlurDX11( );

    private:
        // vaDirectXNotifyTarget
        virtual void                            OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain );
        virtual void                            OnDeviceDestroyed( );

    private:
        void                                    UpdateShaders( vaDrawContext & drawContext );

    public:
        virtual void                            Blur( vaDrawContext & drawContext, vaTexture & srcInput );
    };

}

using namespace VertexAsylum;


vaPostProcessBlurDX11::vaPostProcessBlurDX11( const vaConstructorParamsBase * params )
{
    m_shadersDirty = true;

    //    m_staticShaderMacros.push_back( std::pair<std::string, std::string>( "INVALID", "INVALID" ) );

    VA_DIRECTX_NOTIFY_TARGET_CONSTRUCT_PATCH( vaPostProcessBlurDX11 );
}

vaPostProcessBlurDX11::~vaPostProcessBlurDX11( )
{
    VA_DIRECTX_NOTIFY_TARGET_DESTRUCT_PATCH( vaPostProcessBlurDX11 );
}

void vaPostProcessBlurDX11::OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain )
{
    m_constantsBuffer.Create( );
}

void vaPostProcessBlurDX11::OnDeviceDestroyed( )
{
    m_constantsBuffer.Destroy( );
}

void vaPostProcessBlurDX11::UpdateShaders( vaDrawContext & drawContext )
{
    //if( newShaderMacros != m_staticShaderMacros )
    //{
    //    m_staticShaderMacros = newShaderMacros;
    //    m_shadersDirty = true;
    //}

    if( m_shadersDirty )
    {
        m_shadersDirty = false;

        m_pixelShaderBlur.CreateShaderFromFile( L"vaPostProcessBlur.hlsl", "ps_5_0", "PSBlur", m_staticShaderMacros );
    }
}

void vaPostProcessBlurDX11::Blur( vaDrawContext & drawContext, vaTexture & srcRadiance )
{
    VA_SCOPE_CPUGPU_TIMER( PP_Blur, drawContext.APIContext );

    vaRenderDeviceContextDX11 * apiContext = drawContext.APIContext.SafeCast<vaRenderDeviceContextDX11*>( );
    ID3D11DeviceContext * dx11Context = apiContext->GetDXImmediateContext( );

    // Setup
    UpdateShaders( drawContext );

    // Make sure we're not overwriting anyone's stuff and set our textures
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, POSTPROCESS_TEXTURE_SLOT0 );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, srcRadiance.SafeCast<vaTextureDX11*>( )->GetSRV( ), POSTPROCESS_TEXTURE_SLOT0 );

    // Draw quad!
    apiContext->FullscreenPassDraw( dx11Context, m_pixelShaderBlur );

    // Reset/restore DX API states
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, POSTPROCESS_TEXTURE_SLOT0 );
}

void RegisterPostProcessBlurDX11( )
{
    VA_RENDERING_MODULE_REGISTER( vaPostProcessBlur, vaPostProcessBlurDX11 );
}
