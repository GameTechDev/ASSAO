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

#include "Rendering/Effects/vaASSAO.h"

#include "Rendering/DirectX/vaRenderDeviceDX11.h"
#include "Rendering/DirectX/vaRenderDeviceContextDX11.h"
#include "Rendering/DirectX/vaRenderingToolsDX11.h"
#include "Rendering/DirectX/vaTextureDX11.h"

#ifdef SSAO_ALLOW_INTERNAL_SHADER_DEBUGGING
#include "Rendering/DirectX/vaRenderingGlobalsDX11.h"
#endif

namespace VertexAsylum
{

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

    class vaASSAODX11 : public vaASSAO, VertexAsylum::vaDirectXNotifyTarget
    {
        VA_RENDERING_MODULE_MAKE_FRIENDS( );

    private:
        ID3D11Buffer *                          m_fullscreenVB;

        vaDirectXVertexShader                   m_vertexShader;
        
        vaDirectXPixelShader                    m_pixelShaderPrepareDepths;
        vaDirectXPixelShader                    m_pixelShaderPrepareDepthsAndNormals;
        vaDirectXPixelShader                    m_pixelShaderPrepareDepthsHalf;
        vaDirectXPixelShader                    m_pixelShaderPrepareDepthsAndNormalsHalf;
        vaDirectXPixelShader                    m_pixelShaderPrepareDepthMip[SSAO_DEPTH_MIP_LEVELS-1];
        vaDirectXPixelShader                    m_pixelShaderGenerate[6];                               // number 6 is "adaptive base"
        vaDirectXPixelShader                    m_pixelShaderSmartBlur;
        vaDirectXPixelShader                    m_pixelShaderSmartBlurWide;
        vaDirectXPixelShader                    m_pixelShaderApply;
        vaDirectXPixelShader                    m_pixelShaderNonSmartBlur;
        vaDirectXPixelShader                    m_pixelShaderNonSmartApply;
        vaDirectXPixelShader                    m_pixelShaderNonSmartHalfApply;
        vaDirectXPixelShader                    m_pixelShaderGenerateImportanceMap;
        vaDirectXPixelShader                    m_pixelShaderPostprocessImportanceMapA;
        vaDirectXPixelShader                    m_pixelShaderPostprocessImportanceMapB;

        vaDirectXPixelShader                    m_pixelShaderAlternativeApplySmartBlur[4];
        vaDirectXPixelShader                    m_pixelShaderAlternativeApplyInterleave;

        //vaDirectXPixelShader                    m_pixelShaderGenerateAndApplyReferenceFullscreen;

#ifdef SSAO_ALLOW_INTERNAL_SHADER_DEBUGGING
        vaDirectXPixelShader                    m_pixelShaderGenerateDebug[6];
#endif

        bool                                    m_shadersDirty;

        vaDirectXConstantsBuffer< ASSAOConstants >
                                                m_constantsBuffer;

        vector< pair< string, string > >        m_staticShaderMacros;

        wstring                                 m_shaderFileToUse;

        int                                     m_depthMipLevels;

        ID3D11SamplerState *                    m_samplerStateViewspaceDepthTap;
        ID3D11RasterizerState *                 m_fullscreenRasterizerState;

        bool                                    m_requiresClear;

    public:
        vaASSAODX11(  const vaConstructorParamsBase * params );
        ~vaASSAODX11( );

    private:
        // vaDirectXNotifyTarget
        virtual void                            OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain );
        virtual void                            OnDeviceDestroyed( );

    private:
        const wchar_t *                         GetShaderFilePath( ) const { return m_shaderFileToUse.c_str( ); };
        //
        void                                    UpdateTextures( vaDrawContext & drawContext, int width, int height, bool generateNormals, const vaVector4i & scissorRect );
        void                                    UpdateConstants( vaDrawContext & drawContext, const vaMatrix4x4 & projMatrix, int pass );
        void                                    FullscreenPassDraw( ID3D11DeviceContext * context, ID3D11PixelShader * pixelShader, ID3D11BlendState * blendState = vaDirectXTools::GetBS_Opaque(), ID3D11DepthStencilState * depthStencilState = vaDirectXTools::GetDSS_DepthDisabled_NoDepthWrite(), UINT stencilRef = 0 );
        //
        void                                    UpdateSamplingPattern( vaVector4 destArr[], const vaVector2 * srcArr, int arrCount );
        void                                    UpdateSamplingPattern( vaVector4 destArr[], const vaVector4 * srcArr, int arrCount );
        //
        void                                    PrepareDepths( vaDrawContext & drawContext, vaTexture & depthTexture, bool generateNormals );
        void                                    GenerateSSAO( vaDrawContext & drawContext, const vaMatrix4x4 & projMatrix, vaTexture & normalmapTexture, bool adaptiveBasePass );
        //
        void                                    Draw( vaDrawContext & drawContext, const vaMatrix4x4 & projMatrix, vaTexture & depthTexture, vaTexture & normalmapTexture, bool blend, bool generateNormals, const vaVector4i & scissorRect );
        //
    private:
        // vaASSAO
        virtual void                            Draw( vaDrawContext & drawContext, const vaMatrix4x4 & projMatrix, vaTexture & depthTexture, bool blend, vaTexture * normalmapTexture = nullptr, const vaVector4i & scissorRect = vaVector4i( 0, 0, 0, 0 ) );
    public:
    };

}

using namespace VertexAsylum;

static const vaVector2 *    GetRefSamples( );
static int                  GetRefSampleCount( );

static const int cMaxBlurPassCount = 6;

vaASSAODX11::vaASSAODX11( const vaConstructorParamsBase * params )
{
    m_fullscreenVB = nullptr;

    m_shadersDirty = true;

    m_shaderFileToUse = L"vaASSAO.hlsl";

    m_depthMipLevels = 0;

    m_samplerStateViewspaceDepthTap = nullptr;
    m_fullscreenRasterizerState     = nullptr;

    m_autoPatternGenerateModeEnabled        = false;
    m_autoPatternGenerateModeCurrentCount   = 0;
    //    m_staticShaderMacros.push_back( std::pair<std::string, std::string>( "INVALID", "INVALID" ) );

    m_requiresClear = false;

    VA_DIRECTX_NOTIFY_TARGET_CONSTRUCT_PATCH( vaASSAODX11 );
}

vaASSAODX11::~vaASSAODX11( )
{
    VA_DIRECTX_NOTIFY_TARGET_DESTRUCT_PATCH( vaASSAODX11 );
}

void vaASSAODX11::OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain )
{
    m_constantsBuffer.Create( );

    HRESULT hr;

    CD3D11_BUFFER_DESC desc( 3 * sizeof( CommonSimpleVertex ), D3D11_BIND_VERTEX_BUFFER );

    // <workaround for a bug - the bug possibly doesn't exist anymore>
    // used to make Gather using UV slightly off the border (so we get the 0,0 1,0 0,1 1,1 even if there's a minor calc error, without adding the half pixel offset)
    static const float  c_minorUVOffset = 0.0f; //0.00006f;  // less than 0.5/8192

    // using one big triangle
    CommonSimpleVertex fsVertices[3];
    fsVertices[0] = CommonSimpleVertex( -1.0f, 1.0f, 0.0f, 1.0f, 0.0f + c_minorUVOffset, 0.0f + c_minorUVOffset );
    fsVertices[1] = CommonSimpleVertex( 3.0f, 1.0f, 0.0f, 1.0f, 2.0f + c_minorUVOffset, 0.0f + c_minorUVOffset );
    fsVertices[2] = CommonSimpleVertex( -1.0f, -3.0f, 0.0f, 1.0f, 0.0f + c_minorUVOffset, 2.0f + c_minorUVOffset );

    D3D11_SUBRESOURCE_DATA initSubresData;
    initSubresData.pSysMem = fsVertices;
    initSubresData.SysMemPitch = 0;
    initSubresData.SysMemSlicePitch = 0;
    hr = device->CreateBuffer( &desc, &initSubresData, &m_fullscreenVB );

    assert( SUCCEEDED( hr ) );

    // samplers
    {
        CD3D11_SAMPLER_DESC desc = CD3D11_SAMPLER_DESC( CD3D11_DEFAULT( ) );

        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        device->CreateSamplerState( &desc, &m_samplerStateViewspaceDepthTap );
    }

    // Create the rasterizer state
    {
        CD3D11_RASTERIZER_DESC desc = CD3D11_RASTERIZER_DESC( CD3D11_DEFAULT( ) );

        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_NONE;
        desc.ScissorEnable = true;
        device->CreateRasterizerState( &desc, &m_fullscreenRasterizerState );
    }

}

void vaASSAODX11::OnDeviceDestroyed( )
{
    SAFE_RELEASE( m_fullscreenVB );
    SAFE_RELEASE( m_samplerStateViewspaceDepthTap );
    SAFE_RELEASE( m_fullscreenRasterizerState );
    m_constantsBuffer.Destroy( );
}

static bool ReCreateIfNeeded( shared_ptr<vaTexture> & inoutTex, vaVector2i size, vaTextureFormat format, float & inoutTotalSizeSum, int mipLevels, int arraySize, bool supportUAVs )
{
    int approxSize = size.x * size.y * vaTextureFormatHelpers::GetPixelSizeInBytes( format );
    if( mipLevels != 1 ) approxSize = approxSize * 2; // is this an overestimate?
    inoutTotalSizeSum += approxSize;

    vaTextureBindSupportFlags bindFlags = vaTextureBindSupportFlags::RenderTarget | vaTextureBindSupportFlags::ShaderResource;

    if( supportUAVs )
        bindFlags |= vaTextureBindSupportFlags::UnorderedAccess;

    if( ( size.x == 0 ) || ( size.y == 0 ) || ( format == vaTextureFormat::Unknown ) )
    {
        inoutTex = nullptr;
    }
    else
    {
        vaTextureFormat resourceFormat = format;
        vaTextureFormat srvFormat = format;
        vaTextureFormat rtvFormat = format;
        vaTextureFormat dsvFormat = vaTextureFormat::Unknown;
        vaTextureFormat uavFormat = (supportUAVs)?(format):(vaTextureFormat::Unknown);

        // handle special cases
        if( format == vaTextureFormat::D32_FLOAT )
        {
            bindFlags = ( bindFlags & ~( vaTextureBindSupportFlags::RenderTarget ) ) | vaTextureBindSupportFlags::DepthStencil;
            resourceFormat = vaTextureFormat::R32_TYPELESS;
            srvFormat = vaTextureFormat::R32_FLOAT;
            rtvFormat = vaTextureFormat::Unknown;
            dsvFormat = vaTextureFormat::D32_FLOAT;
        }
        // handle special cases
        if( arraySize != 1 )
        {
            // render target for arrays not really needed (it will get created anyway using resource format - no mechanism to prevent it at the moment)
            rtvFormat = vaTextureFormat::Unknown;
        }
        if( format == vaTextureFormat::R8G8B8A8_UNORM_SRGB )
        {
            resourceFormat = vaTextureFormat::R8G8B8A8_TYPELESS;
            srvFormat = vaTextureFormat::R8G8B8A8_UNORM_SRGB;
            rtvFormat = vaTextureFormat::R8G8B8A8_UNORM_SRGB;
        }

        if( (inoutTex != nullptr) && (inoutTex->GetSizeX() == size.x) && (inoutTex->GetSizeY()==size.y) &&
            (inoutTex->GetResourceFormat()==resourceFormat) && (inoutTex->GetSRVFormat()==srvFormat) && (inoutTex->GetRTVFormat()==rtvFormat) && (inoutTex->GetDSVFormat()==dsvFormat) && (inoutTex->GetUAVFormat()==uavFormat) )
            return false;

        inoutTex = shared_ptr<vaTexture>( vaTexture::Create2D( resourceFormat, size.x, size.y, mipLevels, arraySize, 1, bindFlags, vaTextureAccessFlags::None, nullptr, 0, srvFormat, rtvFormat, dsvFormat, uavFormat ) );
    }

    return true;
}

void vaASSAODX11::UpdateTextures( vaDrawContext & drawContext, int width, int height, bool generateNormals, const vaVector4i & scissorRect )
{
    vector< pair< string, string > > newShaderMacros;
    
    if( m_debugShowNormals )
        newShaderMacros.push_back( std::pair<std::string, std::string>( "SSAO_DEBUG_SHOWNORMALS", "" ) );
    if( m_debugShowEdges )
        newShaderMacros.push_back( std::pair<std::string, std::string>( "SSAO_DEBUG_SHOWEDGES", "" ) );
    if( m_debugShowSampleHeatmap )
        newShaderMacros.push_back( std::pair<std::string, std::string>( "SSAO_DEBUG_SHOWSAMPLEHEATMAP", "" ) );

    if( m_specialShaderMacro != pair<string, string>(std::make_pair( "", "" )) )
        newShaderMacros.push_back( m_specialShaderMacro );

    if( newShaderMacros != m_staticShaderMacros )
    {
        m_staticShaderMacros = newShaderMacros;
        m_shadersDirty = true;
    }

    if( m_shadersDirty )
    {
        m_shadersDirty = false;
        
        std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;
        inputElements.push_back( { "SV_Position", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 } );
        inputElements.push_back( { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 } );
        m_vertexShader.CreateShaderAndILFromFile(   GetShaderFilePath( ), "vs_5_0", "VSMain", m_staticShaderMacros, &inputElements[0], (uint32)inputElements.size( ) );

//#ifdef VA_HOLD_SHADER_DISASM
//        m_pixelShaderPrepareDepths.SetShaderDisasmAutoDumpToFile( true );
//        m_pixelShaderPrepareDepthsAndNormals.SetShaderDisasmAutoDumpToFile( true );
//        //m_pixelShaderGenerate[0].SetShaderDisasmAutoDumpToFile( true );
//        //m_pixelShaderGenerate[1].SetShaderDisasmAutoDumpToFile( true );
//        //m_pixelShaderGenerate[2].SetShaderDisasmAutoDumpToFile( true );
//        //m_pixelShaderApply.SetShaderDisasmAutoDumpToFile( true );
//        m_pixelShaderSmartBlur.SetShaderDisasmAutoDumpToFile( true );
//        m_pixelShaderSmartBlurWide.SetShaderDisasmAutoDumpToFile( true );
//        m_pixelShaderNonSmartBlur.SetShaderDisasmAutoDumpToFile( true );
//#endif

        m_pixelShaderGenerateImportanceMap.CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSGenerateImportanceMap", m_staticShaderMacros );
        m_pixelShaderPostprocessImportanceMapA.CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSPostprocessImportanceMapA", m_staticShaderMacros );
        m_pixelShaderPostprocessImportanceMapB.CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSPostprocessImportanceMapB", m_staticShaderMacros );

        m_pixelShaderPrepareDepths.CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSPrepareDepths", m_staticShaderMacros );
        m_pixelShaderPrepareDepthsAndNormals.CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSPrepareDepthsAndNormals", m_staticShaderMacros );
        m_pixelShaderPrepareDepthsHalf.CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSPrepareDepthsHalf", m_staticShaderMacros );
        m_pixelShaderPrepareDepthsAndNormalsHalf.CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSPrepareDepthsAndNormalsHalf", m_staticShaderMacros );
        m_pixelShaderPrepareDepthMip[0].CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSPrepareDepthMip1", m_staticShaderMacros );
        m_pixelShaderPrepareDepthMip[1].CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSPrepareDepthMip2", m_staticShaderMacros );
        m_pixelShaderPrepareDepthMip[2].CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSPrepareDepthMip3", m_staticShaderMacros );

        // vector< pair< string, string > > shaderMacrosSmoothNormals = m_staticShaderMacros;
        // shaderMacrosSmoothNormals.push_back( std::pair<std::string, std::string>( "SSAO_SMOOTHEN_NORMALS", "" ) );

        m_pixelShaderGenerate[0].CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSGenerateQ0", m_staticShaderMacros );
        m_pixelShaderGenerate[1].CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSGenerateQ1", m_staticShaderMacros );
        m_pixelShaderGenerate[2].CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSGenerateQ2", m_staticShaderMacros );
        m_pixelShaderGenerate[3].CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSGenerateQ3", m_staticShaderMacros );
        m_pixelShaderGenerate[4].CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSGenerateQ4", m_staticShaderMacros );
        m_pixelShaderGenerate[5].CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSGenerateQ3Base", m_staticShaderMacros );

        //m_pixelShaderGenerateAndApplyReferenceFullscreen.CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSGenerateAndApplyReferenceFullscreen", m_staticShaderMacros );

#ifdef SSAO_ALLOW_INTERNAL_SHADER_DEBUGGING
        vector< pair< string, string > > debugMacros = m_staticShaderMacros;
        debugMacros.push_back( std::pair<std::string, std::string>( "SSAO_INTERNAL_SHADER_DEBUG", "" ) );
        m_pixelShaderGenerateDebug[0].CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSGenerateQ0", debugMacros );
        m_pixelShaderGenerateDebug[1].CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSGenerateQ1", debugMacros );
        m_pixelShaderGenerateDebug[2].CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSGenerateQ2", debugMacros );
        m_pixelShaderGenerateDebug[3].CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSGenerateQ3", debugMacros );
        m_pixelShaderGenerateDebug[4].CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSGenerateQ4", debugMacros );
        m_pixelShaderGenerateDebug[5].CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSGenerateQ3Base", debugMacros );
#endif 

        m_pixelShaderSmartBlur.CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSSmartBlur", m_staticShaderMacros );
        m_pixelShaderSmartBlurWide.CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSSmartBlurWide", m_staticShaderMacros );
        m_pixelShaderNonSmartBlur.CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSNonSmartBlur", m_staticShaderMacros );

        m_pixelShaderNonSmartApply.CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSNonSmartApply", m_staticShaderMacros );
        m_pixelShaderNonSmartHalfApply.CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSNonSmartHalfApply", m_staticShaderMacros );
        m_pixelShaderApply.CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSApply", m_staticShaderMacros );
        //m_pixelShaderApplyHalfWidth.CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSApplyHalfWidth", m_staticShaderMacros );

        m_pixelShaderAlternativeApplySmartBlur[0].CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSAlternativeApplySmartBlur0", m_staticShaderMacros );
        m_pixelShaderAlternativeApplySmartBlur[1].CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSAlternativeApplySmartBlur1", m_staticShaderMacros );
        m_pixelShaderAlternativeApplySmartBlur[2].CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSAlternativeApplySmartBlur2", m_staticShaderMacros );
        m_pixelShaderAlternativeApplySmartBlur[3].CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSAlternativeApplySmartBlur3", m_staticShaderMacros );
        m_pixelShaderAlternativeApplyInterleave.CreateShaderFromFile( GetShaderFilePath( ), "ps_5_0", "PSAlternativeApplyInterleave", m_staticShaderMacros );
    }

    bool needsUpdate = false;

    if( !generateNormals )
    {
        needsUpdate |= m_normals != nullptr;
        m_normals = nullptr;
    }
    else
    {
        needsUpdate |= m_normals == nullptr;
    }

    if( width == -1 )   width = drawContext.APIContext.GetViewport( ).Width;
    if( height == -1 )  height = drawContext.APIContext.GetViewport( ).Height;

    needsUpdate |= (m_size.x != width) || (m_size.y != height);

    m_size.x        = width;
    m_size.y        = height;
    m_halfSize.x    = (width+1)/2;
    m_halfSize.y    = (height+1)/2;
    m_quarterSize.x = (m_halfSize.x+1)/2;
    m_quarterSize.y = (m_halfSize.y+1)/2;

    vaVector4i prevScissorRect = m_fullResOutScissorRect;

    if( (scissorRect.z == 0) || (scissorRect.w == 0) )
        m_fullResOutScissorRect = vaVector4i( 0, 0, width, height );
    else
        m_fullResOutScissorRect = vaVector4i( vaMath::Max( 0, scissorRect.x ), vaMath::Max( 0, scissorRect.y ), vaMath::Min( width, scissorRect.z ), vaMath::Min( height, scissorRect.w ) );

    needsUpdate |= prevScissorRect != m_fullResOutScissorRect;

#if SSAO_USE_SEPARATE_LOWQ_AORESULTS_TEXTURE
    vaTextureFormat AOResultsFormat = ( m_settings.QualityLevel == 0 ) ? m_formats.AOResultLowQ : m_formats.AOResult;

    if( (m_pingPongHalfResultA != nullptr) && (m_pingPongHalfResultA->GetSRVFormat() != AOResultsFormat ) )
        needsUpdate = true;
#else
    vaTextureFormat AOResultsFormat = m_formats.AOResult;
#endif
    if( !needsUpdate )
        return;
    
    m_halfResOutScissorRect = vaVector4i( m_fullResOutScissorRect.x/2, m_fullResOutScissorRect.y/2, (m_fullResOutScissorRect.z+1) / 2, (m_fullResOutScissorRect.w+1) / 2 );
    int blurEnlarge = cMaxBlurPassCount + max( 0, cMaxBlurPassCount-2 );  // +1 for max normal blurs, +2 for wide blurs
    m_halfResOutScissorRect = vaVector4i( vaMath::Max( 0, m_halfResOutScissorRect.x - blurEnlarge ), vaMath::Max( 0, m_halfResOutScissorRect.y - blurEnlarge ), vaMath::Min( m_halfSize.x, m_halfResOutScissorRect.z + blurEnlarge ), vaMath::Min( m_halfSize.y, m_halfResOutScissorRect.w + blurEnlarge ) );

    float totalSizeInMB = 0.0f;

    m_depthMipLevels = SSAO_DEPTH_MIP_LEVELS;

    for( int i = 0; i < 4; i++ )
    {
        if( ReCreateIfNeeded( m_halfDepths[i], m_halfSize, m_formats.DepthBufferViewspaceLinear, totalSizeInMB, m_depthMipLevels, 1, false ) )
        {
            for( int j = 0; j < m_depthMipLevels; j++ )
                m_halfDepthsMipViews[i][j] = nullptr;

            for( int j = 0; j < m_depthMipLevels; j++ )
                m_halfDepthsMipViews[i][j] = shared_ptr<vaTexture>( vaTexture::CreateView( m_halfDepths[i], m_halfDepths[i]->GetBindSupportFlags(), vaTextureFormat::Unknown, m_halfDepths[i]->GetRTVFormat(), vaTextureFormat::Unknown, vaTextureFormat::Unknown, j ) );
        }
        
    }
    ReCreateIfNeeded( m_pingPongHalfResultA, m_halfSize, AOResultsFormat, totalSizeInMB, 1, 1, false );
    ReCreateIfNeeded( m_pingPongHalfResultB, m_halfSize, AOResultsFormat, totalSizeInMB, 1, 1, false );
    ReCreateIfNeeded( m_finalResults, m_halfSize, AOResultsFormat, totalSizeInMB, 1, 4, false );
#ifdef SSAO_ENABLE_ALTERNATIVE_APPLY
    ReCreateIfNeeded( m_AAFinalResults, m_halfSize, m_formats.AAFinalAOResult, totalSizeInMB, 1, 4, false );
#endif

    ReCreateIfNeeded( m_importanceMap, m_quarterSize, m_formats.ImportanceMap, totalSizeInMB, 1, 1, false );
    ReCreateIfNeeded( m_importanceMapPong, m_quarterSize, m_formats.ImportanceMap, totalSizeInMB, 1, 1, false );
    for( int i = 0; i < 4; i++ )
    {
        m_finalResultsArrayViews[i] = shared_ptr<vaTexture>( vaTexture::CreateView( m_finalResults, vaTextureBindSupportFlags::RenderTarget, vaTextureFormat::Unknown, AOResultsFormat, vaTextureFormat::Unknown, vaTextureFormat::Unknown, 0, i ) );
#ifdef SSAO_ENABLE_ALTERNATIVE_APPLY
        m_AAFinalResultsArrayViews[i] = shared_ptr<vaTexture>( vaTexture::CreateView( m_AAFinalResults, vaTextureBindSupportFlags::RenderTarget, vaTextureFormat::Unknown, m_formats.AAFinalAOResult, vaTextureFormat::Unknown, vaTextureFormat::Unknown, 0, i ) );
#endif
    }
    
    if( generateNormals )
        ReCreateIfNeeded( m_normals, m_size, m_formats.Normals, totalSizeInMB, 1, 1, true );

    float dummy = 0.0f;
    ReCreateIfNeeded( m_debuggingOutput, m_size, vaTextureFormat::R8G8B8A8_UNORM, dummy, 1, 1, true );

    if( m_loadCounter == nullptr )
    {
        m_loadCounter = shared_ptr<vaTexture>( vaTexture::Create1D( vaTextureFormat::R32_UINT, 1, 1, 1, vaTextureBindSupportFlags::UnorderedAccess | vaTextureBindSupportFlags::ShaderResource, vaTextureAccessFlags::None, nullptr ) );
    }

    totalSizeInMB /= 1024 * 1024;

    m_debugInfo = vaStringTools::Format( "Approx. %.2fMB video memory used.", totalSizeInMB );

    // trigger a full buffers clear first time; only really required when using scissor rects
    m_requiresClear = true;
}

void vaASSAODX11::UpdateSamplingPattern( vaVector4 arr[], const vaVector2 * srcArr, int arrCount )
{
    assert( arrCount <= SSAO_MAX_REF_TAPS );
    const vaVector2 * sourcePattern = srcArr;

    for( int i = 0; i < arrCount; i++ )
    {
        vaVector2 pt = sourcePattern[ i ];

        float len       = pt.Length();
        float log2Len   = log2f( len );

        arr[i] = vaVector4( pt.x, pt.y, 1.0f, log2Len );
    }
}

void vaASSAODX11::UpdateSamplingPattern( vaVector4 arr[], const vaVector4 * sourcePattern, int arrCount )
{
    assert( arrCount <= SSAO_MAX_REF_TAPS );

    for( int i = 0; i < arrCount; i++ )
        arr[i] = sourcePattern[i];
}

void vaASSAODX11::UpdateConstants( vaDrawContext & drawContext, const vaMatrix4x4 & projMatrix, int pass )
{
    vaRenderDeviceContextDX11 * apiContext = drawContext.APIContext.SafeCast<vaRenderDeviceContextDX11*>( );
    ID3D11DeviceContext * dx11Context = apiContext->GetDXImmediateContext( );

    // update constants
    {
        ASSAOConstants consts;

        const vaMatrix4x4 & proj = projMatrix;

        consts.ViewportPixelSize                    = vaVector2( 1.0f / (float)m_size.x, 1.0f / (float)m_size.y );
        consts.HalfViewportPixelSize                = vaVector2( 1.0f / (float)m_halfSize.x, 1.0f / (float)m_halfSize.y );

        consts.Viewport2xPixelSize                  = vaVector2( consts.ViewportPixelSize.x * 2.0f, consts.ViewportPixelSize.y * 2.0f );
        consts.Viewport2xPixelSize_x_025            = consts.Viewport2xPixelSize * 0.25;

        float depthLinearizeMul                          = -proj.m[3][2];         // float depthLinearizeMul = ( clipFar * clipNear ) / ( clipFar - clipNear );
        float depthLinearizeAdd                          = proj.m[2][2];          // float depthLinearizeAdd = clipFar / ( clipFar - clipNear );
        // correct the handedness issue. need to make sure this below is correct, but I think it is.
        if( depthLinearizeMul * depthLinearizeAdd < 0 )
            depthLinearizeAdd = -depthLinearizeAdd;
        consts.DepthUnpackConsts                    = vaVector2( depthLinearizeMul, depthLinearizeAdd );

        float tanHalfFOVY                           = 1.0f / proj.m[1][1];    // = tanf( drawContext.Camera.GetYFOV( ) * 0.5f );
        float tanHalfFOVX                           = 1.0F / proj.m[0][0];    // = tanHalfFOVY * drawContext.Camera.GetAspect( );
        consts.CameraTanHalfFOV                     = vaVector2( tanHalfFOVX, tanHalfFOVY );

        consts.NDCToViewMul                         = vaVector2::ComponentMul( consts.CameraTanHalfFOV, vaVector2( 2.0, -2.0 ) );
        consts.NDCToViewAdd                         = vaVector2::ComponentMul( consts.CameraTanHalfFOV, vaVector2( -1.0, 1.0 ) );

        consts.EffectRadius                         = vaMath::Clamp( m_settings.Radius, 0.0f, 100000.0f );
        consts.EffectShadowStrength                 = vaMath::Clamp( m_settings.ShadowMultiplier * 4.3f, 0.0f, 10.0f );
        consts.EffectShadowPow                      = vaMath::Clamp( m_settings.ShadowPower, 0.0f, 5.0f );
        consts.EffectShadowClamp                    = vaMath::Clamp( m_settings.ShadowClamp, 0.0f, 1.0f );
        consts.EffectFadeOutMul                     = - 1.0f / (m_settings.FadeOutTo - m_settings.FadeOutFrom);
        consts.EffectFadeOutAdd                     = m_settings.FadeOutFrom / (m_settings.FadeOutTo - m_settings.FadeOutFrom) + 1.0f;
        consts.EffectHorizonAngleThreshold          = vaMath::Clamp( m_settings.HorizonAngleThreshold, 0.0f, 1.0f );

        // 1.2 seems to be around the best trade off - 1.0 means on-screen radius will stop/slow growing when the camera is at 1.0 distance, so, depending on FOV, basically filling up most of the screen
        // This setting is viewspace-dependent and not screen size dependent intentionally, so that when you change FOV the effect stays (relatively) similar.
        float effectSamplingRadiusNearLimit        = ( m_settings.Radius * 1.2f );

        // if the depth precision is switched to 32bit float, this can be set to something closer to 1 (0.9999 is fine)
        consts.DepthPrecisionOffsetMod              = 0.9992f;

        //consts.RadiusDistanceScalingFunctionPow     = 1.0f - vaMath::Clamp( m_settings.RadiusDistanceScalingFunction, 0.0f, 1.0f );

        int lastHalfDepthMipX = m_halfDepthsMipViews[0][SSAO_DEPTH_MIP_LEVELS-1]->GetViewedSliceSizeX();
        int lastHalfDepthMipY = m_halfDepthsMipViews[0][SSAO_DEPTH_MIP_LEVELS-1]->GetViewedSliceSizeY();

        // used to get average load per pixel; 9.0 is there to compensate for only doing every 9th InterlockedAdd in PSPostprocessImportanceMapB for performance reasons
        consts.LoadCounterAvgDiv                    = 9.0f / (float)( m_quarterSize.x * m_quarterSize.y * 255.0 );

        // Special settings for lowest quality level - just nerf the effect a tiny bit
        if( m_settings.QualityLevel == 0 )
        {
            //consts.EffectShadowStrength         *= 0.9f;
            effectSamplingRadiusNearLimit       *= 1.50f;

            if( m_settings.SkipHalfPixelsOnLowQualityLevel )
                consts.EffectRadius             *= 0.8f;
        }
        effectSamplingRadiusNearLimit /= tanHalfFOVY; // to keep the effect same regardless of FOV

        consts.EffectSamplingRadiusNearLimitRec         = 1.0f / effectSamplingRadiusNearLimit;

        consts.AdaptiveSampleCountLimit                 = m_settings.AdaptiveQualityLimit; 

        consts.NegRecEffectRadius                       = -1.0f / consts.EffectRadius;

        consts.PerPassFullResCoordOffset                = vaVector2i( pass % 2, pass / 2 );
        consts.PerPassFullResUVOffset                   = vaVector2( ((pass % 2) - 0.0f) / m_size.x, ((pass / 2) - 0.0f) / m_size.y );

#ifdef SSAO_ALLOW_INTERNAL_SHADER_DEBUGGING
        consts.DebugCursorPos                           = m_debugShowSamplesAtCursorPos;
        consts.FullResOffset                            = vaVector2i( pass % 2, pass / 2 );
#endif

        consts.InvSharpness                             = vaMath::Clamp( 1.0f - m_settings.Sharpness, 0.0f, 1.0f );
        consts.PassIndex                                = pass;
        consts.QuarterResPixelSize                      = vaVector2( 1.0f / (float)m_quarterSize.x, 1.0f / (float)m_quarterSize.y );

        float additionalAngleOffset = m_settings.TemporalSupersamplingAngleOffset;  // if using temporal supersampling approach (like "Progressive Rendering Using Multi-frame Sampling" from GPU Pro 7, etc.)
        float additionalRadiusScale = m_settings.TemporalSupersamplingRadiusOffset; // if using temporal supersampling approach (like "Progressive Rendering Using Multi-frame Sampling" from GPU Pro 7, etc.)
        const int subPassCount = 5;
        for( int subPass = 0; subPass < subPassCount; subPass++ )
        {
            int a = pass;
            int b = subPass;

            int spmap[5] { 0, 1, 4, 3, 2 };
            b = spmap[subPass];

            float ca, sa;
            float angle0 = ( (float)a + (float)b / (float)subPassCount ) * VA_PIf * 0.5f;
            angle0 += additionalAngleOffset;

            ca = vaMath::Cos( angle0 );
            sa = vaMath::Sin( angle0 );

            float scale = 1.0f + (a-1.5f + (b - (subPassCount-1.0f) * 0.5f ) / (float)subPassCount ) * 0.07f;
            scale *= additionalRadiusScale;

            consts.PatternRotScaleMatrices[subPass] = vaVector4( scale * ca, scale * -sa, -scale * sa, -scale * ca );
        }
        
        consts.DebugRefSamplesDistribution              = m_debugRefSamplesDistribution;
        memset( consts.SamplesArray, 0, sizeof(consts.SamplesArray) );
        if( m_autoPatternGenerateModeEnabled && ( m_autoPatternGenerateModeCurrentCount != 0 ) )
        {
            consts.AutoMatchSampleCount = m_autoPatternGenerateModeCurrentCount;
            consts.DebugRefSamplesDistribution = 1.0f;
            UpdateSamplingPattern( consts.SamplesArray, m_autoPatternGenerateModeData, consts.AutoMatchSampleCount );
        }
        else
        {
            consts.AutoMatchSampleCount = GetRefSampleCount();
            UpdateSamplingPattern( consts.SamplesArray, GetRefSamples(), consts.AutoMatchSampleCount );
        }

        consts.DetailAOStrength = m_settings.DetailShadowStrength;
        consts.Dummy0           = 0.0f;
        consts.Dummy1           = 0.0f;
        consts.Dummy2           = 0.0f;

        m_constantsBuffer.Update( dx11Context, consts );
    }
}

void vaASSAODX11::FullscreenPassDraw( ID3D11DeviceContext * context, ID3D11PixelShader * pixelShader, ID3D11BlendState * blendState, ID3D11DepthStencilState * depthStencilState, UINT stencilRef )
{
    ID3D11VertexShader * vertexShader = m_vertexShader.GetShader( );

    // Topology
    context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

    // Vertex buffer
    const u_int stride = sizeof( CommonSimpleVertex );   UINT offsetInBytes = 0;
    context->IASetVertexBuffers( 0, 1, &m_fullscreenVB, &stride, &offsetInBytes );

    // Shaders and input layout

    context->IASetInputLayout( m_vertexShader.GetInputLayout( ) );
    context->VSSetShader( vertexShader, NULL, 0 );
    context->PSSetShader( pixelShader, NULL, 0 );

    float blendFactor[4] = { 0, 0, 0, 0 };
    context->OMSetBlendState( blendState, blendFactor, 0xFFFFFFFF );
    context->OMSetDepthStencilState( depthStencilState, stencilRef );
    context->RSSetState( m_fullscreenRasterizerState );

    context->Draw( 3, 0 );
}

void vaASSAODX11::PrepareDepths( vaDrawContext & drawContext, vaTexture & depthTexture, bool generateNormals )
{

    vaRenderDeviceContextDX11 * apiContext = drawContext.APIContext.SafeCast<vaRenderDeviceContextDX11*>( );
    ID3D11DeviceContext * dx11Context = apiContext->GetDXImmediateContext( );

    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, depthTexture.SafeCast<vaTextureDX11*>( )->GetSRV( ), SSAO_TEXTURE_SLOT0 );

    const shared_ptr<vaTexture> fourDepths[] = { m_halfDepths[0], m_halfDepths[1], m_halfDepths[2], m_halfDepths[3] };
    const shared_ptr<vaTexture> twoDepths[] = { m_halfDepths[0], m_halfDepths[3] };
    if( !generateNormals )
    {
        VA_SCOPE_CPUGPU_TIMER( PrepareDepths, drawContext.APIContext );

        if( m_settings.SkipHalfPixelsOnLowQualityLevel )
        {
            apiContext->SetRenderTargets( 2, twoDepths, nullptr, true );
            FullscreenPassDraw( dx11Context, m_pixelShaderPrepareDepthsHalf );
        }
        else
        {
            apiContext->SetRenderTargets( 4, fourDepths, nullptr, true );
            FullscreenPassDraw( dx11Context, m_pixelShaderPrepareDepths );
        }
    }
    else
    {
        VA_SCOPE_CPUGPU_TIMER( PrepareDepthsAndNormals, drawContext.APIContext );

        shared_ptr<vaTexture> uavs[] = { m_normals };
        if( m_settings.SkipHalfPixelsOnLowQualityLevel )
        {
            apiContext->SetRenderTargetsAndUnorderedAccessViews( 2, twoDepths, nullptr, SSAO_NORMALMAP_OUT_UAV_SLOT, 1, uavs, true );
            FullscreenPassDraw( dx11Context, m_pixelShaderPrepareDepthsAndNormalsHalf );
        }
        else
        {
            apiContext->SetRenderTargetsAndUnorderedAccessViews( 4, fourDepths, nullptr, SSAO_NORMALMAP_OUT_UAV_SLOT, 1, uavs, true );
            FullscreenPassDraw( dx11Context, m_pixelShaderPrepareDepthsAndNormals );
        }
    }

    // only do mipmaps for higher quality levels
    if( m_settings.QualityLevel > 1 )
    {
        VA_SCOPE_CPUGPU_TIMER( PrepareDepthMips, drawContext.APIContext );

        for( int i = 1; i < m_depthMipLevels; i++ )
        {
            const shared_ptr<vaTexture> fourDepthMips[] = { m_halfDepthsMipViews[0][i], m_halfDepthsMipViews[1][i], m_halfDepthsMipViews[2][i], m_halfDepthsMipViews[3][i] };

            apiContext->SetRenderTargets( 4, fourDepthMips, nullptr, true );

            vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, m_halfDepthsMipViews[0][i - 1]->SafeCast<vaTextureDX11*>( )->GetSRV( ), SSAO_TEXTURE_SLOT0 );
            vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, m_halfDepthsMipViews[1][i - 1]->SafeCast<vaTextureDX11*>( )->GetSRV( ), SSAO_TEXTURE_SLOT1 );
            vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, m_halfDepthsMipViews[2][i - 1]->SafeCast<vaTextureDX11*>( )->GetSRV( ), SSAO_TEXTURE_SLOT2 );
            vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, m_halfDepthsMipViews[3][i - 1]->SafeCast<vaTextureDX11*>( )->GetSRV( ), SSAO_TEXTURE_SLOT3 );
         
            FullscreenPassDraw( dx11Context, m_pixelShaderPrepareDepthMip[i-1] );
        }
    }
}

void vaASSAODX11::GenerateSSAO( vaDrawContext & drawContext, const vaMatrix4x4 & projMatrix, vaTexture & normalmapTexture, bool adaptiveBasePass )
{
    vaRenderDeviceContextDX11 * apiContext = drawContext.APIContext.SafeCast<vaRenderDeviceContextDX11*>( );
    ID3D11DeviceContext * dx11Context = apiContext->GetDXImmediateContext( );

    vaViewport halfResNormalVP( 0, 0, m_halfSize.x, m_halfSize.y );

    apiContext->SetViewportAndScissorRect( halfResNormalVP, m_halfResOutScissorRect );

    if( adaptiveBasePass )
    {
        assert( m_settings.QualityLevel == 3 );
    }

    int passCount = 4;

    for( int pass = 0; pass < passCount; pass++ )
    {
        if( m_settings.SkipHalfPixelsOnLowQualityLevel && ( (pass == 1) || (pass == 2) ) )
            continue;

        int blurPasses = m_settings.BlurPassCount;
        blurPasses = vaMath::Min( blurPasses, cMaxBlurPassCount );

        if( m_settings.QualityLevel == 3 )
        {
            // if adaptive, at least one blur pass needed as the first pass needs to read the final texture results - kind of awkward
            if( adaptiveBasePass )
                blurPasses = 0;
            else
                blurPasses = vaMath::Max( 1, blurPasses );
        } 
        else if( m_settings.QualityLevel == 0 )
        {
            // just one blur pass allowed for minimum quality 
            blurPasses = vaMath::Min( 1, m_settings.BlurPassCount );
        }

        UpdateConstants( drawContext, projMatrix, pass );

        shared_ptr<vaTexture> * pPingRT = &m_pingPongHalfResultA;
        shared_ptr<vaTexture> * pPongRT = &m_pingPongHalfResultB;

        // Generate
        {
            VA_SCOPE_CPUGPU_TIMER_NAMED( Generate, vaStringTools::Format( "Generate_quad%d", pass ), drawContext.APIContext );

            // to avoid API complaints
            vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT0 );
            vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT1 );
            vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT2 );
            vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT3 );
            vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT4 );
            
            shared_ptr<vaTexture> rts[] = { *pPingRT }; //, (m_settings.QualityLevel!=0)?(m_edges):(nullptr) };   // the quality level 0 doesn't export edges

            // no blur?
            if( blurPasses == 0 )
                rts[0] = m_finalResultsArrayViews[pass];

            apiContext->SetRenderTargets( _countof(rts), rts, nullptr, false );

            if( m_debugShowNormals || m_debugShowEdges || m_debugShowSampleHeatmap )
            {
                shared_ptr<vaTexture> uavs[] = { m_debuggingOutput };
                apiContext->SetRenderTargetsAndUnorderedAccessViews( _countof(rts), rts, nullptr, SSAO_DEBUGGINGOUTPUT_OUT_UAV_SLOT, 1, uavs, false );
            }

            vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, m_halfDepths[pass]->SafeCast<vaTextureDX11*>( )->GetSRV( ), SSAO_TEXTURE_SLOT0 );
            vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, normalmapTexture.SafeCast<vaTextureDX11*>( )->GetSRV( ), SSAO_TEXTURE_SLOT1 );
            
            if( !adaptiveBasePass && (m_settings.QualityLevel == 3) )
            {
                vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, m_loadCounter->SafeCast<vaTextureDX11*>( )->GetSRV( ), SSAO_TEXTURE_SLOT2 );
                vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, m_importanceMap->SafeCast<vaTextureDX11*>( )->GetSRV( ), SSAO_TEXTURE_SLOT3 );
                vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, m_finalResults->SafeCast<vaTextureDX11*>( )->GetSRV( ), SSAO_TEXTURE_SLOT4 );
            }

            FullscreenPassDraw( dx11Context, m_pixelShaderGenerate[(!adaptiveBasePass)?(m_settings.QualityLevel):(5)] );

            // to avoid API complaints
            vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT4 );

#ifdef SSAO_ALLOW_INTERNAL_SHADER_DEBUGGING
            if( m_debugShowSamplesAtCursor )
            {
                int pixelPassID = m_debugShowSamplesAtCursorPos.x % 2 + (m_debugShowSamplesAtCursorPos.y % 2) * 2;
                if( pixelPassID == pass )
                {
                    // shared_ptr<vaTexture> rts[] = { *pPingRT, ( m_settings.QualityLevel != 0 ) ? ( m_edges ) : ( nullptr ) };   // the quality level 0 doesn't export edges

                    shared_ptr<vaTexture> uavs[] = { vaSaferStaticCast<vaRenderingGlobalsDX11*>( &drawContext.Globals )->GetCurrentShaderDebugOutput() };

                    //apiContext->SetViewport( vaViewport( m_debugShowSamplesAtCursorPos.x/2, m_debugShowSamplesAtCursorPos.y/2, 1, 1 ) );
                    apiContext->SetRenderTargetsAndUnorderedAccessViews( _countof( rts ), rts, nullptr, SSAO_DEBUGGING_DUMP_UAV_SLOT, 1, uavs, false );
                    FullscreenPassDraw( dx11Context, m_pixelShaderGenerateDebug[(!adaptiveBasePass)?(m_settings.QualityLevel):(5)] );
                }
            }
#endif
        }
        
        // Blur
        if( blurPasses > 0 )
        {
            VA_SCOPE_CPUGPU_TIMER_NAMED( Blur, vaStringTools::Format( "Blur_quad%d", pass ), drawContext.APIContext );

            int wideBlursRemaining = vaMath::Max( 0, blurPasses-2 );

            for( int i = 0; i < blurPasses; i++ )
            {
                // remove textures from slots 0, 1 to avoid API complaints
                vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT2 );
                vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT3 );

                shared_ptr<vaTexture> rts[] = { *pPongRT };

                // last pass?
                if( i == (blurPasses-1) )
                    rts[0] = m_finalResultsArrayViews[pass];

                apiContext->SetRenderTargets( _countof( rts ), rts, nullptr, false );

                vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, (*pPingRT)->SafeCast<vaTextureDX11*>( )->GetSRV( ), SSAO_TEXTURE_SLOT2 );

                if( m_settings.QualityLevel != 0 )
                {
                    if( wideBlursRemaining > 0 )
                    {
                        FullscreenPassDraw( dx11Context, m_pixelShaderSmartBlurWide );
                        wideBlursRemaining--;
                    }
                    else
                    {
                        FullscreenPassDraw( dx11Context, m_pixelShaderSmartBlur );
                    }
                }
                else
                {
                    FullscreenPassDraw( dx11Context, m_pixelShaderNonSmartBlur ); // just for quality level 0
                }

                std::swap( pPingRT, pPongRT );
            }
        }
        
        // remove textures to avoid API complaints
        vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT2 );
        vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT3 );
        vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT0 );
        vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT1 );
    }
}

// draws provided depthTexture (can be the one obtained using GetDepthBuffer( )) into currently selected RT; relies on settings set in vaRenderingGlobals and will assert and return without doing anything if those are not present
void vaASSAODX11::Draw( vaDrawContext & drawContext, const vaMatrix4x4 & projMatrix, vaTexture & depthTexture, vaTexture & normalmapTexture, bool blend, bool generateNormals, const vaVector4i & scissorRect )
{
//    assert( drawContext.GetRenderingGlobalsUpdated( ) );    if( !drawContext.GetRenderingGlobalsUpdated( ) ) return;
    assert( ((normalmapTexture.GetSizeX() == m_size.x) || (normalmapTexture.GetSizeX() == m_size.x-1)) && ( (normalmapTexture.GetSizeY() == m_size.y) || (normalmapTexture.GetSizeY() == m_size.y-1)) );
    assert( !m_shadersDirty ); if( m_shadersDirty ) return;

    if( m_debugShowNormals || m_debugShowEdges || m_debugShowSampleHeatmap )
        blend = false;

    // must be fixed to 3 for auto-generate or 4 for reference!
    if( m_autoPatternGenerateModeEnabled )
    {
        assert( m_settings.QualityLevel != 0 );
        assert( m_settings.QualityLevel != 1 );
        assert( m_settings.QualityLevel != 2 );
        assert( m_settings.QualityLevel != 3 );
        m_settings.QualityLevel = vaMath::Clamp( m_settings.QualityLevel, 4, 4 );
    }

    if( m_debugExperimentalFullscreenReferencePath && generateNormals )
    {
        VA_LOG_ERROR( "Fullscreen (noninterleaved) reference SSAO path not available without normals" );
        m_debugExperimentalFullscreenReferencePath = false;
    }

    vaRenderDeviceContextDX11 * apiContext = drawContext.APIContext.SafeCast<vaRenderDeviceContextDX11*>( );
    ID3D11DeviceContext * dx11Context = apiContext->GetDXImmediateContext( );

    if( m_requiresClear )
    {
        float fourZeroes[4] = { 0, 0, 0, 0 };
        float fourOnes[4]   = { 1, 1, 1, 1 };
        m_halfDepths[0]->ClearRTV( drawContext.APIContext, fourZeroes );
        m_halfDepths[1]->ClearRTV( drawContext.APIContext, fourZeroes );
        m_halfDepths[2]->ClearRTV( drawContext.APIContext, fourZeroes );
        m_halfDepths[3]->ClearRTV( drawContext.APIContext, fourZeroes );
        m_pingPongHalfResultA->ClearRTV( drawContext.APIContext, fourOnes );
        m_pingPongHalfResultB->ClearRTV( drawContext.APIContext, fourZeroes );
        m_finalResultsArrayViews[0]->ClearRTV( drawContext.APIContext, fourOnes );
        m_finalResultsArrayViews[1]->ClearRTV( drawContext.APIContext, fourOnes );
        m_finalResultsArrayViews[2]->ClearRTV( drawContext.APIContext, fourOnes );
        m_finalResultsArrayViews[3]->ClearRTV( drawContext.APIContext, fourOnes );
        if( m_normals != nullptr ) m_normals->ClearRTV( drawContext.APIContext, fourZeroes );
        m_importanceMap->ClearRTV( drawContext.APIContext, fourZeroes );
        m_importanceMapPong->ClearRTV( drawContext.APIContext, fourZeroes );

        m_requiresClear = false;
    }

    UpdateConstants( drawContext, projMatrix, 0 );

    // setup API states
    {
        // Constants
        vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, (ID3D11Buffer*)NULL, SSAO_CONSTANTS_BUFFERSLOT );
        m_constantsBuffer.SetToD3DContextAllShaderTypes( dx11Context, SSAO_CONSTANTS_BUFFERSLOT );

        // Textures
        vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT0 );
        vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT1 );
        vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT2 );
        vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT3 );
        vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT4 );
        vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT5 );   // needed for debugging only

        // Samplers
        ID3D11SamplerState * samplers[] =
        {
            vaDirectXTools::GetSamplerStatePointClamp( ),
            vaDirectXTools::GetSamplerStateLinearClamp( ),
            vaDirectXTools::GetSamplerStatePointMirror( ),
            m_samplerStateViewspaceDepthTap,
        };
        vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, samplers, SSAO_SAMPLERS_SLOT0, _countof( samplers ) );
        // this fills SSAO_SAMPLERS_SLOT0 and SSAO_SAMPLERS_SLOT1
    }

     if( m_debugExperimentalFullscreenReferencePath )
     {
    //     VA_SCOPE_CPUGPU_TIMER( GenerateAndApplyNonInterleaved, drawContext.APIContext );
    // 
    //     vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, depthTexture.SafeCast<vaTextureDX11*>( )->GetSRV( ), SSAO_TEXTURE_SLOT0 );
    //     vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, normalmapTexture.SafeCast<vaTextureDX11*>( )->GetSRV( ), SSAO_TEXTURE_SLOT1 );
    // 
    //     ID3D11BlendState * blendState = (!blend)?( vaDirectXTools::GetBS_Opaque() ):( vaDirectXTools::GetBS_Mult() );
    // 
    //     FullscreenPassDraw( dx11Context, m_pixelShaderGenerateAndApplyReferenceFullscreen, blendState );
    // 
    //     // Reset API states
    //     vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT0 );
    //     vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT1 );
    //     vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, (ID3D11Buffer*)NULL, SSAO_CONSTANTS_BUFFERSLOT );
    // 
    //     // Reset samplers
    //     ID3D11SamplerState * samplers[4] = { nullptr, nullptr, nullptr, nullptr };
    //     vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, samplers, SSAO_SAMPLERS_SLOT0, _countof( samplers ) );
         return;
     }

    // backup currently set RTs
    vaRenderDeviceContext::OutputsState rtState = apiContext->GetOutputs();

    // Generate depths
    PrepareDepths( drawContext, depthTexture, generateNormals );

    // for adaptive quality, importance map pass
    if( m_settings.QualityLevel == 3 )
    {
        // Generate simple quality SSAO
        {
            VA_SCOPE_CPUGPU_TIMER( LowResGenerateAO, drawContext.APIContext );
            GenerateSSAO( drawContext, projMatrix, normalmapTexture, true );
        }

        // Generate importance map
        {
            VA_SCOPE_CPUGPU_TIMER( GenerateImportanceMap, drawContext.APIContext );

            // drawing into importanceMap
            apiContext->SetRenderTarget( m_importanceMap, nullptr, true );
            // select 4 deinterleaved AO textures (texture array)
            vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, m_finalResults->SafeCast<vaTextureDX11*>( )->GetSRV( ), SSAO_TEXTURE_SLOT4 );
            ID3D11BlendState * blendState = vaDirectXTools::GetBS_Opaque();
            FullscreenPassDraw( dx11Context, m_pixelShaderGenerateImportanceMap, blendState );
            
            // postprocess A
            apiContext->SetRenderTarget( m_importanceMapPong, nullptr, true );
            vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, m_importanceMap->SafeCast<vaTextureDX11*>( )->GetSRV( ), SSAO_TEXTURE_SLOT3 );
            FullscreenPassDraw( dx11Context, m_pixelShaderPostprocessImportanceMapA, blendState );
            vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, (ID3D11ShaderResourceView*)nullptr, SSAO_TEXTURE_SLOT3 );

            // postprocess B
            m_loadCounter->ClearUAV( drawContext.APIContext, vaVector4ui( 0, 0, 0, 0 ) );
            shared_ptr<vaTexture> uavs[] = { m_loadCounter };
            const shared_ptr<vaTexture> importanceMapRT[] = { m_importanceMap };
            apiContext->SetRenderTargetsAndUnorderedAccessViews( 1, importanceMapRT, nullptr, SSAO_LOAD_COUNTER_UAV_SLOT, 1, uavs, true );
            // select previous pass input importance map
            vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, m_importanceMapPong->SafeCast<vaTextureDX11*>( )->GetSRV( ), SSAO_TEXTURE_SLOT3 );
            FullscreenPassDraw( dx11Context, m_pixelShaderPostprocessImportanceMapB, blendState );
        }
    }

    // Generate SSAO
    {
        VA_SCOPE_CPUGPU_TIMER( GenerateAO, drawContext.APIContext );
        GenerateSSAO( drawContext, projMatrix, normalmapTexture, false );
    }

    // Apply
    {
        VA_SCOPE_CPUGPU_TIMER( Apply, drawContext.APIContext );
    #ifdef SSAO_ENABLE_ALTERNATIVE_APPLY
        if( m_debugUseAlternativeApply && m_settings.QualityLevel != 0 )
        {
            VA_SCOPE_CPUGPU_TIMER( AlternativeApplySmartBlur, drawContext.APIContext );
            for ( int i = 0; i < 4; i++ )
            {
                apiContext->SetRenderTarget( m_AAFinalResultsArrayViews[i], nullptr, true );
                if( i == 0 )
                {
                    // select 4 deinterleaved AO textures (texture array)
                    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, m_finalResults->SafeCast<vaTextureDX11*>( )->GetSRV( ), SSAO_TEXTURE_SLOT4 );
                }
                FullscreenPassDraw( dx11Context, m_pixelShaderAlternativeApplySmartBlur[i] );
            }
        }
    #endif

        // restore previous RTs
        apiContext->SetOutputs( rtState );


        // select 4 deinterleaved AO textures (texture array)
        vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, m_finalResults->SafeCast<vaTextureDX11*>( )->GetSRV( ), SSAO_TEXTURE_SLOT4 );


        if( m_debugShowNormals || m_debugShowEdges || m_debugShowSampleHeatmap )
        {
            vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, m_debuggingOutput->SafeCast<vaTextureDX11*>( )->GetSRV( ), SSAO_TEXTURE_SLOT5 );  // needed for debugging only
        }

        ID3D11BlendState * blendState = (!blend)?( vaDirectXTools::GetBS_Opaque() ):( vaDirectXTools::GetBS_Mult() );

        vaViewport applyVP( 0, 0, m_size.x, m_size.y );
        
        apiContext->SetViewportAndScissorRect( applyVP, m_fullResOutScissorRect );

        if( m_settings.QualityLevel == 0 )
        {
            if( m_settings.SkipHalfPixelsOnLowQualityLevel )
                FullscreenPassDraw( dx11Context, m_pixelShaderNonSmartHalfApply, blendState );
            else
                FullscreenPassDraw( dx11Context, m_pixelShaderNonSmartApply, blendState );
        }
        else
        {
#ifdef SSAO_ENABLE_ALTERNATIVE_APPLY
            if( m_debugUseAlternativeApply )
            {
                VA_SCOPE_CPUGPU_TIMER( AlternativeApplyInterleave, drawContext.APIContext );
                vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, m_AAFinalResults->SafeCast<vaTextureDX11*>( )->GetSRV( ), SSAO_TEXTURE_SLOT4 );
                FullscreenPassDraw( dx11Context, m_pixelShaderAlternativeApplyInterleave, blendState );
            }
            else
#endif
            FullscreenPassDraw( dx11Context, m_pixelShaderApply, blendState );
        }

        // restore VP
        apiContext->SetViewport( rtState.Viewport );
    }

//    FullscreenPassDraw( dx11Context, m_pixelShaderDebugDraw );

    // Reset API states
    {
        vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT0 );
        vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT1 );
        vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT2 );
        vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT3 );
        vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT4 );
        vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, SSAO_TEXTURE_SLOT5 );  // needed for debugging only
        vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, (ID3D11Buffer*)NULL, SSAO_CONSTANTS_BUFFERSLOT );

        // Samplers
        ID3D11SamplerState * samplers[4] = { nullptr, nullptr, nullptr, nullptr };
        vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, samplers, SSAO_SAMPLERS_SLOT0, _countof( samplers ) );
    }
}

void vaASSAODX11::Draw( vaDrawContext & drawContext, const vaMatrix4x4 & projMatrix, vaTexture & depthTexture, bool blend, vaTexture * normalmapTexture, const vaVector4i & scissorRect )
{
    UpdateTextures( drawContext, depthTexture.GetSizeX( ), depthTexture.GetSizeY( ), normalmapTexture == nullptr, scissorRect );
    if( normalmapTexture != nullptr )
        Draw( drawContext, projMatrix, depthTexture, *normalmapTexture, blend, false, scissorRect );
    else
        Draw( drawContext, projMatrix, depthTexture, *m_normals, blend, true, scissorRect );
}

void RegisterASSAODX11( )
{
    VA_RENDERING_MODULE_REGISTER( vaASSAO, vaASSAODX11 );
}

namespace
{
    // 512 points:
    static const vaVector2 g_referenceSamples[SSAO_MAX_REF_TAPS] =
    {
        { 0.032120f, -0.929580f  },
        { 0.037570f, -0.998120f  },
        { 0.022250f, -0.559440f  },
        { 0.037880f, -0.801680f  },
        { 0.045170f, -0.675860f  },
        { 0.094060f, -0.953030f  },
        { 0.032820f, -0.281180f  },
        { 0.089760f, -0.738250f  },
        { 0.107150f, -0.828790f  },
        { 0.054770f, -0.380750f  },
        { 0.135090f, -0.897370f  },
        { 0.086180f, -0.524740f  },
        { 0.078160f, -0.444520f  },
        { 0.109320f, -0.603960f  },
        { 0.179080f, -0.955820f  },
        { 0.032450f, -0.160380f  },
        { 0.168610f, -0.800840f  },
        { 0.149090f, -0.706590f  },
        { 0.220500f, -0.847030f  },
        { 0.171710f, -0.640610f  },
        { 0.152720f, -0.528510f  },
        { 0.224620f, -0.764530f  },
        { 0.095390f, -0.315910f  },
        { 0.288630f, -0.946500f  },
        { 0.281850f, -0.880300f  },
        { 0.225530f, -0.693400f  },
        { 0.149430f, -0.428970f  },
        { 0.083680f, -0.232330f  },
        { 0.228070f, -0.603820f  },
        { 0.314150f, -0.818050f  },
        { 0.290750f, -0.751830f  },
        { 0.357110f, -0.922990f  },
        { 0.201030f, -0.475800f  },
        { 0.293260f, -0.673090f  },
        { 0.386140f, -0.851960f  },
        { 0.243340f, -0.535440f  },
        { 0.387460f, -0.784950f  },
        { 0.356510f, -0.693660f  },
        { 0.302090f, -0.574580f  },
        { 0.462400f, -0.876670f  },
        { 0.195580f, -0.370480f  },
        { 0.166740f, -0.302920f  },
        { 0.361600f, -0.626320f  },
        { 0.474760f, -0.788640f  },
        { 0.304010f, -0.499840f  },
        { 0.525610f, -0.848980f  },
        { 0.474500f, -0.705070f  },
        { 0.280780f, -0.415010f  },
        { 0.444470f, -0.645310f  },
        { 0.547530f, -0.784440f  },
        { 0.167600f, -0.236600f  },
        { 0.373580f, -0.524310f  },
        { 0.413020f, -0.578830f  },
        { 0.047140f, -0.063960f  },
        { 0.119100f, -0.160480f  },
        { 0.235690f, -0.313830f  },
        { 0.546470f, -0.707740f  },
        { 0.343920f, -0.434290f  },
        { 0.618830f, -0.768910f  },
        { 0.614760f, -0.699490f  },
        { 0.460740f, -0.522220f  },
        { 0.326550f, -0.363340f  },
        { 0.532680f, -0.590640f  },
        { 0.585750f, -0.638740f  },
        { 0.681460f, -0.706000f  },
        { 0.446830f, -0.436420f  },
        { 0.315980f, -0.291130f  },
        { 0.642110f, -0.585680f  },
        { 0.589260f, -0.533460f  },
        { 0.742780f, -0.663790f  },
        { 0.397290f, -0.353610f  },
        { 0.261720f, -0.226690f  },
        { 0.557360f, -0.471820f  },
        { 0.710350f, -0.595720f  },
        { 0.781140f, -0.601180f  },
        { 0.490090f, -0.367700f  },
        { 0.692750f, -0.516340f  },
        { 0.633710f, -0.469260f  },
        { 0.128390f, -0.094990f  },
        { 0.556010f, -0.396440f  },
        { 0.184550f, -0.130650f  },
        { 0.377060f, -0.263470f  },
        { 0.439760f, -0.290520f  },
        { 0.762110f, -0.492940f  },
        { 0.703640f, -0.446680f  },
        { 0.249170f, -0.151520f  },
        { 0.654970f, -0.393780f  },
        { 0.852790f, -0.500020f  },
        { 0.805290f, -0.438330f  },
        { 0.568800f, -0.307160f  },
        { 0.386460f, -0.196090f  },
        { 0.644940f, -0.323670f  },
        { 0.712070f, -0.346310f  },
        { 0.878570f, -0.425700f  },
        { 0.491910f, -0.237080f  },
        { 0.774430f, -0.368010f  },
        { 0.314630f, -0.140460f  },
        { 0.857600f, -0.362910f  },
        { 0.593480f, -0.245030f  },
        { 0.454270f, -0.182740f  },
        { 0.930310f, -0.359830f  },
        { 0.669780f, -0.253950f  },
        { 0.761390f, -0.287450f  },
        { 0.888330f, -0.304050f  },
        { 0.183880f, -0.059000f  },
        { 0.578020f, -0.175780f  },
        { 0.812760f, -0.242600f  },
        { 0.255560f, -0.074160f  },
        { 0.959540f, -0.276810f  },
        { 0.437940f, -0.118250f  },
        { 0.902690f, -0.228760f  },
        { 0.699020f, -0.176680f  },
        { 0.507590f, -0.121050f  },
        { 0.805580f, -0.173360f  },
        { 0.973740f, -0.209420f  },
        { 0.345380f, -0.070710f  },
        { 0.649730f, -0.117090f  },
        { 0.580340f, -0.103880f  },
        { 0.921140f, -0.162340f  },
        { 0.753470f, -0.122730f  },
        { 0.114270f, -0.015440f  },
        { 0.847980f, -0.107780f  },
        { 0.956220f, -0.104100f  },
        { 0.539110f, -0.050620f  },
        { 0.463080f, -0.041310f  },
        { 0.725710f, -0.053810f  },
        { 0.607210f, -0.040430f  },
        { 0.800400f, -0.053150f  },
        { 0.392670f, -0.022780f  },
        { 0.233400f, -0.011730f  },
        { 0.301680f, -0.011900f  },
        { 0.924250f, -0.019080f  },
        { 0.858300f, -0.012050f  },
        { 0.992950f, -0.008940f  },
        { 0.800260f, 0.022000f   },
        { 0.541450f, 0.018450f   },
        { 0.714050f, 0.027400f   },
        { 0.623110f, 0.032540f   },
        { 0.924530f, 0.048520f   },
        { 0.440850f, 0.032540f   },
        { 0.976070f, 0.094420f   },
        { 0.871920f, 0.089950f   },
        { 0.766840f, 0.084660f   },
        { 0.373200f, 0.042490f   },
        { 0.505070f, 0.076400f   },
        { 0.661920f, 0.100210f   },
        { 0.573450f, 0.091100f   },
        { 0.934280f, 0.155290f   },
        { 0.758270f, 0.158250f   },
        { 0.271360f, 0.058300f   },
        { 0.816690f, 0.191130f   },
        { 0.680540f, 0.164130f   },
        { 0.940210f, 0.227080f   },
        { 0.384580f, 0.112160f   },
        { 0.868630f, 0.258150f   },
        { 0.573200f, 0.175200f   },
        { 0.947230f, 0.295890f   },
        { 0.205110f, 0.064190f   },
        { 0.770480f, 0.254570f   },
        { 0.441930f, 0.151860f   },
        { 0.655550f, 0.235420f   },
        { 0.832520f, 0.316640f   },
        { 0.915130f, 0.372670f   },
        { 0.498840f, 0.204990f   },
        { 0.787520f, 0.368970f   },
        { 0.329400f, 0.156390f   },
        { 0.615970f, 0.294690f   },
        { 0.855720f, 0.416440f   },
        { 0.712300f, 0.347110f   },
        { 0.533940f, 0.289110f   },
        { 0.792750f, 0.438930f   },
        { 0.258360f, 0.150010f   },
        { 0.843240f, 0.492090f   },
        { 0.451500f, 0.274900f   },
        { 0.679270f, 0.421070f   },
        { 0.143600f, 0.090530f   },
        { 0.370250f, 0.234530f   },
        { 0.591090f, 0.390720f   },
        { 0.821710f, 0.554460f   },
        { 0.524110f, 0.355700f   },
        { 0.713380f, 0.499540f   },
        { 0.078910f, 0.055370f   },
        { 0.647370f, 0.479140f   },
        { 0.744790f, 0.592610f   },
        { 0.397250f, 0.323510f   },
        { 0.581800f, 0.490970f   },
        { 0.675770f, 0.571850f   },
        { 0.178560f, 0.151840f   },
        { 0.464610f, 0.400380f   },
        { 0.747480f, 0.659840f   },
        { 0.322410f, 0.297430f   },
        { 0.493390f, 0.459830f   },
        { 0.254380f, 0.245140f   },
        { 0.662040f, 0.638120f   },
        { 0.694010f, 0.705310f   },
        { 0.570170f, 0.580610f   },
        { 0.349770f, 0.371200f   },
        { 0.396310f, 0.430310f   },
        { 0.484070f, 0.530000f   },
        { 0.575210f, 0.689050f   },
        { 0.626270f, 0.755850f   },
        { 0.524380f, 0.634520f   },
        { 0.420260f, 0.513140f   },
        { 0.009610f, 0.012730f   },
        { 0.164810f, 0.219970f   },
        { 0.458450f, 0.629030f   },
        { 0.510020f, 0.711560f   },
        { 0.291560f, 0.411710f   },
        { 0.229140f, 0.328840f   },
        { 0.547140f, 0.790590f   },
        { 0.352950f, 0.521470f   },
        { 0.390940f, 0.595960f   },
        { 0.432960f, 0.701920f   },
        { 0.471370f, 0.766870f   },
        { 0.483540f, 0.832590f   },
        { 0.328530f, 0.631570f   },
        { 0.405360f, 0.781090f   },
        { 0.245150f, 0.482320f   },
        { 0.155050f, 0.307090f   },
        { 0.353630f, 0.705420f   },
        { 0.182410f, 0.400190f   },
        { 0.387540f, 0.879110f   },
        { 0.086920f, 0.199320f   },
        { 0.244500f, 0.567330f   },
        { 0.319640f, 0.812950f   },
        { 0.276190f, 0.741990f   },
        { 0.099640f, 0.270020f   },
        { 0.236670f, 0.651350f   },
        { 0.336760f, 0.929600f   },
        { 0.187920f, 0.525700f   },
        { 0.040230f, 0.117760f   },
        { 0.290680f, 0.877680f   },
        { 0.185050f, 0.608330f   },
        { 0.211220f, 0.716560f   },
        { 0.135860f, 0.470250f   },
        { 0.205980f, 0.808830f   },
        { 0.217040f, 0.888530f   },
        { 0.230570f, 0.964980f   },
        { 0.095260f, 0.415640f   },
        { 0.127350f, 0.570120f   },
        { 0.129710f, 0.672640f   },
        { 0.058150f, 0.322990f   },
        { 0.129510f, 0.816270f   },
        { 0.111230f, 0.738970f   },
        { 0.143480f, 0.966700f   },
        { 0.121820f, 0.887710f   },
        { 0.082820f, 0.620010f   },
        { 0.046730f, 0.544410f   },
        { 0.050820f, 0.696040f   },
        { 0.070980f, 0.989690f   },
        { 0.061170f, 0.922350f   },
        { 0.025370f, 0.406280f   },
        { 0.025600f, 0.480830f   },
        { 0.045100f, 0.851130f   },
        { 0.037550f, 0.774350f   },
        { 0.005660f, 0.227430f   },
        { 0.015730f, 0.636460f   },
        { -0.021450f, 0.877390f  },
        { -0.021360f, 0.810240f  },
        { -0.026830f, 0.961950f  },
        { -0.019670f, 0.699980f  },
        { -0.027970f, 0.340120f  },
        { -0.059700f, 0.617600f  },
        { -0.084190f, 0.788600f  },
        { -0.108860f, 0.972270f  },
        { -0.102090f, 0.905560f  },
        { -0.089140f, 0.679510f  },
        { -0.069540f, 0.526510f  },
        { -0.128060f, 0.841770f  },
        { -0.169100f, 0.903920f  },
        { -0.141760f, 0.749630f  },
        { -0.195810f, 0.978680f  },
        { -0.088640f, 0.431500f  },
        { -0.064330f, 0.280680f  },
        { -0.195420f, 0.806640f  },
        { -0.156420f, 0.637400f  },
        { -0.131330f, 0.499170f  },
        { -0.246000f, 0.919160f  },
        { -0.097920f, 0.357820f  },
        { -0.204850f, 0.722900f  },
        { -0.160810f, 0.566450f  },
        { -0.063020f, 0.199200f  },
        { -0.324200f, 0.934220f  },
        { -0.277830f, 0.777360f  },
        { -0.305300f, 0.852530f  },
        { -0.158130f, 0.438240f  },
        { -0.227670f, 0.592760f  },
        { -0.219310f, 0.523150f  },
        { -0.285810f, 0.649080f  },
        { -0.417300f, 0.905380f  },
        { -0.350490f, 0.755350f  },
        { -0.117790f, 0.239670f  },
        { -0.178800f, 0.352960f  },
        { -0.348830f, 0.683100f  },
        { -0.423400f, 0.813440f  },
        { -0.240980f, 0.459440f  },
        { -0.071890f, 0.127170f  },
        { -0.333860f, 0.570090f  },
        { -0.302100f, 0.511970f  },
        { -0.505790f, 0.848140f  },
        { -0.454900f, 0.752690f  },
        { -0.411080f, 0.649580f  },
        { -0.567330f, 0.814510f  },
        { -0.533670f, 0.748170f  },
        { -0.326670f, 0.450050f  },
        { -0.395090f, 0.537940f  },
        { -0.289580f, 0.393690f  },
        { -0.202540f, 0.272080f  },
        { -0.483240f, 0.634430f  },
        { -0.459920f, 0.568600f  },
        { -0.634600f, 0.753050f  },
        { -0.584130f, 0.692130f  },
        { -0.139500f, 0.165070f  },
        { -0.559570f, 0.628970f  },
        { -0.405130f, 0.447510f  },
        { -0.300410f, 0.328640f  },
        { -0.360100f, 0.382710f  },
        { -0.685870f, 0.696500f  },
        { -0.636110f, 0.644190f  },
        { -0.568790f, 0.545360f  },
        { -0.504130f, 0.468290f  },
        { -0.267050f, 0.246670f  },
        { -0.635980f, 0.577740f  },
        { -0.719770f, 0.637050f  },
        { -0.442040f, 0.388930f  },
        { -0.379780f, 0.318000f  },
        { -0.599920f, 0.485130f  },
        { -0.683470f, 0.525400f  },
        { -0.762790f, 0.586070f  },
        { -0.074000f, 0.054920f  },
        { -0.240350f, 0.178130f  },
        { -0.339050f, 0.246550f  },
        { -0.557870f, 0.387490f  },
        { -0.499910f, 0.340930f  },
        { -0.620450f, 0.417700f  },
        { -0.755760f, 0.505650f  },
        { -0.814110f, 0.539650f  },
        { -0.444320f, 0.294440f  },
        { -0.698990f, 0.460690f  },
        { -0.760990f, 0.436700f  },
        { -0.167760f, 0.095560f  },
        { -0.688980f, 0.384920f  },
        { -0.828910f, 0.462220f  },
        { -0.618440f, 0.332220f  },
        { -0.547680f, 0.271310f  },
        { -0.452390f, 0.219310f  },
        { -0.890340f, 0.430460f  },
        { -0.356240f, 0.159550f  },
        { -0.803300f, 0.334160f  },
        { -0.743420f, 0.299930f  },
        { -0.892960f, 0.354620f  },
        { -0.656860f, 0.240490f  },
        { -0.243460f, 0.084970f  },
        { -0.865350f, 0.293480f  },
        { -0.728420f, 0.233040f  },
        { -0.438230f, 0.139120f  },
        { -0.932000f, 0.295480f  },
        { -0.556060f, 0.174940f  },
        { -0.824740f, 0.231700f  },
        { -0.668490f, 0.170850f  },
        { -0.325450f, 0.079680f  },
        { -0.928860f, 0.221860f  },
        { -0.509180f, 0.112310f  },
        { -0.736110f, 0.159940f  },
        { -0.395380f, 0.081250f  },
        { -0.815380f, 0.159470f  },
        { -0.927050f, 0.147010f  },
        { -0.740840f, 0.093830f  },
        { -0.573960f, 0.072050f  },
        { -0.640690f, 0.076140f  },
        { -0.989020f, 0.108240f  },
        { -0.847960f, 0.085990f  },
        { -0.949350f, 0.051560f  },
        { -0.802190f, 0.037680f  },
        { -0.371540f, 0.017430f  },
        { -0.703870f, 0.029640f  },
        { -0.524760f, 0.017120f  },
        { -0.241660f, 0.006010f  },
        { -0.438190f, 0.008200f  },
        { -0.897780f, -0.002510f },
        { -0.648490f, -0.008860f },
        { -0.759160f, -0.013630f },
        { -0.988680f, -0.036890f },
        { -0.582920f, -0.026770f },
        { -0.825010f, -0.038940f },
        { -0.165630f, -0.008040f },
        { -0.305070f, -0.017740f },
        { -0.494210f, -0.044770f },
        { -0.898600f, -0.088870f },
        { -0.717250f, -0.070940f },
        { -0.993980f, -0.104950f },
        { -0.823690f, -0.112980f },
        { -0.637050f, -0.087580f },
        { -0.412550f, -0.066360f },
        { -0.962950f, -0.169770f },
        { -0.558530f, -0.104600f },
        { -0.751720f, -0.142020f },
        { -0.854940f, -0.179880f },
        { -0.343500f, -0.073960f },
        { -0.911740f, -0.213910f },
        { -0.649620f, -0.153930f },
        { -0.247350f, -0.060560f },
        { -0.775420f, -0.215860f },
        { -0.941220f, -0.276260f },
        { -0.508320f, -0.152120f },
        { -0.431540f, -0.131260f },
        { -0.864000f, -0.267120f },
        { -0.571930f, -0.178240f },
        { -0.703920f, -0.232350f },
        { -0.790400f, -0.281460f },
        { -0.621430f, -0.225670f },
        { -0.914090f, -0.354360f },
        { -0.842200f, -0.337560f },
        { -0.297380f, -0.133360f },
        { -0.514280f, -0.232420f },
        { -0.695600f, -0.320390f },
        { -0.570140f, -0.273450f },
        { -0.096620f, -0.046600f },
        { -0.809590f, -0.395230f },
        { -0.389450f, -0.193650f },
        { -0.447230f, -0.229180f },
        { -0.622150f, -0.323800f },
        { -0.863000f, -0.456940f },
        { -0.723080f, -0.386530f },
        { -0.180890f, -0.102570f },
        { -0.796070f, -0.471420f },
        { -0.645290f, -0.392670f },
        { -0.844220f, -0.521760f },
        { -0.731750f, -0.454410f },
        { -0.467620f, -0.292650f },
        { -0.321890f, -0.206520f },
        { -0.552790f, -0.363660f },
        { -0.382950f, -0.265410f },
        { -0.770250f, -0.535640f },
        { -0.612070f, -0.457480f },
        { -0.786770f, -0.610080f },
        { -0.545460f, -0.432860f },
        { -0.711450f, -0.570720f },
        { -0.643580f, -0.526140f },
        { -0.423990f, -0.351180f },
        { -0.483770f, -0.402440f },
        { -0.701250f, -0.639500f },
        { -0.315790f, -0.296040f },
        { -0.591870f, -0.584800f },
        { -0.256030f, -0.254390f },
        { -0.193830f, -0.197690f },
        { -0.526600f, -0.543890f },
        { -0.400630f, -0.414330f },
        { -0.337760f, -0.364300f },
        { -0.617970f, -0.674250f },
        { -0.438000f, -0.483290f },
        { -0.661240f, -0.748650f },
        { -0.112120f, -0.132410f },
        { -0.452650f, -0.573250f },
        { -0.566480f, -0.719530f },
        { -0.371940f, -0.475090f },
        { -0.512110f, -0.669510f },
        { -0.592540f, -0.801150f },
        { -0.225500f, -0.313460f },
        { -0.385320f, -0.554280f },
        { -0.274290f, -0.416760f },
        { -0.484550f, -0.759790f },
        { -0.394610f, -0.623400f },
        { -0.161580f, -0.260850f },
        { -0.436710f, -0.706080f },
        { -0.514810f, -0.833290f },
        { -0.321010f, -0.536930f },
        { -0.117050f, -0.200430f },
        { -0.216270f, -0.379240f },
        { -0.442020f, -0.811950f },
        { -0.267780f, -0.496480f },
        { -0.329170f, -0.614700f },
        { -0.391210f, -0.764170f },
        { -0.434220f, -0.880630f },
        { -0.345400f, -0.702950f },
        { -0.208860f, -0.451270f },
        { -0.023200f, -0.050690f },
        { -0.263270f, -0.597930f },
        { -0.272360f, -0.668100f },
        { -0.212490f, -0.533590f },
        { -0.150990f, -0.389850f },
        { -0.318620f, -0.823450f },
        { -0.292710f, -0.762080f },
        { -0.339090f, -0.912740f },
        { -0.042780f, -0.118790f },
        { -0.114510f, -0.324790f },
        { -0.201110f, -0.620180f },
        { -0.081910f, -0.261910f },
        { -0.216830f, -0.716020f },
        { -0.251630f, -0.835500f },
        { -0.271000f, -0.960990f },
        { -0.137070f, -0.513460f },
        { -0.234840f, -0.905800f },
        { -0.197340f, -0.779770f },
        { -0.106380f, -0.443610f },
        { -0.041460f, -0.190220f },
        { -0.134590f, -0.623550f },
        { -0.158000f, -0.882120f },
        { -0.122290f, -0.708230f },
        { -0.129650f, -0.796050f },
        { -0.142160f, -0.978990f },
        { -0.076080f, -0.541350f },
        { -0.076310f, -0.838460f },
        { -0.028060f, -0.308480f },
        { -0.068670f, -0.914500f },
        { -0.025410f, -0.387320f },
        { -0.036610f, -0.707960f },
        { -0.023020f, -0.495550f },
        { -0.032990f, -0.775020f },
        { -0.031070f, -0.985340f },
        { -0.011940f, -0.637970f },
        { -0.007230f, -0.876590f },
    };
}

static const vaVector2 *    GetRefSamples( )        { return g_referenceSamples; }
static int                  GetRefSampleCount( )    { return _countof(g_referenceSamples); }
