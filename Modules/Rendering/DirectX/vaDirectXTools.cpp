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

#include "vaDirectXTools.h"
//#include "DirectX\vaDirectXCanvas.h"
#include "Rendering/DirectX/Tools/DirectXTex-master/DDSTextureLoader/DDSTextureLoader.h"
#include "Rendering/DirectX/Tools/DirectXTex-master/WICTextureLoader/WICTextureLoader.h"
#include "Rendering/DirectX/Tools/DirectXTex-master/DirectXTex/DirectXTex.h"
//#include "DirectXTex\DirectXTex.h"

#include <stdarg.h>

#include <dxgiformat.h>
#include <assert.h>
#include <memory>
#include <algorithm>

#include <wrl\client.h>

using Microsoft::WRL::ComPtr;

//#include "Rendering/DirectX/Tools/DirectXTex-master/ScreenGrab/ScreenGrab.h"

#pragma warning (default : 4995)

using namespace std;
using namespace VertexAsylum;

static ID3D11RasterizerState *      g_RS_CullNone_Fill = NULL;
static ID3D11RasterizerState *      g_RS_CullCCW_Fill = NULL;
static ID3D11RasterizerState *      g_RS_CullCW_Fill = NULL;
static ID3D11RasterizerState *      g_RS_CullNone_Wireframe = NULL;
static ID3D11RasterizerState *      g_RS_CullCW_Wireframe = NULL;
static ID3D11RasterizerState *      g_RS_CullCCW_Wireframe = NULL;


static ID3D11DepthStencilState *    g_DSS_DepthEnabledL_DepthWrite = NULL;
static ID3D11DepthStencilState *    g_DSS_DepthEnabledG_DepthWrite = NULL;
static ID3D11DepthStencilState *    g_DSS_DepthEnabledLE_NoDepthWrite = NULL;
static ID3D11DepthStencilState *    g_DSS_DepthEnabledL_NoDepthWrite = NULL;
static ID3D11DepthStencilState *    g_DSS_DepthEnabledGE_NoDepthWrite = NULL;
static ID3D11DepthStencilState *    g_DSS_DepthEnabledG_NoDepthWrite = NULL;
static ID3D11DepthStencilState *    g_DSS_DepthDisabled_NoDepthWrite = NULL;
static ID3D11DepthStencilState *    g_DSS_DepthPassAlways_DepthWrite = NULL;
static ID3D11DepthStencilState *    g_DSS_DepthDisabled_StencilCreateMask = NULL;
static ID3D11DepthStencilState *    g_DSS_DepthDisabled_StencilUseMask = NULL;

static ID3D11BlendState *           g_BS_Opaque = NULL;
static ID3D11BlendState *           g_BS_Additive = NULL;
static ID3D11BlendState *           g_BS_AlphaBlend = NULL;
static ID3D11BlendState *           g_BS_PremultAlphaBlend = NULL;
static ID3D11BlendState *           g_BS_Mult = NULL;

static ID3D11ShaderResourceView *   g_Texture2D_SRV_White1x1 = NULL;

static ID3D11ShaderResourceView *   g_Texture2D_SRV_Noise2D = NULL;
static ID3D11ShaderResourceView *   g_Texture2D_SRV_Noise3D = NULL;

static ID3D11SamplerState *         g_samplerStatePointClamp = NULL;
static ID3D11SamplerState *         g_samplerStatePointWrap = NULL;
static ID3D11SamplerState *         g_samplerStatePointMirror = NULL;
static ID3D11SamplerState *         g_samplerStateLinearClamp = NULL;
static ID3D11SamplerState *         g_samplerStateLinearWrap = NULL;
static ID3D11SamplerState *         g_samplerStateAnisotropicClamp = NULL;
static ID3D11SamplerState *         g_samplerStateAnisotropicWrap = NULL;

struct D3D11_RASTERIZER_DESCComparer
{
    bool operator()( const D3D11_RASTERIZER_DESC & Left, const D3D11_RASTERIZER_DESC & Right ) const
    {
        // comparison logic goes here
        return memcmp( &Left, &Right, sizeof( Right ) ) < 0;
    }
};

class vaDirectXTools_RasterizerStatesLib
{
    ID3D11Device *                  m_device;

    std::map< D3D11_RASTERIZER_DESC, ID3D11RasterizerState *, D3D11_RASTERIZER_DESCComparer >
        m_items;

public:
    vaDirectXTools_RasterizerStatesLib( ID3D11Device* device )
    {
        m_device = device;
    }
    ~vaDirectXTools_RasterizerStatesLib( )
    {
        for( auto it = m_items.begin( ); it != m_items.end( ); it++ )
        {
            SAFE_RELEASE( it->second );
        }
    }

    ID3D11RasterizerState * vaDirectXTools_RasterizerStatesLib::FindOrCreateRasterizerState( const D3D11_RASTERIZER_DESC & desc );
};

static vaDirectXTools_RasterizerStatesLib * g_rasterizerStatesLib = NULL;

void vaDirectXTools_OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain )
{
    HRESULT hr;

    g_rasterizerStatesLib = new vaDirectXTools_RasterizerStatesLib( device );

    {
        CD3D11_RASTERIZER_DESC desc = CD3D11_RASTERIZER_DESC( CD3D11_DEFAULT( ) );

        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_NONE;
        device->CreateRasterizerState( &desc, &g_RS_CullNone_Fill );
        desc.FillMode = D3D11_FILL_WIREFRAME;
        device->CreateRasterizerState( &desc, &g_RS_CullNone_Wireframe );

        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_BACK;
        desc.FrontCounterClockwise = true;

        device->CreateRasterizerState( &desc, &g_RS_CullCW_Fill );
        desc.FillMode = D3D11_FILL_WIREFRAME;
        device->CreateRasterizerState( &desc, &g_RS_CullCW_Wireframe );

        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_BACK;
        desc.FrontCounterClockwise = false;

        device->CreateRasterizerState( &desc, &g_RS_CullCCW_Fill );
        desc.FillMode = D3D11_FILL_WIREFRAME;
        device->CreateRasterizerState( &desc, &g_RS_CullCCW_Wireframe );
    }

    {
        CD3D11_DEPTH_STENCIL_DESC desc = CD3D11_DEPTH_STENCIL_DESC( CD3D11_DEFAULT( ) );

        desc.DepthEnable = TRUE;
        desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
        V( device->CreateDepthStencilState( &desc, &g_DSS_DepthPassAlways_DepthWrite ) );

        desc.DepthEnable = TRUE;
        desc.DepthFunc = D3D11_COMPARISON_LESS;
        V( device->CreateDepthStencilState( &desc, &g_DSS_DepthEnabledL_DepthWrite ) );

        desc.DepthFunc = D3D11_COMPARISON_GREATER;
        V( device->CreateDepthStencilState( &desc, &g_DSS_DepthEnabledG_DepthWrite ) );

        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        desc.DepthFunc = D3D11_COMPARISON_LESS;
        V( device->CreateDepthStencilState( &desc, &g_DSS_DepthEnabledL_NoDepthWrite ) );

        desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        V( device->CreateDepthStencilState( &desc, &g_DSS_DepthEnabledLE_NoDepthWrite ) );

        desc.DepthFunc = D3D11_COMPARISON_GREATER;
        V( device->CreateDepthStencilState( &desc, &g_DSS_DepthEnabledG_NoDepthWrite ) );

        desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
        V( device->CreateDepthStencilState( &desc, &g_DSS_DepthEnabledGE_NoDepthWrite ) );

        desc.DepthEnable = FALSE;
        desc.DepthFunc = D3D11_COMPARISON_LESS;
        V( device->CreateDepthStencilState( &desc, &g_DSS_DepthDisabled_NoDepthWrite ) );

        desc.StencilEnable = true;
        desc.StencilReadMask = 0xFF;
        desc.StencilWriteMask = 0xFF;
        desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
        desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        desc.BackFace = desc.FrontFace;
        V( device->CreateDepthStencilState( &desc, &g_DSS_DepthDisabled_StencilCreateMask ) );

        desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        desc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
        desc.BackFace = desc.FrontFace;
        V( device->CreateDepthStencilState( &desc, &g_DSS_DepthDisabled_StencilUseMask ) );
    }

    {
        CD3D11_BLEND_DESC desc = CD3D11_BLEND_DESC( CD3D11_DEFAULT( ) );

        desc.RenderTarget[0].BlendEnable = false;

        V( device->CreateBlendState( &desc, &g_BS_Opaque ) );

        desc.RenderTarget[0].BlendEnable = true;
        desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

        desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
        desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
        desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
        V( device->CreateBlendState( &desc, &g_BS_Additive ) );

        desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
        desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
        V( device->CreateBlendState( &desc, &g_BS_AlphaBlend ) );

        desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
        desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
        desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
        V( device->CreateBlendState( &desc, &g_BS_PremultAlphaBlend ) );

        desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
        desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
        desc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_COLOR;
        desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_SRC_ALPHA;
        V( device->CreateBlendState( &desc, &g_BS_Mult ) );

    }

    {
        D3D11_SUBRESOURCE_DATA data;
        uint8 whitePixel[4] = { 255, 255, 255, 255 };
        data.pSysMem = whitePixel;
        data.SysMemPitch = sizeof( whitePixel );
        data.SysMemSlicePitch = data.SysMemPitch;

        ID3D11Texture2D * texture2D = vaDirectXTools::CreateTexture2D( DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, &data );
        g_Texture2D_SRV_White1x1 = vaDirectXTools::CreateShaderResourceView( texture2D );

        vaDirectXCore::NameObject( texture2D, "g_Texture2D_SRV_White1x1" );
        SAFE_RELEASE( texture2D );
    }

    // samplers
    {
        CD3D11_SAMPLER_DESC desc = CD3D11_SAMPLER_DESC( CD3D11_DEFAULT( ) );

        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        device->CreateSamplerState( &desc, &g_samplerStatePointClamp );
        desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        device->CreateSamplerState( &desc, &g_samplerStatePointWrap );
        desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
        device->CreateSamplerState( &desc, &g_samplerStatePointMirror );

        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        device->CreateSamplerState( &desc, &g_samplerStateLinearClamp );
        desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        device->CreateSamplerState( &desc, &g_samplerStateLinearWrap );

        desc.Filter = D3D11_FILTER_ANISOTROPIC;
        desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        device->CreateSamplerState( &desc, &g_samplerStateAnisotropicClamp );
        desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        device->CreateSamplerState( &desc, &g_samplerStateAnisotropicWrap );

    }

    {
        //SAFE_RELEASE( m_noiseTexture );

        //V( GetDevice()->CreateTexture( m_noiseTextureResolution, m_noiseTextureResolution, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_noiseTexture, NULL) );

        //int levelCount = m_noiseTexture->GetLevelCount();
        //int resolution = m_noiseTextureResolution;

        //for( int level = 0; level < levelCount; level++ )
        //{
        //   D3DLOCKED_RECT noiseLockedRect; 
        //   V( m_noiseTexture->LockRect( level, &noiseLockedRect, NULL, 0 ) );

        //   unsigned int * texRow = (unsigned int *)noiseLockedRect.pBits;

        //   for( int y = 0; y < resolution; y++ )
        //   {
        //      for( int x = 0; x < resolution; x++ )
        //      {
        //         texRow[x] = (0xFF & (int)(randf() * 256.0f));
        //         texRow[x] |= (0xFF & (int)(randf() * 256.0f)) << 8;

        //         float ang = randf();
        //         float fx = sinf( ang * (float)PI * 2.0f ) * 0.5f + 0.5f;
        //         float fy = sinf( ang * (float)PI * 2.0f ) * 0.5f + 0.5f;

        //         texRow[x] |= (0xFF & (int)(fx * 256.0f)) << 16;
        //         texRow[x] |= (0xFF & (int)(fy * 256.0f)) << 24;
        //      }
        //      texRow += noiseLockedRect.Pitch / sizeof(*texRow);
        //   }
        //   V( m_noiseTexture->UnlockRect(level) );
        //   resolution /= 2;
        //vaDirectXCore::NameObject( texture2D, "g_Texture2D_SRV_White1x1" );
        //}
    }
}

void vaDirectXTools_OnDeviceDestroyed( )
{
    SAFE_RELEASE( g_RS_CullNone_Fill );
    SAFE_RELEASE( g_RS_CullCCW_Fill );
    SAFE_RELEASE( g_RS_CullCW_Fill );
    SAFE_RELEASE( g_RS_CullNone_Wireframe );
    SAFE_RELEASE( g_RS_CullCW_Wireframe );
    SAFE_RELEASE( g_RS_CullCCW_Wireframe );


    SAFE_RELEASE( g_DSS_DepthEnabledL_DepthWrite );
    SAFE_RELEASE( g_DSS_DepthEnabledG_DepthWrite );
    SAFE_RELEASE( g_DSS_DepthEnabledL_NoDepthWrite );
    SAFE_RELEASE( g_DSS_DepthEnabledLE_NoDepthWrite );
    SAFE_RELEASE( g_DSS_DepthEnabledG_NoDepthWrite );
    SAFE_RELEASE( g_DSS_DepthEnabledGE_NoDepthWrite );
    SAFE_RELEASE( g_DSS_DepthDisabled_NoDepthWrite );
    SAFE_RELEASE( g_DSS_DepthPassAlways_DepthWrite );
    SAFE_RELEASE( g_DSS_DepthDisabled_StencilCreateMask );
    SAFE_RELEASE( g_DSS_DepthDisabled_StencilUseMask );

    SAFE_RELEASE( g_BS_Opaque );
    SAFE_RELEASE( g_BS_Additive );
    SAFE_RELEASE( g_BS_AlphaBlend );
    SAFE_RELEASE( g_BS_PremultAlphaBlend );
    SAFE_RELEASE( g_BS_Mult );

    SAFE_RELEASE( g_Texture2D_SRV_White1x1 );
    SAFE_RELEASE( g_Texture2D_SRV_Noise2D );
    SAFE_RELEASE( g_Texture2D_SRV_Noise3D );

    SAFE_RELEASE( g_samplerStatePointClamp );
    SAFE_RELEASE( g_samplerStatePointWrap );
    SAFE_RELEASE( g_samplerStatePointMirror );
    SAFE_RELEASE( g_samplerStateLinearClamp );
    SAFE_RELEASE( g_samplerStateLinearWrap );
    SAFE_RELEASE( g_samplerStateAnisotropicClamp );
    SAFE_RELEASE( g_samplerStateAnisotropicWrap );

    SAFE_DELETE( g_rasterizerStatesLib );
}

ID3D11RasterizerState * vaDirectXTools::GetRS_CullNone_Fill( )
{
    return g_RS_CullNone_Fill;
}

ID3D11RasterizerState * vaDirectXTools::GetRS_CullCCW_Fill( )
{
    return g_RS_CullCCW_Fill;
}

ID3D11RasterizerState * vaDirectXTools::GetRS_CullCW_Fill( )
{
    return g_RS_CullCW_Fill;
}

ID3D11RasterizerState * vaDirectXTools::GetRS_CullNone_Wireframe( )
{
    return g_RS_CullNone_Wireframe;
}

ID3D11RasterizerState * vaDirectXTools::GetRS_CullCCW_Wireframe( )
{
    return g_RS_CullCCW_Wireframe;
}

ID3D11RasterizerState * vaDirectXTools::GetRS_CullCW_Wireframe( )
{
    return g_RS_CullCW_Wireframe;
}

ID3D11DepthStencilState * vaDirectXTools::GetDSS_DepthEnabledL_DepthWrite( )
{
    return g_DSS_DepthEnabledL_DepthWrite;
}

ID3D11DepthStencilState * vaDirectXTools::GetDSS_DepthEnabledG_DepthWrite( )
{
    return g_DSS_DepthEnabledG_DepthWrite;
}

ID3D11DepthStencilState * vaDirectXTools::GetDSS_DepthEnabledL_NoDepthWrite( )
{
    return g_DSS_DepthEnabledL_NoDepthWrite;
}

ID3D11DepthStencilState * vaDirectXTools::GetDSS_DepthEnabledLE_NoDepthWrite( )
{
    return g_DSS_DepthEnabledLE_NoDepthWrite;
}

ID3D11DepthStencilState * vaDirectXTools::GetDSS_DepthEnabledG_NoDepthWrite( )
{
    return g_DSS_DepthEnabledG_NoDepthWrite;
}

ID3D11DepthStencilState * vaDirectXTools::GetDSS_DepthEnabledGE_NoDepthWrite( )
{
    return g_DSS_DepthEnabledGE_NoDepthWrite;
}

ID3D11DepthStencilState * vaDirectXTools::GetDSS_DepthDisabled_NoDepthWrite( )
{
    return g_DSS_DepthDisabled_NoDepthWrite;
}

ID3D11DepthStencilState * vaDirectXTools::GetDSS_DepthPassAlways_DepthWrite( )
{
    return g_DSS_DepthPassAlways_DepthWrite;
}

ID3D11DepthStencilState * vaDirectXTools::GetDSS_DepthDisabled_NoDepthWrite_StencilCreateMask( )
{
    return g_DSS_DepthDisabled_StencilCreateMask;
}

ID3D11DepthStencilState * vaDirectXTools::GetDSS_DepthDisabled_NoDepthWrite_StencilUseMask( )
{
    return g_DSS_DepthDisabled_StencilUseMask;
}

ID3D11ShaderResourceView * vaDirectXTools::GetTexture2D_SRV_White1x1( )
{
    return g_Texture2D_SRV_White1x1;
}

ID3D11SamplerState * vaDirectXTools::GetSamplerStatePointClamp( ) { return g_samplerStatePointClamp; }
ID3D11SamplerState * vaDirectXTools::GetSamplerStatePointWrap( ) { return g_samplerStatePointWrap; }
ID3D11SamplerState * vaDirectXTools::GetSamplerStatePointMirror( ) { return g_samplerStatePointMirror; }
ID3D11SamplerState * vaDirectXTools::GetSamplerStateLinearClamp( ) { return g_samplerStateLinearClamp; }
ID3D11SamplerState * vaDirectXTools::GetSamplerStateLinearWrap( ) { return g_samplerStateLinearWrap; }
ID3D11SamplerState * vaDirectXTools::GetSamplerStateAnisotropicClamp( ) { return g_samplerStateAnisotropicClamp; }
ID3D11SamplerState * vaDirectXTools::GetSamplerStateAnisotropicWrap( ) { return g_samplerStateAnisotropicWrap; }

ID3D11SamplerState * * vaDirectXTools::GetSamplerStatePtrPointClamp( ) { return &g_samplerStatePointClamp; }
ID3D11SamplerState * * vaDirectXTools::GetSamplerStatePtrPointWrap( ) { return &g_samplerStatePointWrap; }
ID3D11SamplerState * * vaDirectXTools::GetSamplerStatePtrLinearClamp( ) { return &g_samplerStateLinearClamp; }
ID3D11SamplerState * * vaDirectXTools::GetSamplerStatePtrLinearWrap( ) { return &g_samplerStateLinearWrap; }
ID3D11SamplerState * * vaDirectXTools::GetSamplerStatePtrAnisotropicClamp( ) { return &g_samplerStateAnisotropicClamp; }
ID3D11SamplerState * * vaDirectXTools::GetSamplerStatePtrAnisotropicWrap( ) { return &g_samplerStateAnisotropicWrap; }

ID3D11RasterizerState * vaDirectXTools::CreateRasterizerState( const D3D11_RASTERIZER_DESC & desc )
{
    ID3D11RasterizerState * retVal = NULL;

    HRESULT hr;

    hr = vaDirectXCore::GetDevice( )->CreateRasterizerState( &desc, &retVal );

    assert( SUCCEEDED( hr ) );
    if( !SUCCEEDED( hr ) )
    {
        return NULL;
    }

    return retVal;
}

ID3D11RasterizerState * vaDirectXTools_RasterizerStatesLib::FindOrCreateRasterizerState( const D3D11_RASTERIZER_DESC & desc )
{
    auto it = m_items.find( desc );
    if( it != m_items.end( ) )
    {
        return it->second;
    }
    else
    {
        ID3D11RasterizerState * newRS = vaDirectXTools::CreateRasterizerState( desc );
        m_items.insert( std::make_pair( desc, newRS ) );
        return newRS;
    }
}

ID3D11RasterizerState * vaDirectXTools::FindOrCreateRasterizerState( const D3D11_RASTERIZER_DESC & desc )
{
    assert( g_rasterizerStatesLib != nullptr );
    return g_rasterizerStatesLib->FindOrCreateRasterizerState( desc );
}

ID3D11BlendState * vaDirectXTools::GetBS_Opaque( )
{
    return g_BS_Opaque;
}

ID3D11BlendState * vaDirectXTools::GetBS_Additive( )
{
    return g_BS_Additive;
}

ID3D11BlendState * vaDirectXTools::GetBS_AlphaBlend( )
{
    return g_BS_AlphaBlend;
}

ID3D11BlendState * vaDirectXTools::GetBS_PremultAlphaBlend( )
{
    return g_BS_PremultAlphaBlend;
}

ID3D11BlendState * vaDirectXTools::GetBS_Mult( )
{
    return g_BS_Mult;
}

ID3D11ShaderResourceView * vaDirectXTools::CreateShaderResourceView( ID3D11Resource * texture, DXGI_FORMAT format, int mipSlice, int arraySlice )
{
    ID3D11ShaderResourceView * ret = NULL;

    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    bool descInitialized = false;

    {
        D3D11_SRV_DIMENSION dimension = D3D11_SRV_DIMENSION_UNKNOWN;

        if( !descInitialized )
        {
            ID3D11Texture2D * texture2D = NULL;
            if( SUCCEEDED( texture->QueryInterface( IID_ID3D11Texture2D, (void**)&texture2D ) ) )
            {
                D3D11_TEXTURE2D_DESC descTex2D;
                texture2D->GetDesc( &descTex2D );
                dimension = ( descTex2D.ArraySize == 1 ) ? ( D3D11_SRV_DIMENSION_TEXTURE2D ) : ( D3D11_SRV_DIMENSION_TEXTURE2DARRAY );
                desc = CD3D11_SHADER_RESOURCE_VIEW_DESC( texture2D, dimension, format );
                descInitialized = true;
                SAFE_RELEASE( texture2D );

                if( mipSlice != -1 )
                {
                    if( dimension == D3D11_SRV_DIMENSION_TEXTURE2D )
                    {
                        desc.Texture2D.MipLevels = 1;
                        desc.Texture2D.MostDetailedMip = mipSlice;
                    }
                    else
                    {
                        desc.Texture2DArray.MipLevels = 1;
                        desc.Texture2DArray.MostDetailedMip = mipSlice;
                        //assert( false ); //not implemented
                    }
                }
                if( arraySlice != -1 )
                {
                    if( dimension == D3D11_SRV_DIMENSION_TEXTURE2DARRAY )
                    {
                        assert( false );
                        //desc.Texture2D.MipLevels = 1;
                        //desc.Texture2D.MostDetailedMip = mipSlice;
                    }
                    else
                    {
                        assert( false ); //not implemented
                    }
                }
            }
        }

        if( !descInitialized )
        {
            ID3D11Texture3D * texture3D = NULL;
            if( SUCCEEDED( texture->QueryInterface( IID_ID3D11Texture3D, (void**)&texture3D ) ) )
            {
                D3D11_TEXTURE3D_DESC descTex3D;
                texture3D->GetDesc( &descTex3D );
                dimension = D3D11_SRV_DIMENSION_TEXTURE3D;
                desc = CD3D11_SHADER_RESOURCE_VIEW_DESC( texture3D, format );
                descInitialized = true;
                SAFE_RELEASE( texture3D );
                if( mipSlice != -1 )
                {
                    assert( false ); //not implemented
                }
            }
        }

        if( !descInitialized )
        {
            ID3D11Texture1D * texture1D = NULL;
            if( SUCCEEDED( texture->QueryInterface( IID_ID3D11Texture1D, (void**)&texture1D ) ) )
            {
                D3D11_TEXTURE1D_DESC descTex1D;
                texture1D->GetDesc( &descTex1D );
                dimension = ( descTex1D.ArraySize == 1 ) ? ( D3D11_SRV_DIMENSION_TEXTURE1D ) : ( D3D11_SRV_DIMENSION_TEXTURE1DARRAY );
                desc = CD3D11_SHADER_RESOURCE_VIEW_DESC( texture1D, dimension, format );
                descInitialized = true;
                SAFE_RELEASE( texture1D );

                if( mipSlice != -1 )
                {
                    if( dimension == D3D11_SRV_DIMENSION_TEXTURE1D )
                    {
                        desc.Texture1D.MipLevels = 1;
                        desc.Texture1D.MostDetailedMip = mipSlice;
                    }
                    else
                    {
                        assert( false ); //not implemented
                    }
                }
            }
        }
    }

    if( !descInitialized )
    {
        assert( false ); // resource not recognized; additional code might be needed above
        return NULL;
    }


    if( SUCCEEDED( vaDirectXCore::GetDevice( )->CreateShaderResourceView( texture, &desc, &ret ) ) )
    {
        return ret;
    }
    else
    {
        assert( false );
        return NULL;
    }
}

ID3D11DepthStencilView * vaDirectXTools::CreateDepthStencilView( ID3D11Resource * texture, DXGI_FORMAT format )
{
    ID3D11DepthStencilView * ret = NULL;

    D3D11_DEPTH_STENCIL_VIEW_DESC desc;
    bool descInitialized = false;

    {
        D3D11_DSV_DIMENSION dimension = D3D11_DSV_DIMENSION_UNKNOWN;
        ID3D11Texture2D * texture2D = NULL;

        if( SUCCEEDED( texture->QueryInterface( IID_ID3D11Texture2D, (void**)&texture2D ) ) )
        {
            D3D11_TEXTURE2D_DESC descTex2D;
            texture2D->GetDesc( &descTex2D );
            dimension = ( descTex2D.ArraySize == 1 ) ? ( D3D11_DSV_DIMENSION_TEXTURE2D ) : ( D3D11_DSV_DIMENSION_TEXTURE2DARRAY );
            desc = CD3D11_DEPTH_STENCIL_VIEW_DESC( texture2D, dimension, format );
            descInitialized = true;
            SAFE_RELEASE( texture2D );
        }
    }

    if( !descInitialized )
    {
        assert( false ); // resource not recognized; additional code might be needed above
        return NULL;
    }


    if( SUCCEEDED( vaDirectXCore::GetDevice( )->CreateDepthStencilView( texture, &desc, &ret ) ) )
    {
        return ret;
    }
    else
    {
        assert( false );
        return NULL;
    }
}

ID3D11RenderTargetView * vaDirectXTools::CreateRenderTargetView( ID3D11Resource * resource, DXGI_FORMAT format, int mipSlice, int arraySlice )
{
    ID3D11RenderTargetView * ret = NULL;

    D3D11_RENDER_TARGET_VIEW_DESC desc;
    bool descInitialized = false;

    // there's no way to bind multiple mip slices at once, so -1 means slice 0
    if( mipSlice == -1 )
        mipSlice = 0;

    {
        D3D11_RTV_DIMENSION dimension = D3D11_RTV_DIMENSION_UNKNOWN;

        ID3D11Texture2D * texture2D = NULL;
        if( SUCCEEDED( resource->QueryInterface( IID_ID3D11Texture2D, (void**)&texture2D ) ) )
        {
            D3D11_TEXTURE2D_DESC descTex2D;
            texture2D->GetDesc( &descTex2D );
            if( arraySlice == -1 )
            {
                // if array, bind the whole array
                dimension = ( descTex2D.ArraySize == 1 ) ? ( D3D11_RTV_DIMENSION_TEXTURE2D ) : ( D3D11_RTV_DIMENSION_TEXTURE2DARRAY );
                desc = CD3D11_RENDER_TARGET_VIEW_DESC( texture2D, dimension, format );

                if( dimension == D3D11_RTV_DIMENSION_TEXTURE2D )
                {
                    desc.Texture2D.MipSlice = mipSlice;
                }
                else
                {
                    desc.Texture2DArray.MipSlice = mipSlice;
                    assert( desc.Texture2DArray.ArraySize == descTex2D.ArraySize );
                }
            }
            else
            {
                // if array but specific slice requested, bind specific slice
                assert( arraySlice < (int)descTex2D.ArraySize );
                dimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                desc = CD3D11_RENDER_TARGET_VIEW_DESC( texture2D, dimension, format );
                desc.Texture2DArray.MipSlice        = mipSlice;
                desc.Texture2DArray.FirstArraySlice = arraySlice;
                desc.Texture2DArray.ArraySize       = 1;            // size > 1 when selecting specific slice not supported at the moment
            }

            descInitialized = true;
            SAFE_RELEASE( texture2D );
        }

        ID3D11Texture3D * texture3D = NULL;
        if( SUCCEEDED( resource->QueryInterface( IID_ID3D11Texture3D, (void**)&texture3D ) ) )
        {
            D3D11_TEXTURE3D_DESC descTex3D;
            texture3D->GetDesc( &descTex3D );
            desc = CD3D11_RENDER_TARGET_VIEW_DESC( texture3D, format );
            desc.Texture3D.MipSlice = mipSlice;
            descInitialized = true;
            SAFE_RELEASE( texture3D );
        }
    }

    if( !descInitialized )
    {
        assert( false ); // resource not recognized; additional code might be needed above
        return NULL;
    }

    if( SUCCEEDED( vaDirectXCore::GetDevice( )->CreateRenderTargetView( resource, &desc, &ret ) ) )
    {
        return ret;
    }
    else
    {
        assert( false );
        return NULL;
    }
}

ID3D11UnorderedAccessView * vaDirectXTools::CreateUnorderedAccessView( ID3D11Resource * resource, DXGI_FORMAT format, int mipSlice, int arraySlice )
{
    ID3D11UnorderedAccessView * ret = NULL;

    D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
    bool descInitialized = false;

    // there's no way to bind multiple mip slices at once, so -1 means slice 0
    if( mipSlice == -1 )
        mipSlice = 0;

    {
        D3D11_UAV_DIMENSION dimension = D3D11_UAV_DIMENSION_UNKNOWN;

        if( !descInitialized )
        {
            ID3D11Texture2D * texture2D = NULL;
            if( SUCCEEDED( resource->QueryInterface( IID_ID3D11Texture2D, (void**)&texture2D ) ) )
            {
                D3D11_TEXTURE2D_DESC descTex2D;
                texture2D->GetDesc( &descTex2D );

                if( arraySlice == -1 )
                {
                    // if array, bind the whole array
                    dimension = ( descTex2D.ArraySize == 1 ) ? ( D3D11_UAV_DIMENSION_TEXTURE2D ) : ( D3D11_UAV_DIMENSION_TEXTURE2DARRAY );
                    desc = CD3D11_UNORDERED_ACCESS_VIEW_DESC( texture2D, dimension, format );

                    if( dimension == D3D11_UAV_DIMENSION_TEXTURE2D )
                    {
                        desc.Texture2D.MipSlice = mipSlice;
                    }
                    else
                    {
                        desc.Texture2DArray.MipSlice = mipSlice;
                        assert( desc.Texture2DArray.ArraySize == descTex2D.ArraySize );
                    }
                }
                else
                {
                    // if array but specific slice requested, bind specific slice
                    assert( arraySlice < (int)descTex2D.ArraySize );
                    dimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
                    desc = CD3D11_UNORDERED_ACCESS_VIEW_DESC( texture2D, dimension, format );
                    desc.Texture2DArray.MipSlice = mipSlice;
                    desc.Texture2DArray.FirstArraySlice = arraySlice;
                    desc.Texture2DArray.ArraySize = 1;            // size > 1 when selecting specific slice not supported at the moment
                }
                descInitialized = true;
                SAFE_RELEASE( texture2D );
            }
        }

        if( !descInitialized )
        {
            ID3D11Texture3D * texture3D = NULL;
            if( SUCCEEDED( resource->QueryInterface( IID_ID3D11Texture3D, (void**)&texture3D ) ) )
            {
                D3D11_TEXTURE3D_DESC descTex3D;
                texture3D->GetDesc( &descTex3D );
                dimension = D3D11_UAV_DIMENSION_TEXTURE3D;
                desc = CD3D11_UNORDERED_ACCESS_VIEW_DESC( texture3D, format );
                descInitialized = true;
                SAFE_RELEASE( texture3D );
            }
        }

        if( !descInitialized )
        {
            ID3D11Texture1D * texture1D = NULL;
            if( SUCCEEDED( resource->QueryInterface( IID_ID3D11Texture1D, (void**)&texture1D ) ) )
            {
                D3D11_TEXTURE1D_DESC descTex1D;
                texture1D->GetDesc( &descTex1D );
                dimension = ( descTex1D.ArraySize == 1 ) ? ( D3D11_UAV_DIMENSION_TEXTURE1D ) : ( D3D11_UAV_DIMENSION_TEXTURE1DARRAY );
                desc = CD3D11_UNORDERED_ACCESS_VIEW_DESC( texture1D, dimension, format );
                descInitialized = true;
                SAFE_RELEASE( texture1D );
            }
        }
    }

    if( !descInitialized )
    {
        assert( false ); // resource not recognized; additional code might be needed above
        return NULL;
    }

    if( SUCCEEDED( vaDirectXCore::GetDevice( )->CreateUnorderedAccessView( resource, &desc, &ret ) ) )
    {
        return ret;
    }
    else
    {
        assert( false );
        return NULL;
    }
}

ID3D11Texture3D * vaDirectXTools::CreateTexture3D( DXGI_FORMAT format, UINT width, UINT height, UINT depth, D3D11_SUBRESOURCE_DATA * initialData, UINT mipLevels,
    UINT bindFlags, D3D11_USAGE usage, UINT cpuaccessFlags, UINT miscFlags )
{
    CD3D11_TEXTURE3D_DESC desc( format, width, height, depth, mipLevels, bindFlags, usage, cpuaccessFlags, miscFlags );

    ID3D11Texture3D * texture3D = NULL;

    if( SUCCEEDED( vaDirectXCore::GetDevice( )->CreateTexture3D( &desc, initialData, &texture3D ) ) )
    {
        //vaDirectXCore::NameObject( texture2D, "vaDirectXTools::CreateTexture3D" );
        return texture3D;
    }
    else
    {
        assert( false );
        return NULL;
    }

}

ID3D11Texture2D * vaDirectXTools::CreateTexture2D( DXGI_FORMAT format, UINT width, UINT height, D3D11_SUBRESOURCE_DATA * initialData, UINT arraySize, UINT mipLevels,
    UINT bindFlags, D3D11_USAGE usage, UINT cpuaccessFlags, UINT sampleCount, UINT sampleQuality, UINT miscFlags )
{
    CD3D11_TEXTURE2D_DESC desc( format, width, height, arraySize, mipLevels, bindFlags, usage, cpuaccessFlags, sampleCount, sampleQuality, miscFlags );

    ID3D11Texture2D * texture2D = NULL;

    if( SUCCEEDED( vaDirectXCore::GetDevice( )->CreateTexture2D( &desc, initialData, &texture2D ) ) )
    {
        //vaDirectXCore::NameObject( texture2D, "vaDirectXTools::CreateTexture2D" );
        return texture2D;
    }
    else
    {
        assert( false );
        return NULL;
    }

}

ID3D11Texture1D * vaDirectXTools::CreateTexture1D( DXGI_FORMAT format, UINT width, D3D11_SUBRESOURCE_DATA * initialData, UINT arraySize, UINT mipLevels,
    UINT bindFlags, D3D11_USAGE usage, UINT cpuaccessFlags, UINT miscFlags )
{
    CD3D11_TEXTURE1D_DESC desc( format, width, arraySize, mipLevels, bindFlags, usage, cpuaccessFlags, miscFlags );

    ID3D11Texture1D * texture1D = NULL;

    if( SUCCEEDED( vaDirectXCore::GetDevice( )->CreateTexture1D( &desc, initialData, &texture1D ) ) )
    {
        //vaDirectXCore::NameObject( texture1D, "vaDirectXTools::CreateTexture2D" );
        return texture1D;
    }
    else
    {
        assert( false );
        return NULL;
    }

}

int vaDirectXTools::CalcApproxTextureSizeInMemory( DXGI_FORMAT format, int width, int height, int mipCount )
{
    // does this work?
    assert( false );

    int pixels = 0;
    int tw = width, th = height;
    for( int i = 0; i < mipCount; i++ )
    {
        pixels += tw * th;
        tw /= 2; th /= 2;
    }

    return pixels * (int)DirectX::BitsPerPixel( format );
}

ID3D11Resource * vaDirectXTools::LoadTextureDDS( void * dataBuffer, int64 dataSize, bool assumeSourceIsInSRGB, bool dontAutogenerateMIPs, uint64 * outCRC )
{
    ID3D11Resource * texture = NULL;

    // not actually used, needed internally for mipmap generation
    ID3D11ShaderResourceView * textureSRV = NULL;

    HRESULT hr;
    if( dontAutogenerateMIPs )
    {
        // todo: cleanup dontAutogenerateMIPs hack from DirectX::CreateDDSTextureFromMemoryEx
        hr = DirectX::CreateDDSTextureFromMemoryEx( vaDirectXCore::GetDevice( ), (const uint8_t *)dataBuffer, (size_t)dataSize, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, assumeSourceIsInSRGB, dontAutogenerateMIPs, &texture, &textureSRV, NULL );
    }
    else
    {
        // todo: cleanup dontAutogenerateMIPs hack from DirectX::CreateDDSTextureFromMemoryEx
        hr = DirectX::CreateDDSTextureFromMemoryEx( vaDirectXCore::GetDevice( ), vaDirectXCore::GetImmediateContext( ), (const uint8_t *)dataBuffer, (size_t)dataSize, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, assumeSourceIsInSRGB, dontAutogenerateMIPs, &texture, &textureSRV, NULL );
    }

    if( SUCCEEDED( hr ) )
    {
        SAFE_RELEASE( textureSRV );
        return texture;
    }
    else
    {
        SAFE_RELEASE( texture );
        SAFE_RELEASE( textureSRV );
        assert( false ); // check hr
        throw "Error creating the texture from the stream";
    }
}

ID3D11Resource * vaDirectXTools::LoadTextureDDS( const wchar_t * path, bool assumeSourceIsInSRGB, bool dontAutogenerateMIPs, uint64 * outCRC )
{
    auto buffer = vaFileTools::LoadFileToMemoryStream( path );

    if( buffer == nullptr )
        return nullptr;

    return LoadTextureDDS( buffer->GetBuffer( ), buffer->GetLength( ), assumeSourceIsInSRGB, dontAutogenerateMIPs, outCRC );
}

ID3D11Resource * vaDirectXTools::LoadTextureWIC( void * dataBuffer, int64 dataSize, bool assumeSourceIsInSRGB, bool dontAutogenerateMIPs, uint64 * outCRC )
{
    ID3D11Resource * texture = NULL;

    // not actually used, needed internally for mipmap generation
    ID3D11ShaderResourceView * textureSRV = NULL;

    HRESULT hr;

    if( dontAutogenerateMIPs )
    {
        hr = DirectX::CreateWICTextureFromMemoryEx( vaDirectXCore::GetDevice( ), (const uint8_t *)dataBuffer, (size_t)dataSize, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, assumeSourceIsInSRGB, &texture, &textureSRV );
    }
    else
    {
        hr = DirectX::CreateWICTextureFromMemoryEx( vaDirectXCore::GetDevice( ), vaDirectXCore::GetImmediateContext( ), (const uint8_t *)dataBuffer, (size_t)dataSize, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, assumeSourceIsInSRGB, &texture, &textureSRV );
    }

    if( SUCCEEDED( hr ) )
    {
        SAFE_RELEASE( textureSRV );
        return texture;
    }
    else
    {
        SAFE_RELEASE( texture );
        SAFE_RELEASE( textureSRV );
        VA_LOG_ERROR_STACKINFO( L"Error loading texture" );
        return nullptr;
    }
}

ID3D11Resource * vaDirectXTools::LoadTextureWIC( const wchar_t * path, bool assumeSourceIsInSRGB, bool dontAutogenerateMIPs, uint64 * outCRC )
{
    //auto buffer = vaFileTools::LoadFileToMemoryStream( path );
    //
    //if( buffer == nullptr )
    //    return nullptr;
    //
    //return LoadTextureWIC( path, assumeSourceIsInSRGB, dontAutogenerateMIPs, outCRC );

    ID3D11Resource * texture = NULL;

    // not actually used, needed internally for mipmap generation
    ID3D11ShaderResourceView * textureSRV = NULL;

    HRESULT hr;

    if( dontAutogenerateMIPs )
    {
        hr = DirectX::CreateWICTextureFromFileEx( vaDirectXCore::GetDevice( ), path, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, assumeSourceIsInSRGB, &texture, &textureSRV );
    }
    else
    {
        hr = DirectX::CreateWICTextureFromFileEx( vaDirectXCore::GetDevice( ), vaDirectXCore::GetImmediateContext( ), path, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, assumeSourceIsInSRGB, &texture, &textureSRV );
    }

    if( SUCCEEDED( hr ) )
    {
        SAFE_RELEASE( textureSRV );
        return texture;
    }
    else
    {
        SAFE_RELEASE( texture );
        SAFE_RELEASE( textureSRV );
        VA_LOG_ERROR_STACKINFO( L"Error loading texture '%s'", path );
        return nullptr;
    }
}
// ID3D11Texture2D * vaDirectXTools::LoadTexture2D( void * buffer, int64 bufferSize, bool assumeSourceIsInSRGB, bool dontAutogenerateMIPs, uint64 * outCRC )
// {
//     ID3D11Resource * texRes = LoadTexture( buffer, bufferSize, assumeSourceIsInSRGB, dontAutogenerateMIPs, outCRC );
// 
//     ID3D11Texture2D * tex2D = QueryResourceInterface<ID3D11Texture2D>( texRes, IID_ID3D11Texture2D );
// 
//     SAFE_RELEASE( texRes );
// 
//     return tex2D;
// }
// 
// ID3D11Texture2D * vaDirectXTools::LoadTexture2D( const wchar_t * path, bool assumeSourceIsInSRGB, bool dontAutogenerateMIPs, uint64 * outCRC )
// {
//     ID3D11Resource * texRes = LoadTexture( path, assumeSourceIsInSRGB, dontAutogenerateMIPs, outCRC );
// 
//     ID3D11Texture2D * tex2D = QueryResourceInterface<ID3D11Texture2D>( texRes, IID_ID3D11Texture2D );
// 
//     SAFE_RELEASE( texRes );
// 
//     return tex2D;
// }

//ID3D11ShaderResourceView * vaDirectXTools::LoadTextureSRV( const wchar_t * path, bool assumeSourceIsInSRGB, bool dontAutogenerateMIPs, uint64 * outCRC )
//{
//    //ID3D11ShaderResourceView * textureSRV = NULL;
//    ID3D11Resource * texture = LoadTexture( path, assumeSourceIsInSRGB, dontAutogenerateMIPs, outCRC );
//    if( texture == NULL ) return NULL;
//
//    ID3D11ShaderResourceView * srv = CreateShaderResourceView( texture );
//    vaDirectXCore::NameObject( texture, "vaDirectXTools::LoadTextureSRV" );
//    SAFE_RELEASE( texture );
//
//    return srv;
//}

void vaDirectXTools::RenderProfiler( int x, int y, int width, int height, vaSimpleProfiler * profiler )
{
    /*
    const vaSimpleProfiler::EntryContainerType & cont = profiler->GetEntries();

    ICanvas2D* canvas2D = vaCanvas2DGetGlobalInstance();

    canvas2D->FillRectangle( (float)x, (float)y, (float)width, (float)height, 0x80FFFFFF );
    canvas2D->DrawRectangle( (float)x, (float)y, (float)width, (float)height, 0xFF000000 );

    const int lineHeight = 19;
    int y0 = y + 4;
    int x0 = x + 2;

    int x1 = x + width - 128;

    canvas2D->DrawLine( (float)x1, (float)y+1, (float)x1, (float)y+height-1, 0xFF808080 );

    for( int i = 0; i < (int)cont.size(); i++ )
    {
       const vaSimpleProfilerEntry & entry = *cont[i];

       double time = entry.GetLastAverageTimePerFrame();

       canvas2D->DrawString( x0, y0, 0xFF000000, 0x00000000, " %s ", entry.GetName() );
       canvas2D->DrawString( x1, y0, 0xFF400000, 0x00000000, " %Lf ", entry.GetLastAverageTimePerFrame() );

       canvas2D->DrawLine( (float)x0, (float)y0 + lineHeight - 1, (float)x0+width-2, (float)y0 + lineHeight - 1, 0xFF202020 );

       y0 += lineHeight;

       //canvas2D->F( (float)x, (float)y, (float)width, (float)height, 0xFF000000 );
    }
    */
}

ID3D11Buffer * vaDirectXTools::CreateBuffer( uint32 sizeInBytes, uint32 bindFlags, D3D11_USAGE usage, uint32 cpuAccessFlags, uint32 miscFlags, uint32 structByteStride, const void * initializeData )
{
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = sizeInBytes;
    desc.Usage = usage;
    desc.BindFlags = bindFlags;
    desc.CPUAccessFlags = cpuAccessFlags;
    desc.MiscFlags = miscFlags;
    desc.StructureByteStride = structByteStride;
    HRESULT hr;
    ID3D11Buffer * buffer = NULL;
    if( initializeData == NULL )
    {
        V( vaDirectXCore::GetDevice( )->CreateBuffer( &desc, NULL, &buffer ) );
    }
    else
    {
        D3D11_SUBRESOURCE_DATA initSubresData;
        initSubresData.pSysMem = initializeData;
        initSubresData.SysMemPitch = sizeInBytes;
        initSubresData.SysMemSlicePitch = sizeInBytes;
        V( vaDirectXCore::GetDevice( )->CreateBuffer( &desc, &initSubresData, &buffer ) );
    }
    if( FAILED( hr ) )
        return NULL;
    vaDirectXCore::NameObject( buffer, "vaDirectXTools::CreateBuffer" );
    return buffer;
}

/*
bool vaDirectXTools::SaveResource( VertexAsylum::vaStream & outStream, ID3D11Resource * d3dResource )
{
    D3D11_RESOURCE_DIMENSION dim;
    d3dResource->GetType( &dim );

    if( ( dim == D3D11_RESOURCE_DIMENSION_UNKNOWN )
        || ( dim == D3D11_RESOURCE_DIMENSION_TEXTURE1D ) || ( dim == D3D11_RESOURCE_DIMENSION_TEXTURE3D ) )
    {
        assert( false ); // not supported!
        return false;
    }

    // Write header
    outStream.WriteValue<int32>( 0 );  // version
    outStream.WriteValue<int32>( (int32)dim );
    int64 sizePlaceholder = outStream.GetPosition( );
    outStream.WriteValue<int64>( 0 );  // size placeholder

    if( dim == D3D11_RESOURCE_DIMENSION_TEXTURE2D )
    {
        assert( false );

        //ID3D11Texture2D * tex2D = QueryResourceInterface<ID3D11Texture2D>( d3dResource, IID_ID3D11Texture2D );
        //assert( tex2D != NULL );

        //D3D11_TEXTURE2D_DESC desc;
        //tex2D->GetDesc(&desc);
        //outStream.WriteValue<D3D11_TEXTURE2D_DESC>( desc );

        //{
        //    desc.Usage = D3D11_USAGE_STAGING;
        //    desc.BindFlags = 0; // bindFlags;
        //    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ; // cpuAccessFlags;
        //    desc.MiscFlags = 0; // miscFlags;
        //    //desc.StructureByteStride   = structByteStride;

        //    HRESULT hr;
        //    ID3D11Texture2D * tempTexture = NULL;
        //    V( vaDirectXCore::GetDevice( )->CreateTexture2D( &desc, NULL, &tempTexture ) );
        //    if( FAILED( hr ) )
        //    {
        //        return false;
        //    }

        //    vaDirectXCore::GetImmediateContext( )->CopyResource( tempTexture, tex2D );

        //    assert( desc.ArraySize == 1 ); // not tested for more

        //    int subResCount = desc.MipLevels * desc.ArraySize;

        //    for( int subResIndex = 0; subResIndex < subResCount; subResIndex++ )
        //    {
        //        D3D11_MAPPED_SUBRESOURCE mapSubres;
        //        V( vaDirectXCore::GetImmediateContext( )->Map( tempTexture, 1, D3D11_MAP_READ, 0, &mapSubres ) );
        //        if( FAILED( hr ) )
        //        {
        //            SAFE_RELEASE( tempTexture );
        //            return false;
        //        }

        //        if( !outStream.Write( mapSubres.pData,  ) )
        //        {
        //            SAFE_RELEASE( tempTexture );
        //            return false;
        //        }
        //    }

        //    vaDirectXCore::GetImmediateContext( )->Unmap( tempTexture, 0 );
        //    //return buffer;
        //    SAFE_RELEASE( tempTexture );
        //}

        //SAFE_RELEASE( tex2D );
    }
    else if( dim == D3D11_RESOURCE_DIMENSION_BUFFER )
    {
        ID3D11Buffer * buffer = QueryResourceInterface<ID3D11Buffer>( d3dResource, IID_ID3D11Buffer );
        assert( buffer != NULL );

        D3D11_BUFFER_DESC desc;
        buffer->GetDesc( &desc );

        outStream.WriteValue<D3D11_BUFFER_DESC>( desc );

        {
            //D3D11_BUFFER_DESC desc;
            //desc.ByteWidth             = sizeInBytes;
            desc.Usage = D3D11_USAGE_STAGING;
            desc.BindFlags = 0; // bindFlags;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ; // cpuAccessFlags;
            desc.MiscFlags = 0; // miscFlags;
            //desc.StructureByteStride   = structByteStride;
            HRESULT hr;
            ID3D11Buffer * tempBuffer = NULL;
            V( vaDirectXCore::GetDevice( )->CreateBuffer( &desc, NULL, &tempBuffer ) );
            if( FAILED( hr ) )
            {
                return false;
            }

            vaDirectXCore::GetImmediateContext( )->CopyResource( tempBuffer, buffer );

            D3D11_MAPPED_SUBRESOURCE mapSubres;
            V( vaDirectXCore::GetImmediateContext( )->Map( tempBuffer, 0, D3D11_MAP_READ, 0, &mapSubres ) );
            if( FAILED( hr ) )
            {
                SAFE_RELEASE( tempBuffer );
                return false;
            }

            if( !outStream.Write( mapSubres.pData, desc.ByteWidth ) )
            {
                SAFE_RELEASE( tempBuffer );
                return false;
            }

            vaDirectXCore::GetImmediateContext( )->Unmap( tempBuffer, 0 );
            //return buffer;
            SAFE_RELEASE( tempBuffer );
        }

        SAFE_RELEASE( buffer );
    }

    // Write data size to header
    int64 endPos = outStream.GetPosition( );
    outStream.Seek( sizePlaceholder );
    outStream.WriteValue<int64>( endPos - sizePlaceholder - sizeof( int64 ) );
    outStream.Seek( endPos );

    return true;
}

ID3D11Resource * vaDirectXTools::LoadResource( VertexAsylum::vaStream & inStream, uint64 * outCRC )
{
    assert( outCRC == NULL ); // not implemented

    int version;
    if( !inStream.ReadValue<int32>( version ) ) return NULL;

    assert( version == 0 );

    D3D11_RESOURCE_DIMENSION dim;
    if( !inStream.ReadValue<D3D11_RESOURCE_DIMENSION>( dim ) ) return NULL;

    int64 dataSize;
    if( !inStream.ReadValue<int64>( dataSize ) ) return NULL;

    int64 endPos = inStream.GetPosition( ) + dataSize;

    ID3D11Resource * retValue = NULL;

    char * dataBuffer = NULL;
    try
    {
        if( version != 0 )
        {
            assert( false ); // version not supported!
            throw "version not supported";
        }

        if( ( dim == D3D11_RESOURCE_DIMENSION_UNKNOWN )
            || ( dim == D3D11_RESOURCE_DIMENSION_TEXTURE1D ) || ( dim == D3D11_RESOURCE_DIMENSION_TEXTURE3D ) )
        {
            assert( false ); // not supported!
            throw "type not supported";
        }

        assert( dataSize < ( 1 << 30 ) );

        if( dim == D3D11_RESOURCE_DIMENSION_TEXTURE2D )
        {
            dataBuffer = new char[(uint32)dataSize];
            if( !inStream.Read( dataBuffer, (int32)dataSize ) )
            {
                assert( false );
                throw "problem with reading the stream";
            }

            ID3D11Resource * texture = NULL;

            HRESULT hr;
            if( SUCCEEDED( hr = DirectX::CreateDDSTextureFromMemory( vaDirectXCore::GetDevice( ), (const uint8_t *)dataBuffer, (size_t)dataSize, &texture, NULL ) ) )
            {
                retValue = texture;
            }
            else
            {
                assert( false ); // check hr
                throw "Error creating the texture from the stream";
            }
        }
        else if( dim == D3D11_RESOURCE_DIMENSION_BUFFER )
        {
            D3D11_BUFFER_DESC desc;

            if( !inStream.ReadValue<D3D11_BUFFER_DESC>( desc ) )
            {
                assert( false );
                throw "Error creating the texture from the stream";
            }

            dataBuffer = new char[desc.ByteWidth];

            if( !inStream.Read( dataBuffer, desc.ByteWidth ) )
            {
                assert( false );
                throw "Error creating the texture from the stream";
            }

            ID3D11Buffer * outBuffer = NULL;
            D3D11_SUBRESOURCE_DATA xData;
            xData.pSysMem = dataBuffer;
            xData.SysMemPitch = desc.ByteWidth;
            xData.SysMemSlicePitch = desc.ByteWidth;

            HRESULT hr;
            V( vaDirectXCore::GetDevice( )->CreateBuffer( &desc, &xData, &outBuffer ) );
            if( FAILED( hr ) )
            {
                return false;
            }
            retValue = outBuffer;
        }

    }
    catch( const char * )
    {
        assert( retValue == NULL );
        inStream.Seek( endPos );
    }

    if( dataBuffer != NULL )
    {
        delete[] dataBuffer;
        dataBuffer = NULL;
    }

    if( retValue != NULL )
    {
        vaDirectXCore::NameObject( retValue, "vaDirectXTools::LoadResource" );
    }

    return retValue;
}
*/

void vaDirectXTools::CopyDepthStencil( ID3D11DeviceContext * destContext, ID3D11DepthStencilView * dsvSrc )
{
    ID3D11DepthStencilView * dsvDest;
    destContext->OMGetRenderTargets( 0, NULL, &dsvDest );
    ID3D11Resource * resSrc;
    ID3D11Resource * resDest;
    dsvSrc->GetResource( &resSrc );
    dsvDest->GetResource( &resDest );
    destContext->CopyResource( resDest, resSrc );
    SAFE_RELEASE( dsvDest );
}

void vaDirectXTools::ClearColorDepthStencil( ID3D11DeviceContext * destContext, bool clearAllColorRTs, bool clearDepth, bool clearStencil, const vaVector4 & clearColor, float depth, uint8 stencil )
{
    float clearColorF[4] = { clearColor.x, clearColor.y, clearColor.z, clearColor.w };
    ID3D11RenderTargetView * RTVs[4];
    ID3D11DepthStencilView * DSV;
    destContext->OMGetRenderTargets( _countof( RTVs ), RTVs, &DSV );

    for( int i = 0; i < _countof( RTVs ); i++ )
    {
        if( RTVs[i] == NULL ) continue;
        if( clearAllColorRTs )
            destContext->ClearRenderTargetView( RTVs[i], clearColorF );
        SAFE_RELEASE( RTVs[i] );
    }
    if( clearDepth || clearStencil )
    {
        UINT flags = 0;
        if( clearDepth )     flags |= D3D11_CLEAR_DEPTH;
        if( clearStencil )   flags |= D3D11_CLEAR_STENCIL;
        destContext->ClearDepthStencilView( DSV, flags, depth, stencil );
    }
    SAFE_RELEASE( DSV );
}


//--------------------------------------------------------------------------------------
bool vaDirectXTools::SaveDDSTexture( VertexAsylum::vaStream & outStream, ID3D11Resource* pSource )
{
    //fix this properly using 
//    HRESULT hr = DirectX::SaveDDSTextureToFile( vaDirectXCore::GetImmediateContext(), pSource, outStream );
//    return SUCCEEDED( hr );

    DirectX::ScratchImage scratchImage;

    HRESULT hr = DirectX::CaptureTexture( vaDirectXCore::GetDevice(), vaDirectXCore::GetImmediateContext(), pSource, scratchImage );
    if( FAILED( hr ) )
    {
        assert( false );
        return false;
    }

    DirectX::Blob blob;

    hr = DirectX::SaveToDDSMemory( scratchImage.GetImages( ), scratchImage.GetImageCount( ), scratchImage.GetMetadata( ), DirectX::DDS_FLAGS_NONE, blob );
    if( FAILED( hr ) )
    {
        assert( false );
        return false;
    }

    return outStream.Write( blob.GetBufferPointer(), blob.GetBufferSize() );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



/*
////////////////////////////////////////////////////////////////////////////////
// vaDirectXTexture
////////////////////////////////////////////////////////////////////////////////
void vaDirectXTexture::Create( DXGI_FORMAT format, DXGI_FORMAT formatSRV, UINT width, UINT height, UINT arraySize, UINT mipLevels, UINT sampleCount, UINT sampleQuality, UINT additionalBindFlags, UINT miscFlags, D3D11_SUBRESOURCE_DATA * initialData )
{
    Destroy( );

    m_format = format;
    m_formatSRV = formatSRV;

    m_width = width;
    m_height = height;

    if( formatSRV != DXGI_FORMAT_UNKNOWN )
    {
        additionalBindFlags |= D3D11_BIND_SHADER_RESOURCE;
    }

    m_texture = vaDirectXTools::CreateTexture2D( format, width, height, initialData, arraySize, mipLevels, additionalBindFlags,
        D3D11_USAGE_DEFAULT, 0, sampleCount, sampleQuality, miscFlags );

    vaDirectXCore::NameObject( m_texture, "vaDirectXTexture::Create" );

    if( formatSRV != DXGI_FORMAT_UNKNOWN )
    {
        m_textureSRV = vaDirectXTools::CreateShaderResourceView( m_texture, formatSRV );
    }
}
//
void vaDirectXTexture::Destroy( )
{
    SAFE_RELEASE( m_texture );
    SAFE_RELEASE( m_textureSRV );
    m_format = DXGI_FORMAT_UNKNOWN;
    m_formatSRV = DXGI_FORMAT_UNKNOWN;
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// vaDirectXRenderTarget
////////////////////////////////////////////////////////////////////////////////
void vaDirectXRenderTarget::Create( DXGI_FORMAT format, DXGI_FORMAT formatSRV, DXGI_FORMAT formatRTV, UINT width, UINT height, UINT arraySize, UINT mipLevels, UINT sampleCount, UINT sampleQuality, UINT additionalBindFlags, UINT miscFlags, D3D11_SUBRESOURCE_DATA * initialData )
{
    if( formatRTV != DXGI_FORMAT_UNKNOWN )
    {
        additionalBindFlags |= D3D11_BIND_RENDER_TARGET;
    }

    vaDirectXTexture::Create( format, formatSRV, width, height, arraySize, mipLevels, sampleCount, sampleQuality, additionalBindFlags, miscFlags, initialData );

    m_formatRTV = formatRTV;

    if( formatRTV != DXGI_FORMAT_UNKNOWN )
    {
        D3D11_RENDER_TARGET_VIEW_DESC desc;

        desc.Format = formatRTV;
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;   // currently only texture2D supported, but should be expanded for cubemaps, etc.
        desc.Texture2D.MipSlice = 0;

        if( !SUCCEEDED( vaDirectXCore::GetDevice( )->CreateRenderTargetView( m_texture, &desc, &m_textureRTV ) ) )
        {
            assert( false );
        }
    }
}
//
void vaDirectXRenderTarget::Destroy( )
{
    vaDirectXTexture::Destroy( );

    SAFE_RELEASE( m_textureRTV );
    m_formatRTV = DXGI_FORMAT_UNKNOWN;
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// vaDirectXDepthStencilBuffer
////////////////////////////////////////////////////////////////////////////////
void vaDirectXDepthStencilBuffer::Create( DXGI_FORMAT formatTex, DXGI_FORMAT formatSRV, DXGI_FORMAT formatDSV, UINT width, UINT height, UINT arraySize, UINT sampleCount, UINT sampleQuality, UINT miscFlags )
{
    Destroy( );

    m_formatTex = formatTex;
    m_formatSRV = formatSRV;
    m_formatDSV = formatDSV;

    m_width = width;
    m_height = height;

    UINT bindFlags = D3D11_BIND_DEPTH_STENCIL;
    if( formatSRV != DXGI_FORMAT_UNKNOWN )
    {
        bindFlags |= D3D11_BIND_SHADER_RESOURCE;
    }

    m_texture = vaDirectXTools::CreateTexture2D( formatTex, width, height, NULL, 1, 1, bindFlags,
        D3D11_USAGE_DEFAULT, 0, sampleCount, sampleQuality, 0 );

    vaDirectXCore::NameObject( m_texture, "vaDirectXDepthStencilBuffer::Create" );

    if( formatSRV != DXGI_FORMAT_UNKNOWN )
    {
        m_textureSRV = vaDirectXTools::CreateShaderResourceView( m_texture, m_formatSRV );
    }
    else
    {
        m_textureSRV = NULL;
    }

    {
        D3D11_DEPTH_STENCIL_VIEW_DESC desc;

        desc.Format = formatDSV;
        desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;   // currently only texture2D supported, but should be expanded for cubemaps, etc.
        desc.Flags = 0;
        desc.Texture2D.MipSlice = 0;

        if( !SUCCEEDED( vaDirectXCore::GetDevice( )->CreateDepthStencilView( m_texture, &desc, &m_textureDSV ) ) )
        {
            assert( false );
        }
    }
}
//
void vaDirectXDepthStencilBuffer::CreateCopy( vaDirectXDepthStencilBuffer * copyFrom )
{
    Destroy( );

    assert( copyFrom->GetTexture( ) != NULL );
    assert( copyFrom->GetSRV( ) != NULL );
    assert( copyFrom->GetDSV( ) != NULL );
    m_formatTex = copyFrom->GetFormatTex( );
    m_formatSRV = copyFrom->GetFormatSRV( );
    m_formatDSV = copyFrom->GetFormatDSV( );
    m_texture = copyFrom->GetTexture( );
    m_textureSRV = copyFrom->GetSRV( );
    m_textureDSV = copyFrom->GetDSV( );
    m_width = copyFrom->GetWidth( );
    m_height = copyFrom->GetHeight( );
    m_texture->AddRef( );
    m_textureSRV->AddRef( );
    m_textureDSV->AddRef( );
}
//
void vaDirectXDepthStencilBuffer::Destroy( )
{
    SAFE_RELEASE( m_texture );
    SAFE_RELEASE( m_textureSRV );
    SAFE_RELEASE( m_textureDSV );
    m_formatTex = DXGI_FORMAT_UNKNOWN;
    m_formatSRV = DXGI_FORMAT_UNKNOWN;
    m_formatDSV = DXGI_FORMAT_UNKNOWN;
    m_width = 0;
    m_height = 0;
}
////////////////////////////////////////////////////////////////////////////////
*/