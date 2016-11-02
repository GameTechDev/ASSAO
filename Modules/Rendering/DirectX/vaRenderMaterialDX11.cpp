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

#include "Rendering/DirectX/vaRenderMaterialDX11.h"

#include "Rendering/DirectX/vaTextureDX11.h"
#include "Rendering/DirectX/vaRenderingToolsDX11.h"
#include "Rendering/DirectX/vaRenderDeviceContextDX11.h"

using namespace VertexAsylum;

vaRenderMaterialDX11::vaRenderMaterialDX11( const vaConstructorParamsBase * params ) : vaRenderMaterial( params )
{
    VA_DIRECTX_NOTIFY_TARGET_CONSTRUCT_PATCH( vaRenderMaterialDX11 );
}

vaRenderMaterialDX11::~vaRenderMaterialDX11( )
{
    VA_DIRECTX_NOTIFY_TARGET_DESTRUCT_PATCH( vaRenderMaterialDX11 );
}

void vaRenderMaterialDX11::OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain )
{
    m_constantsBuffer.Create( );
}

void vaRenderMaterialDX11::OnDeviceDestroyed( )
{
    m_constantsBuffer.Destroy( );
}

void vaRenderMaterialDX11::UploadToAPIContext( vaDrawContext & drawContext )
{
    UpdateShaderMacros( );

    if( m_shadersDirty || (m_shaders == nullptr) )
    {
        m_shaders = m_renderMaterialManager.SafeCast<vaRenderMaterialManagerDX11*>()->FindOrCreateShaders( m_shaderFileName, m_settings.AlphaTest, m_shaderEntryVS_PosOnly, m_shaderEntryPS_DepthOnly, m_shaderEntryVS_Standard, m_shaderEntryPS_Forward, m_shaderEntryPS_Deferred, m_shaderMacros );

        m_shadersDirty = false;
    }

    vaRenderDeviceContextDX11 * apiContext = drawContext.APIContext.SafeCast<vaRenderDeviceContextDX11*>( );
    ID3D11DeviceContext * dx11Context = apiContext->GetDXImmediateContext( );

    if( (drawContext.PassType == vaRenderPassType::DepthPrePass) || (drawContext.PassType == vaRenderPassType::GenerateShadowmap) )
    {
        dx11Context->IASetInputLayout( m_shaders->VS_PosOnly.GetInputLayout( ) );
        dx11Context->VSSetShader( m_shaders->VS_PosOnly.GetShader( ), NULL, 0 );
        
        if( m_settings.AlphaTest )
            dx11Context->PSSetShader( m_shaders->PS_DepthOnly.GetShader(), NULL, 0 );
        else
            dx11Context->PSSetShader( NULL, NULL, 0 );
    }
    else if( (drawContext.PassType == vaRenderPassType::Deferred) || (drawContext.PassType == vaRenderPassType::ForwardOpaque) || (drawContext.PassType == vaRenderPassType::ForwardTransparent) || (drawContext.PassType == vaRenderPassType::ForwardDebugWireframe) )
    {
        dx11Context->IASetInputLayout( m_shaders->VS_Standard.GetInputLayout( ) );
        dx11Context->VSSetShader( m_shaders->VS_Standard.GetShader( ), NULL, 0 );
        if( drawContext.PassType == vaRenderPassType::Deferred ) 
            dx11Context->PSSetShader( m_shaders->PS_Deferred, NULL, 0 );
        else
            dx11Context->PSSetShader( m_shaders->PS_Forward, NULL, 0 );
    }
    else // all other
    {
        assert( false ); // not implemented!
    }

    ID3D11ShaderResourceView *      materialTextures[6] = { NULL, NULL, NULL, NULL, NULL, NULL };

    if( m_textureAlbedo != nullptr )    materialTextures[0] = m_textureAlbedo->SafeCast<vaTextureDX11*>( )->GetSRV();
    if( m_textureNormalmap != nullptr ) materialTextures[1] = m_textureNormalmap->SafeCast<vaTextureDX11*>( )->GetSRV( );
    if( m_textureSpecular!= nullptr )   materialTextures[2] = m_textureSpecular->SafeCast<vaTextureDX11*>( )->GetSRV( );

    dx11Context->PSSetShaderResources( RENDERMESH_TEXTURE_SLOT0, _countof( materialTextures ), materialTextures );
}

vaRenderMaterialManagerDX11::vaRenderMaterialManagerDX11( const vaConstructorParamsBase * params ) : vaRenderMaterialManager( )
{
    //    m_shadersDirty = true;

    VA_DIRECTX_NOTIFY_TARGET_CONSTRUCT_PATCH( vaRenderMaterialManagerDX11 );
}

vaRenderMaterialManagerDX11::~vaRenderMaterialManagerDX11( )
{
    VA_DIRECTX_NOTIFY_TARGET_DESTRUCT_PATCH( vaRenderMaterialManagerDX11 );
}

shared_ptr<vaRenderMaterialCachedShadersDX11>
vaRenderMaterialManagerDX11::FindOrCreateShaders( const wstring & fileName, bool alphaTest, const string & entryVS_PosOnly, const string & entryPS_DepthOnly, const string & entryVS_Standard, string & entryPS_Forward, const string & entryPS_Deferred, const vector< pair< string, string > > & shaderMacros )
{
    vaRenderMaterialCachedShadersDX11::Key cacheKey( fileName, alphaTest, entryVS_PosOnly, entryPS_DepthOnly, entryVS_Standard, entryPS_Forward, entryPS_Deferred, shaderMacros );

    auto it = m_cachedShaders.find( cacheKey );
    
    // in cache but no longer used by anyone so it was destroyed
    if( (it != m_cachedShaders.end()) && it->second.expired() )
    {
        m_cachedShaders.erase( it );
        it = m_cachedShaders.end();
    }

    // not in cache
    if( it == m_cachedShaders.end() )
    {
        shared_ptr<vaRenderMaterialCachedShadersDX11> newShaders( new vaRenderMaterialCachedShadersDX11 );

        std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;
        inputElements.push_back( vaVertexInputLayoutsDX11::CD3D11_INPUT_ELEMENT_DESC( "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 ) );
        inputElements.push_back( vaVertexInputLayoutsDX11::CD3D11_INPUT_ELEMENT_DESC( "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 ) );
        inputElements.push_back( vaVertexInputLayoutsDX11::CD3D11_INPUT_ELEMENT_DESC( "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 ) );
        inputElements.push_back( vaVertexInputLayoutsDX11::CD3D11_INPUT_ELEMENT_DESC( "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 ) );
        inputElements.push_back( vaVertexInputLayoutsDX11::CD3D11_INPUT_ELEMENT_DESC( "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 ) );
        inputElements.push_back( vaVertexInputLayoutsDX11::CD3D11_INPUT_ELEMENT_DESC( "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 ) );

        newShaders->VS_PosOnly.CreateShaderAndILFromFile( fileName.c_str( ), "vs_5_0", entryVS_PosOnly.c_str( ), shaderMacros, &inputElements[0], (uint32)inputElements.size( ) );
        newShaders->VS_Standard.CreateShaderAndILFromFile( fileName.c_str( ), "vs_5_0", entryVS_Standard.c_str( ), shaderMacros, &inputElements[0], (uint32)inputElements.size( ) );
        if( alphaTest )
            newShaders->PS_DepthOnly.CreateShaderFromFile( fileName.c_str( ), "ps_5_0", entryPS_DepthOnly.c_str( ), shaderMacros );
        else
            newShaders->PS_DepthOnly.Clear( );
        newShaders->PS_Forward.CreateShaderFromFile( fileName.c_str( ), "ps_5_0", entryPS_Forward.c_str( ), shaderMacros );
        newShaders->PS_Deferred.CreateShaderFromFile( fileName.c_str( ), "ps_5_0", entryPS_Deferred.c_str( ), shaderMacros );
        
        m_cachedShaders.insert( std::make_pair( cacheKey, newShaders ) );

        return newShaders;
    }
    else
    {
        return it->second.lock();
    }
}

void RegisterRenderMaterialDX11( )
{
    VA_RENDERING_MODULE_REGISTER( vaRenderMaterial, vaRenderMaterialDX11 );
    VA_RENDERING_MODULE_REGISTER( vaRenderMaterialManager, vaRenderMaterialManagerDX11 );
}
