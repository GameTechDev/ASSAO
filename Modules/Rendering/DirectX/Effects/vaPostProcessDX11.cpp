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

#include "Rendering/Effects/vaPostProcess.h"

#include "Rendering/DirectX/vaDirectXCore.h"
#include "Rendering/DirectX/vaDirectXShader.h"
#include "Rendering/DirectX/vaDirectXTools.h"
#include "Rendering/DirectX/vaRenderDeviceContextDX11.h"
#include "Rendering/DirectX/vaRenderDeviceDX11.h"
#include "Rendering/DirectX/vaTextureDX11.h"

#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP) || (_WIN32_WINNT > _WIN32_WINNT_WIN8)
#include <wincodec.h>
#endif


#include "Rendering/DirectX/Tools/DirectXTex-master/ScreenGrab/ScreenGrab.h"

namespace VertexAsylum
{

    class vaPostProcessDX11 : public vaPostProcess, VertexAsylum::vaDirectXNotifyTarget
    {
        VA_RENDERING_MODULE_MAKE_FRIENDS( );

    private:
        vaDirectXPixelShader                    m_pixelShaderCompare;
        vaDirectXPixelShader                    m_pixelShaderDrawTexturedQuad;

        vaDirectXVertexShader                   m_vertexShaderStretchRect;
        vaDirectXPixelShader                    m_pixelShaderStretchRect;

        bool                                    m_shadersDirty;

        vaDirectXConstantsBuffer< PostProcessConstants >
                                                m_constantsBuffer;

        vector< pair< string, string > >        m_staticShaderMacros;

        ID3D11Buffer *                          m_fullscreenVB;

    protected:
        vaPostProcessDX11( const vaConstructorParamsBase * params );
        ~vaPostProcessDX11( );

    private:
        // vaDirectXNotifyTarget
        virtual void                            OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain );
        virtual void                            OnDeviceDestroyed( );

    private:
        void                                    UpdateShaders( vaDrawContext & drawContext );

    private:
        void                                    FullscreenPassDraw( ID3D11DeviceContext * context, ID3D11PixelShader * pixelShader, ID3D11BlendState * blendState, ID3D11DepthStencilState * depthStencilState, UINT stencilRef, ID3D11VertexShader * vertexShader, ID3D11InputLayout * inputLayout );

    public:
        virtual void                            SaveTextureToDDSFile( vaDrawContext & drawContext, const wstring & path, vaTexture & texture );
        virtual void                            SaveTextureToPNGFile( vaDrawContext & drawContext, const wstring & path, vaTexture & texture );
        virtual void                            CopyResource( vaDrawContext & drawContext, vaTexture & destinationTexture, vaTexture & sourceTexture );
        virtual vaVector4                       CompareImages( vaDrawContext & drawContext, vaTexture & textureA, vaTexture & textureB );

        virtual void                            DrawTexturedQuad( vaDrawContext & drawContext, vaTexture & texture );
        virtual void                            StretchRect( vaDrawContext & drawContext, vaTexture & texture, const vaVector4 & srcRect, const vaVector4 & dstRect, bool linearFilter );

        //        virtual void                            DepthToViewspaceLinear( vaDrawContext & drawContext, vaTexture & depthTexture );
    };

}

using namespace VertexAsylum;

namespace 
{
    struct CommonSimpleVertex
    {
        float   Position[4];
        float   UV[2];

        CommonSimpleVertex( ) {};
        CommonSimpleVertex( float px, float py, float pz, float pw, float uvx, float uvy ) { Position[0] = px; Position[1] = py; Position[2] = pz; Position[3] = pw; UV[0] = uvx; UV[1] = uvy; }
    };
}

vaPostProcessDX11::vaPostProcessDX11( const vaConstructorParamsBase * params )
{
    m_shadersDirty = true;

    m_fullscreenVB = nullptr;

    //    m_staticShaderMacros.push_back( std::pair<std::string, std::string>( "INVALID", "INVALID" ) );

    VA_DIRECTX_NOTIFY_TARGET_CONSTRUCT_PATCH( vaPostProcessDX11 );
}

vaPostProcessDX11::~vaPostProcessDX11( )
{
    VA_DIRECTX_NOTIFY_TARGET_DESTRUCT_PATCH( vaPostProcessDX11 );
}

void vaPostProcessDX11::OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain )
{
    m_constantsBuffer.Create( );

    {
        HRESULT hr;

        CD3D11_BUFFER_DESC desc( 4 * sizeof( CommonSimpleVertex ), D3D11_BIND_VERTEX_BUFFER );

        CommonSimpleVertex fsVertices[4];
        fsVertices[0] = CommonSimpleVertex( -1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f );
        fsVertices[1] = CommonSimpleVertex(  1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 0.0f );
        fsVertices[2] = CommonSimpleVertex( -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f );
        fsVertices[3] = CommonSimpleVertex(  1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f );

        D3D11_SUBRESOURCE_DATA initSubresData;
        initSubresData.pSysMem = fsVertices;
        initSubresData.SysMemPitch = 0;
        initSubresData.SysMemSlicePitch = 0;
        hr = device->CreateBuffer( &desc, &initSubresData, &m_fullscreenVB );
    }
}

void vaPostProcessDX11::OnDeviceDestroyed( )
{
    SAFE_RELEASE( m_fullscreenVB );
    m_constantsBuffer.Destroy( );
}

void vaPostProcessDX11::UpdateShaders( vaDrawContext & drawContext )
{
    //if( newShaderMacros != m_staticShaderMacros )
    //{
    //    m_staticShaderMacros = newShaderMacros;
    //    m_shadersDirty = true;
    //}

    if( m_shadersDirty )
    {
        m_shadersDirty = false;

        m_pixelShaderCompare.CreateShaderFromFile( L"vaPostProcess.hlsl", "ps_5_0", "PSCompareTextures", m_staticShaderMacros );
        m_pixelShaderDrawTexturedQuad.CreateShaderFromFile( L"vaPostProcess.hlsl", "ps_5_0", "PSDrawTexturedQuad", m_staticShaderMacros );
        
        
       D3D11_INPUT_ELEMENT_DESC inputElements[] =
        {
            { "SV_Position", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        m_vertexShaderStretchRect.CreateShaderAndILFromFile( L"vaPostProcess.hlsl", "vs_5_0", "VSStretchRect", m_staticShaderMacros, inputElements, _countof(inputElements) );
        m_pixelShaderStretchRect.CreateShaderFromFile( L"vaPostProcess.hlsl", "ps_5_0", "PSStretchRect", m_staticShaderMacros );
    }
}

void vaPostProcessDX11::FullscreenPassDraw( ID3D11DeviceContext * context, ID3D11PixelShader * pixelShader, ID3D11BlendState * blendState, ID3D11DepthStencilState * depthStencilState, UINT stencilRef, ID3D11VertexShader * vertexShader, ID3D11InputLayout * inputLayout )
{
    assert( vertexShader != nullptr );
//    if( vertexShader == NULL )
//        vertexShader = m_fullscreenVS.GetShader();

    // Topology
    context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

    // Vertex buffer
    const u_int stride = sizeof( CommonSimpleVertex );
    UINT offsetInBytes = 0;
    context->IASetVertexBuffers( 0, 1, &m_fullscreenVB, &stride, &offsetInBytes );

    // Shaders and input layout
    
    context->IASetInputLayout( inputLayout );
    context->VSSetShader( vertexShader, NULL, 0 );

    context->PSSetShader( pixelShader, NULL, 0 );

    float blendFactor[4] = { 0, 0, 0, 0 };
    context->OMSetBlendState( blendState, blendFactor, 0xFFFFFFFF );

    context->OMSetDepthStencilState( depthStencilState, stencilRef );

    context->RSSetState( vaDirectXTools::GetRS_CullNone_Fill() );

    context->Draw( 4, 0 );
}

void vaPostProcessDX11::SaveTextureToDDSFile( vaDrawContext & drawContext, const wstring & path, vaTexture & texture )
{
    vaRenderDeviceContextDX11 * apiContext = drawContext.APIContext.SafeCast<vaRenderDeviceContextDX11*>( );
    ID3D11DeviceContext * dx11Context = apiContext->GetDXImmediateContext( );

    DirectX::SaveDDSTextureToFile( dx11Context, texture.SafeCast<vaTextureDX11*>( )->GetResource( ), path.c_str() );
    //ID3D11Texture2D * destRes = destinationTexture.SafeCast<vaTextureDX11*>( )->GetResource( );
}

void vaPostProcessDX11::SaveTextureToPNGFile( vaDrawContext & drawContext, const wstring & path, vaTexture & texture )
{
    vaRenderDeviceContextDX11 * apiContext = drawContext.APIContext.SafeCast<vaRenderDeviceContextDX11*>( );
    ID3D11DeviceContext * dx11Context = apiContext->GetDXImmediateContext( );

    DirectX::SaveWICTextureToFile( dx11Context, texture.SafeCast<vaTextureDX11*>( )->GetResource( ), GUID_ContainerFormatPng, path.c_str( ) );
}

void vaPostProcessDX11::CopyResource( vaDrawContext & drawContext, vaTexture & destinationTexture, vaTexture & sourceTexture )
{
    vaRenderDeviceContextDX11 * apiContext = drawContext.APIContext.SafeCast<vaRenderDeviceContextDX11*>( );
    ID3D11DeviceContext * dx11Context = apiContext->GetDXImmediateContext( );

    ID3D11Resource * destRes = destinationTexture.SafeCast<vaTextureDX11*>()->GetResource();
    ID3D11Resource * srcRes = sourceTexture.SafeCast<vaTextureDX11*>()->GetResource();

    dx11Context->CopyResource( destRes, srcRes );
}

void vaPostProcessDX11::DrawTexturedQuad( vaDrawContext & drawContext, vaTexture & texture )
{
    VA_SCOPE_CPUGPU_TIMER( PP_DrawTexturedQuad, drawContext.APIContext );

    vaRenderDeviceContextDX11 * apiContext = drawContext.APIContext.SafeCast<vaRenderDeviceContextDX11*>( );
    ID3D11DeviceContext * dx11Context = apiContext->GetDXImmediateContext( );

    // Setup
    UpdateShaders( drawContext );

    // Make sure we're not overwriting anyone's stuff and set our textures
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, POSTPROCESS_TEXTURE_SLOT0 );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, texture.SafeCast<vaTextureDX11*>( )->GetSRV( ), POSTPROCESS_TEXTURE_SLOT0 );

    // Draw quad!
    apiContext->FullscreenPassDraw( dx11Context, m_pixelShaderDrawTexturedQuad );

    // Reset/restore DX API states
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, POSTPROCESS_TEXTURE_SLOT0 );
}

void vaPostProcessDX11::StretchRect( vaDrawContext & drawContext, vaTexture & texture, const vaVector4 & srcRect, const vaVector4 & dstRect, bool linearFilter )
{
    VA_SCOPE_CPUGPU_TIMER( PP_DrawTexturedQuad, drawContext.APIContext );

    vaRenderDeviceContextDX11 * apiContext = drawContext.APIContext.SafeCast<vaRenderDeviceContextDX11*>( );
    ID3D11DeviceContext * dx11Context = apiContext->GetDXImmediateContext( );

    const vaViewport & viewport = drawContext.APIContext.GetViewport();

    // not yet supported / tested
    assert( viewport.X == 0 );
    assert( viewport.Y == 0 );

    vaVector2 dstPixSize = vaVector2( 1.0f / (float)viewport.Width, 1.0f / (float)viewport.Height );

    // Setup
    UpdateShaders( drawContext );

    PostProcessConstants consts;
    consts.Param1.x = dstPixSize.x * dstRect.x * 2.0f - 1.0f;
    consts.Param1.y = 1.0f - dstPixSize.y * dstRect.y * 2.0f;
    consts.Param1.z = dstPixSize.x * dstRect.z * 2.0f - 1.0f;
    consts.Param1.w = 1.0f - dstPixSize.y * dstRect.w * 2.0f;

    vaVector2 srcPixSize = vaVector2( 1.0f / (float)texture.GetSizeX(), 1.0f / (float)texture.GetSizeY() );

    consts.Param2.x = srcPixSize.x * srcRect.x;
    consts.Param2.y = srcPixSize.y * srcRect.y;
    consts.Param2.z = srcPixSize.x * srcRect.z;
    consts.Param2.w = srcPixSize.y * srcRect.w;

    //consts.Param2.x = (float)viewport.Width
    //consts.Param2 = dstRect;

    m_constantsBuffer.Update( dx11Context, consts );

    // Make sure we're not overwriting anyone's stuff and set our textures
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, POSTPROCESS_TEXTURE_SLOT0 );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, texture.SafeCast<vaTextureDX11*>( )->GetSRV( ), POSTPROCESS_TEXTURE_SLOT0 );


    {
        // make sure we're not overwriting someone's samplers
        vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, (ID3D11SamplerState*)nullptr, POSTPROCESS_SAMPLERS_SLOT0 );
        vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, (ID3D11SamplerState*)nullptr, POSTPROCESS_SAMPLERS_SLOT1 );

        // set our samplers
        ID3D11SamplerState * samplers[] =
        {
            (linearFilter)?(vaDirectXTools::GetSamplerStateLinearClamp( )):(vaDirectXTools::GetSamplerStatePointClamp( )),
            nullptr,
        };
        vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, samplers, POSTPROCESS_SAMPLERS_SLOT0, _countof( samplers ) );
    }

    // make sure we're not overwriting anything else, and set our constant buffers
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, (ID3D11Buffer*)NULL, POSTPROCESS_CONSTANTS_BUFFERSLOT );
    m_constantsBuffer.SetToD3DContextAllShaderTypes( dx11Context, POSTPROCESS_CONSTANTS_BUFFERSLOT );

    // Draw quad!
    FullscreenPassDraw( dx11Context, m_pixelShaderStretchRect, vaDirectXTools::GetBS_AlphaBlend(), vaDirectXTools::GetDSS_DepthDisabled_NoDepthWrite(), 0, m_vertexShaderStretchRect, m_vertexShaderStretchRect.GetInputLayout() );

    // Reset/restore DX API states
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, POSTPROCESS_TEXTURE_SLOT0 );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, (ID3D11Buffer*)NULL, POSTPROCESS_CONSTANTS_BUFFERSLOT );

    // Reset samplers
    {
        ID3D11SamplerState * samplers[2] = { nullptr, nullptr };
        vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, samplers, POSTPROCESS_SAMPLERS_SLOT0, _countof( samplers ) );
    }
}

vaVector4 vaPostProcessDX11::CompareImages( vaDrawContext & drawContext, vaTexture & textureA, vaTexture & textureB )
{
    VA_SCOPE_CPUGPU_TIMER( PP_CompareImages, drawContext.APIContext );

    vaRenderDeviceContextDX11 * apiContext = drawContext.APIContext.SafeCast<vaRenderDeviceContextDX11*>( );
    ID3D11DeviceContext * dx11Context = apiContext->GetDXImmediateContext( );

    assert( textureA.GetSizeX() == textureB.GetSizeX() );
    assert( textureA.GetSizeY() == textureB.GetSizeY() );

    // Setup
    UpdateShaders( drawContext );

    // DX states
    // backup render targets
    vaRenderDeviceContext::OutputsState outputsState = drawContext.APIContext.GetOutputs();
    
    int inputSizeX = textureA.GetSizeX();
    int inputSizeY = textureA.GetSizeY();

    // set output
    drawContext.APIContext.SetRenderTargetsAndUnorderedAccessViews( 0, nullptr, nullptr, 0, 1, &m_comparisonResultsGPU, false );
    drawContext.APIContext.SetViewport( vaViewport( inputSizeX, inputSizeY ) );

    // make sure we're not overwriting anyone's stuff and set our textures
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, POSTPROCESS_TEXTURE_SLOT0 );
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, POSTPROCESS_TEXTURE_SLOT1 );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, textureA.SafeCast<vaTextureDX11*>( )->GetSRV( ), POSTPROCESS_TEXTURE_SLOT0 );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, textureB.SafeCast<vaTextureDX11*>( )->GetSRV( ), POSTPROCESS_TEXTURE_SLOT1 );

    // clear results UAV
    m_comparisonResultsGPU->ClearUAV( drawContext.APIContext, vaVector4ui(0, 0, 0, 0) );

    // Call GPU comparison shader
    apiContext->FullscreenPassDraw( dx11Context, m_pixelShaderCompare );

    // GPU -> CPU copy ( SYNC POINT HERE!! but it doesn't matter because this is only supposed to be used for unit tests and etc.)
    CopyResource( drawContext, *m_comparisonResultsCPU.get(), *m_comparisonResultsGPU.get() );

    // 2.) get data from oldest (next) CPU resource
    vaTextureDX11 * readTex = m_comparisonResultsCPU->SafeCast<vaTextureDX11 *>( );
    ID3D11Texture2D * readTexDX11 = readTex->GetTexture2D( );
    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = dx11Context->Map( readTexDX11, 0, D3D11_MAP_READ, 0, &mapped );
    uint32 data[POSTPROCESS_COMPARISONRESULTS_SIZE];
    if( !FAILED( hr ) )
    {
        memcpy( data, mapped.pData, sizeof( data ) );
        dx11Context->Unmap( readTexDX11, 0 );
    }
    else
    {
        assert( false );
    }

    // Reset/restore DX API states
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, POSTPROCESS_TEXTURE_SLOT0 );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, POSTPROCESS_TEXTURE_SLOT1 );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, (ID3D11Buffer*)NULL, POSTPROCESS_CONSTANTS_BUFFERSLOT );
    drawContext.APIContext.SetOutputs( outputsState );

    // calculate results
    vaVector4 ret( 0.0f, 0.0f, 0.0f, 0.0f );

    int totalPixelCount = inputSizeX * inputSizeY;
    int64 resultsSum = 0;

    //double resultsSumD = 0.0;
    //double resultsSumCount = 0.0;

    for( size_t i = 0; i < _countof( data ); i++ )
    {
//        VA_LOG( " %d %x", i, data[i] );

        resultsSum += data[i];
    }

    double MSEVal = ((double)resultsSum / 8191.0 / (double)totalPixelCount);

    // only one metric output so far, MSE!
    ret.x = (float)(MSEVal);
    ret.y = 10.0f * (float)log10( 1.0 / MSEVal );       // PSNR - we assume 1 is the max value
    ret.z = (float)(MSEVal * 10000.0);  // just to make it more human readable

    return ret;
}

/*
void vaPostProcessDX11::DepthToViewspaceLinear( vaDrawContext & drawContext, vaTexture & depthTexture )
{
    ID3D11DeviceContext * dx11Context = vaSaferStaticCast< vaRenderDeviceContextDX11 * >( &drawContext.APIContext )->GetDXImmediateContext( );

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
//    if( m_shadersDirty )
//    {
//        m_shadersDirty = false;
//
//        std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements = vaVertexInputLayoutsDX11::PositionColorNormalTangentTexcoord1VertexDecl( );
//        m_vertexShader.CreateShaderAndILFromFile(               GetShaderFilePath( ), "vs_5_0", GetVertexShaderEntry(), m_staticShaderMacros, &inputElements[0], (uint32)inputElements.size( ) );
//        m_pixelShader.CreateShaderFromFile(                     GetShaderFilePath( ), "ps_5_0", GetPixelShaderEntry(),  m_staticShaderMacros );
//    }

//     // make sure we're not overwriting anything else, and set our constant buffers
//     vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, (ID3D11Buffer*)NULL, RENDERMESH_CONSTANTS_BUFFERSLOT );
//     m_constantsBuffer.SetToD3DContextAllShaderTypes( dx11Context, RENDERMESH_CONSTANTS_BUFFERSLOT );

    // Reset
    ID3D11ShaderResourceView * nullTextures[4] = { NULL, NULL, NULL, NULL };
    dx11Context->VSSetShader( NULL, NULL, 0 );
    dx11Context->VSSetShaderResources( 0, _countof(nullTextures), nullTextures );
    dx11Context->PSSetShader( NULL, NULL, 0 );
    dx11Context->PSSetShaderResources( 0, _countof(nullTextures), nullTextures );

    // make sure nothing messed with our constant buffers and nothing uses them after
    // vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, m_constantsBuffer.GetBuffer( ), RENDERMESH_CONSTANTS_BUFFERSLOT );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, (ID3D11Buffer*)NULL, RENDERMESH_CONSTANTS_BUFFERSLOT );
}
*/

void RegisterPostProcessDX11( )
{
    VA_RENDERING_MODULE_REGISTER( vaPostProcess, vaPostProcessDX11 );
}
