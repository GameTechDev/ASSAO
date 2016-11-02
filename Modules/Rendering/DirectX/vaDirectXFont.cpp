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

#include "vaDirectXFont.h"
#include "vaDirectXTools.h"
#include <D3DCompiler.h>

#include <stdio.h>
#include <stdarg.h>
#include <vector>

#include <DirectXMath.h>

// Direct3D9 includes - should get rid of them
// #include <d3d9.h>
// #include <d3dx9.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace std;

namespace VertexAsylum
{

   struct DXUTSpriteVertex
   {
      DirectX::XMFLOAT3 vPos;
      DirectX::XMFLOAT4 vColor;
      DirectX::XMFLOAT2 vTex;
   };

   struct DXUT_SCREEN_VERTEX_10
   {
      float x, y, z;
      DirectX::XMFLOAT4 color;
      float tu, tv;
   };

   static CHAR g_strUIEffectFile[] = \
      "Texture2D g_Texture;"\
      ""\
      "SamplerState Sampler"\
      "{"\
      "    Filter = MIN_MAG_MIP_LINEAR;"\
      "    AddressU = Wrap;"\
      "    AddressV = Wrap;"\
      "};"\
      ""\
      "BlendState UIBlend"\
      "{"\
      "    AlphaToCoverageEnable = FALSE;"\
      "    BlendEnable[0] = TRUE;"\
      "    SrcBlend = SRC_ALPHA;"\
      "    DestBlend = INV_SRC_ALPHA;"\
      "    BlendOp = ADD;"\
      "    SrcBlendAlpha = ONE;"\
      "    DestBlendAlpha = ZERO;"\
      "    BlendOpAlpha = ADD;"\
      "    RenderTargetWriteMask[0] = 0x0F;"\
      "};"\
      ""\
      "BlendState NoBlending"\
      "{"\
      "    BlendEnable[0] = FALSE;"\
      "    RenderTargetWriteMask[0] = 0x0F;"\
      "};"\
      ""\
      "DepthStencilState DisableDepth"\
      "{"\
      "    DepthEnable = false;"\
      "};"\
      "DepthStencilState EnableDepth"\
      "{"\
      "    DepthEnable = true;"\
      "};"\
      "struct VS_OUTPUT"\
      "{"\
      "    float4 Pos : POSITION;"\
      "    float4 Dif : COLOR;"\
      "    float2 Tex : TEXCOORD;"\
      "};"\
      ""\
      "VS_OUTPUT VS( float3 vPos : POSITION,"\
      "              float4 Dif : COLOR,"\
      "              float2 vTexCoord0 : TEXCOORD )"\
      "{"\
      "    VS_OUTPUT Output;"\
      ""\
      "    Output.Pos = float4( vPos, 1.0f );"\
      "    Output.Dif = Dif;"\
      "    Output.Tex = vTexCoord0;"\
      ""\
      "    return Output;"\
      "}"\
      ""\
      "float4 PS( VS_OUTPUT In ) : SV_Target"\
      "{"\
      "    return g_Texture.Sample( Sampler, In.Tex ) * In.Dif;"\
      "}"\
      ""\
      "float4 PSUntex( VS_OUTPUT In ) : SV_Target"\
      "{"\
      "    return In.Dif;"\
      "}"\
      ""\
      "technique10 RenderUI"\
      "{"\
      "    pass P0"\
      "    {"\
      "        SetVertexShader( CompileShader( vs_5_0, VS() ) );"\
      "        SetGeometryShader( NULL );"\
      "        SetPixelShader( CompileShader( ps_5_0, PS() ) );"\
      "        SetDepthStencilState( DisableDepth, 0 );"\
      "        SetBlendState( UIBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );"\
      "    }"\
      "}"\
      "technique10 RenderUIUntex"\
      "{"\
      "    pass P0"\
      "    {"\
      "        SetVertexShader( CompileShader( vs_5_0, VS() ) );"\
      "        SetGeometryShader( NULL );"\
      "        SetPixelShader( CompileShader( ps_5_0, PSUntex() ) );"\
      "        SetDepthStencilState( DisableDepth, 0 );"\
      "        SetBlendState( UIBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );"\
      "    }"\
      "}"\
      "technique10 RestoreState"\
      "{"\
      "    pass P0"\
      "    {"\
      "        SetDepthStencilState( EnableDepth, 0 );"\
      "        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );"\
      "    }"\
      "}";
   const UINT              g_uUIEffectFileSize = sizeof( g_strUIEffectFile );

   class vaDirectXFontGlobals : public vaSingletonBase<vaDirectXFontGlobals>, public vaDirectXNotifyTarget
   {

   public:
      ID3D11DepthStencilState *  m_pDepthStencilStateStored11;
      UINT                       m_StencilRefStored11; 
      ID3D11RasterizerState *    m_pRasterizerStateStored11;
      ID3D11BlendState *         m_pBlendStateStored11;   
      float                      m_BlendFactorStored11[4];
      UINT                       m_SampleMaskStored11;     
      ID3D11SamplerState *       m_pSamplerStateStored11;

      ID3D11VertexShader *       m_pVSRenderUI11;
      ID3D11PixelShader *        m_pPSRenderUI11;
      ID3D11DepthStencilState *  m_pDepthStencilStateUI11;
      ID3D11RasterizerState *    m_pRasterizerStateUI11;
      ID3D11BlendState *         m_pBlendStateUI11;
      ID3D11SamplerState *       m_pSamplerStateUI11;

      ID3D11Buffer *             m_fontBuffer;
      UINT                       m_fontBufferBytes;
      vector<DXUTSpriteVertex>   m_fontVertices;
      ID3D11ShaderResourceView * m_fontTextureSRV;
      ID3D11InputLayout *        m_fontInputLayout;

      vaDirectXFontGlobals()
      {
         m_pDepthStencilStateStored11  = NULL;
         m_StencilRefStored11          = 0;
         m_pRasterizerStateStored11    = NULL;
         m_pBlendStateStored11         = NULL;
         m_BlendFactorStored11[0] = m_BlendFactorStored11[1] = m_BlendFactorStored11[2] = m_BlendFactorStored11[3] = 0.0f;
         m_SampleMaskStored11          = 0;
         m_pSamplerStateStored11       = NULL;

         m_pVSRenderUI11               = NULL;
         m_pPSRenderUI11               = NULL;
         m_pDepthStencilStateUI11      = NULL;
         m_pRasterizerStateUI11        = NULL;
         m_pBlendStateUI11             = NULL;
         m_pSamplerStateUI11           = NULL;

         m_fontBuffer                  = NULL;
         m_fontBufferBytes             = 0;
         m_fontTextureSRV              = NULL;
         m_fontInputLayout             = NULL;

         VA_DIRECTX_NOTIFY_TARGET_CONSTRUCT_PATCH( vaDirectXFontGlobals );
      }

      ~vaDirectXFontGlobals()
      {
         VA_DIRECTX_NOTIFY_TARGET_DESTRUCT_PATCH( vaDirectXFontGlobals );
      }

      virtual void OnReleasingSwapChain( )                                                 {}
      virtual void OnResizedSwapChain( const DXGI_SURFACE_DESC & backBufferSurfaceDesc )   {}

      virtual void OnDeviceCreated( ID3D11Device* pd3dDevice, IDXGISwapChain* swapChain )
      {
         HRESULT hr;

         // Compile Shaders
         ID3DBlob* pVSBlob = NULL;
         ID3DBlob* pPSBlob = NULL;
         ID3DBlob* pPSUntexBlob = NULL;
         V( D3DCompile( g_strUIEffectFile, g_uUIEffectFileSize, "none", NULL, NULL, "VS", "vs_5_0", 
            D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY, 0, &pVSBlob, NULL ) );
         V( D3DCompile( g_strUIEffectFile, g_uUIEffectFileSize, "none", NULL, NULL, "PS", "ps_5_0", 
            D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY, 0, &pPSBlob, NULL ) );
         //V( D3DCompile( g_strUIEffectFile, g_uUIEffectFileSize, "none", NULL, NULL, "PSUntex", "ps_5_0", 
         //     D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY, 0, &pPSUntexBlob, NULL ) );

         // Create Shaders
         V( pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pVSRenderUI11 ) );
         V( pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPSRenderUI11 ) );
         //V( pd3dDevice->CreatePixelShader( pPSUntexBlob->GetBufferPointer(), pPSUntexBlob->GetBufferSize(), NULL, &m_pPSRenderUIUntex11 ) );

         // States
         D3D11_DEPTH_STENCIL_DESC DSDesc;
         ZeroMemory( &DSDesc, sizeof( D3D11_DEPTH_STENCIL_DESC ) );
         DSDesc.DepthEnable = FALSE;
         DSDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
         DSDesc.DepthFunc = D3D11_COMPARISON_LESS;
         DSDesc.StencilEnable = FALSE;
         V( pd3dDevice->CreateDepthStencilState( &DSDesc, &m_pDepthStencilStateUI11 ) );

         D3D11_RASTERIZER_DESC RSDesc;
         RSDesc.AntialiasedLineEnable = FALSE;
         RSDesc.CullMode = D3D11_CULL_BACK;
         RSDesc.DepthBias = 0;
         RSDesc.DepthBiasClamp = 0.0f;
         RSDesc.DepthClipEnable = TRUE;
         RSDesc.FillMode = D3D11_FILL_SOLID;
         RSDesc.FrontCounterClockwise = FALSE;
         RSDesc.MultisampleEnable = TRUE;
         RSDesc.ScissorEnable = FALSE;
         RSDesc.SlopeScaledDepthBias = 0.0f;
         V( pd3dDevice->CreateRasterizerState( &RSDesc, &m_pRasterizerStateUI11 ) );

         D3D11_BLEND_DESC BSDesc;
         ZeroMemory( &BSDesc, sizeof( D3D11_BLEND_DESC ) );

         BSDesc.RenderTarget[0].BlendEnable = TRUE;
         BSDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
         BSDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
         BSDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
         BSDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
         BSDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
         BSDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
         BSDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;

         V( pd3dDevice->CreateBlendState( &BSDesc, &m_pBlendStateUI11 ) );

         D3D11_SAMPLER_DESC SSDesc;
         ZeroMemory( &SSDesc, sizeof( D3D11_SAMPLER_DESC ) );
         SSDesc.Filter = D3D11_FILTER_ANISOTROPIC   ;
         SSDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
         SSDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
         SSDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
         SSDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
         SSDesc.MaxAnisotropy = 16;
         SSDesc.MinLOD = 0;
         SSDesc.MaxLOD = D3D11_FLOAT32_MAX;
         if ( pd3dDevice->GetFeatureLevel() < D3D_FEATURE_LEVEL_9_3 ) {
            SSDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            SSDesc.MaxAnisotropy = 0;
         }
         V( pd3dDevice->CreateSamplerState( &SSDesc, &m_pSamplerStateUI11 ) );

         /*
         // Create the font and texture objects in the cache arrays.
         int i = 0;
         for( i = 0; i < m_FontCache.GetSize(); i++ )
         {
         hr = CreateFont11( i );
         if( FAILED( hr ) )
         return hr;
         }

         for( i = 0; i < m_TextureCache.GetSize(); i++ )
         {
         hr = CreateTexture11( i );
         if( FAILED( hr ) )
         return hr;
         }
         */

         // Create input layout
         const D3D11_INPUT_ELEMENT_DESC layout[] =
         {
         { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
         { "COLOR",     0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
         { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,       0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
         };

         V( pd3dDevice->CreateInputLayout( layout, ARRAYSIZE( layout ), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_fontInputLayout ) );

         // Release the blobs
         SAFE_RELEASE( pVSBlob );
         SAFE_RELEASE( pPSBlob );
         SAFE_RELEASE( pPSUntexBlob );

         vaFileTools::EmbeddedFileData embeddedFile = vaFileTools::EmbeddedFilesFind( L"textures:\\Font.dds" );
         if( embeddedFile.HasContents() )
         {
            ID3D11Resource * resource = vaDirectXTools::LoadTextureDDS( embeddedFile.MemStream->GetBuffer( ), embeddedFile.MemStream->GetLength( ), true, true );
            if( resource == NULL )
            {
               VA_WARN( L"Unable to create embedded font texture!" );
            }
            else
            {
               m_fontTextureSRV = vaDirectXTools::CreateShaderResourceView( resource );
               SAFE_RELEASE( resource );
            }

            if( m_fontTextureSRV == NULL )
               VA_WARN( L"Unable to create embedded font texture!" );
         }
         else
         {
            VA_WARN( L"Unable to find embedded font texture!" );
         }

      }

      virtual void OnDeviceDestroyed( )
      {
         SAFE_RELEASE( m_pVSRenderUI11 );
         SAFE_RELEASE( m_pPSRenderUI11 );
         SAFE_RELEASE( m_pDepthStencilStateUI11 );
         SAFE_RELEASE( m_pRasterizerStateUI11 );
         SAFE_RELEASE( m_pBlendStateUI11 );
         SAFE_RELEASE( m_pSamplerStateUI11 );
         SAFE_RELEASE( m_fontInputLayout );
         SAFE_RELEASE( m_fontTextureSRV );
         SAFE_RELEASE( m_fontBuffer );
         m_fontBufferBytes = 0;
      }
   };

   static vaDirectXFontGlobals * g_crappyFontGlobals = NULL;

   void vaDirectXFont::InitializeFontGlobals( )
   {
       g_crappyFontGlobals = new vaDirectXFontGlobals( );
   }

   void vaDirectXFont::DeinitializeFontGlobals( )
   {
       delete g_crappyFontGlobals;
   }

}

using namespace VertexAsylum;

// Only one instance of vaDirectXFont can be in use (Begin<->End) simultaneously
static vaDirectXFont *        g_currentlyInUse = NULL;

static inline DirectX::XMFLOAT4 XMFLOAT4_FROM_U32(unsigned int c)
{
   DirectX::XMFLOAT4 cv(((c >> 16) & 0xFF) / 255.0f, ((c >> 8) & 0xFF) / 255.0f,
      ( c & 0xFF ) / 255.0f, ( ( c >> 24 ) & 0xFF ) / 255.0f );
   return cv;
}

static void BeginText11()
{
   g_crappyFontGlobals->m_fontVertices.clear();
}

static void EndText11( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3d11DeviceContext )
{

   // ensure our buffer size can hold our sprites
   UINT FontDataBytes = (UINT) (g_crappyFontGlobals->m_fontVertices.size() * sizeof( DXUTSpriteVertex ));
   if( (g_crappyFontGlobals->m_fontBuffer == NULL) || (g_crappyFontGlobals->m_fontBufferBytes < FontDataBytes) )
   {
      SAFE_RELEASE( g_crappyFontGlobals->m_fontBuffer );
      g_crappyFontGlobals->m_fontBufferBytes = FontDataBytes;

      D3D11_BUFFER_DESC BufferDesc;
      BufferDesc.ByteWidth = g_crappyFontGlobals->m_fontBufferBytes;
      BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
      BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
      BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
      BufferDesc.MiscFlags = 0;

      pd3dDevice->CreateBuffer( &BufferDesc, NULL, &g_crappyFontGlobals->m_fontBuffer );
   }

   // Copy the sprites over
   D3D11_BOX destRegion;
   destRegion.left = 0;
   destRegion.right = FontDataBytes;
   destRegion.top = 0;
   destRegion.bottom = 1;
   destRegion.front = 0;
   destRegion.back = 1;
   D3D11_MAPPED_SUBRESOURCE MappedResource;
   if ( S_OK == pd3d11DeviceContext->Map( g_crappyFontGlobals->m_fontBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) ) 
   {
      DXUTSpriteVertex * dstVertices = (DXUTSpriteVertex *)MappedResource.pData;
      for( unsigned int i = 0; i < g_crappyFontGlobals->m_fontVertices.size(); i++ )
         dstVertices[i] = g_crappyFontGlobals->m_fontVertices[i];
      //CopyMemory( MappedResource.pData, (void*)m_fontVertices.GetData(), FontDataBytes );
      pd3d11DeviceContext->Unmap(g_crappyFontGlobals->m_fontBuffer, 0);
   }

   ID3D11ShaderResourceView* pOldTexture = NULL;
   pd3d11DeviceContext->PSGetShaderResources( 0, 1, &pOldTexture );
   pd3d11DeviceContext->PSSetShaderResources( 0, 1, &g_crappyFontGlobals->m_fontTextureSRV );

   // Draw
   UINT Stride = sizeof( DXUTSpriteVertex );
   UINT Offset = 0;
   pd3d11DeviceContext->IASetVertexBuffers( 0, 1, &g_crappyFontGlobals->m_fontBuffer, &Stride, &Offset );
   pd3d11DeviceContext->IASetInputLayout( g_crappyFontGlobals->m_fontInputLayout );
   pd3d11DeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
   pd3d11DeviceContext->Draw( (UINT)g_crappyFontGlobals->m_fontVertices.size(), 0 );

   pd3d11DeviceContext->PSSetShaderResources( 0, 1, &pOldTexture );
   SAFE_RELEASE( pOldTexture );

   g_crappyFontGlobals->m_fontVertices.clear();
}

static void DrawText11DXUT( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3d11DeviceContext,
   LPCWSTR strText, RECT rcScreen, DirectX::XMFLOAT4 vFontColor,
   float fBBWidth, float fBBHeight, bool bCenter )
{
   float fCharTexSizeX = 0.010526315f;
   //float fGlyphSizeX = 14.0f / fBBWidth;
   //float fGlyphSizeY = 32.0f / fBBHeight;
   float fGlyphSizeX = 15.0f / fBBWidth;
   float fGlyphSizeY = 42.0f / fBBHeight;


   float fRectLeft = rcScreen.left / fBBWidth;
   float fRectTop = 1.0f - rcScreen.top / fBBHeight;

   fRectLeft = fRectLeft * 2.0f - 1.0f;
   fRectTop = fRectTop * 2.0f - 1.0f;

   int NumChars = (int)wcslen( strText );
   if( bCenter )
   {
      float fRectRight = rcScreen.right / fBBWidth;
      fRectRight = fRectRight * 2.0f - 1.0f;
      float fRectBottom = 1.0f - rcScreen.bottom / fBBHeight;
      fRectBottom = fRectBottom * 2.0f - 1.0f;
      float fcenterx = ((fRectRight - fRectLeft) - (float)NumChars*fGlyphSizeX) *0.5f;
      float fcentery = ((fRectTop - fRectBottom) - (float)1*fGlyphSizeY) *0.5f;
      fRectLeft += fcenterx ;    
      fRectTop -= fcentery;
   }
   float fOriginalLeft = fRectLeft;
   float fTexTop = 0.0f;
   float fTexBottom = 1.0f;

   float fDepth = 0.5f;
   for( int i=0; i<NumChars; i++ )
   {
      if( strText[i] == '\n' )
      {
         fRectLeft = fOriginalLeft;
         fRectTop -= fGlyphSizeY;

         continue;
      }
      else if( strText[i] < 32 || strText[i] > 126 )
      {
         continue;
      }

      // Add 6 sprite vertices
      DXUTSpriteVertex SpriteVertex;
      float fRectRight = fRectLeft + fGlyphSizeX;
      float fRectBottom = fRectTop - fGlyphSizeY;
      float fTexLeft = ( strText[i] - 32 ) * fCharTexSizeX;
      float fTexRight = fTexLeft + fCharTexSizeX;

      // tri1
      SpriteVertex.vPos = DirectX::XMFLOAT3(fRectLeft, fRectTop, fDepth);
      SpriteVertex.vTex = DirectX::XMFLOAT2(fTexLeft, fTexTop);
      SpriteVertex.vColor = vFontColor;
      g_crappyFontGlobals->m_fontVertices.push_back( SpriteVertex );

      SpriteVertex.vPos = DirectX::XMFLOAT3(fRectRight, fRectTop, fDepth);
      SpriteVertex.vTex = DirectX::XMFLOAT2(fTexRight, fTexTop);
      SpriteVertex.vColor = vFontColor;
      g_crappyFontGlobals->m_fontVertices.push_back( SpriteVertex );

      SpriteVertex.vPos = DirectX::XMFLOAT3(fRectLeft, fRectBottom, fDepth);
      SpriteVertex.vTex = DirectX::XMFLOAT2(fTexLeft, fTexBottom);
      SpriteVertex.vColor = vFontColor;
      g_crappyFontGlobals->m_fontVertices.push_back( SpriteVertex );

      // tri2
      SpriteVertex.vPos = DirectX::XMFLOAT3(fRectRight, fRectTop, fDepth);
      SpriteVertex.vTex = DirectX::XMFLOAT2(fTexRight, fTexTop);
      SpriteVertex.vColor = vFontColor;
      g_crappyFontGlobals->m_fontVertices.push_back( SpriteVertex );

      SpriteVertex.vPos = DirectX::XMFLOAT3(fRectRight, fRectBottom, fDepth);
      SpriteVertex.vTex = DirectX::XMFLOAT2(fTexRight, fTexBottom);
      SpriteVertex.vColor = vFontColor;
      g_crappyFontGlobals->m_fontVertices.push_back( SpriteVertex );

      SpriteVertex.vPos = DirectX::XMFLOAT3(fRectLeft, fRectBottom, fDepth);
      SpriteVertex.vTex = DirectX::XMFLOAT2(fTexLeft, fTexBottom);
      SpriteVertex.vColor = vFontColor;
      g_crappyFontGlobals->m_fontVertices.push_back( SpriteVertex );

      fRectLeft += fGlyphSizeX;

   }

   // TODO: We have to end text after every line so that rendering order between sprites and fonts is preserved
   EndText11( pd3dDevice, pd3d11DeviceContext );
}


vaDirectXFont::vaDirectXFont( int lineHeight )
{
   m_color = 0xFFFFFFFF;
   m_posX = 0;
   m_posY = 0;
   m_lineHeight = lineHeight;
}

vaDirectXFont::~vaDirectXFont()
{
}

//--------------------------------------------------------------------------------------
HRESULT vaDirectXFont::DrawFormattedTextLine( const WCHAR* strMsg, ... )
{
   WCHAR strBuffer[512];

   va_list args;
   va_start( args, strMsg );
   vswprintf_s( strBuffer, 512, strMsg, args );
   strBuffer[511] = L'\0';
   va_end( args );

   return DrawTextLine( strBuffer );
}

//--------------------------------------------------------------------------------------
HRESULT vaDirectXFont::DrawTextLine( const WCHAR* strMsg )
{
   RECT rc;
   SetRect( &rc, m_posX, m_posY, 0, 0 );

   // vaDirectXCore::GetImmediateContext() should not be used...
   DrawText11DXUT( vaDirectXCore::GetDevice(), vaDirectXCore::GetImmediateContext(), strMsg, rc, 
      XMFLOAT4_FROM_U32( m_color ), (float)vaDirectXCore::GetBackBufferSurfaceDesc().Width,
      (float)vaDirectXCore::GetBackBufferSurfaceDesc().Height, false );

   m_posY += m_lineHeight;

   return S_OK;
}

HRESULT vaDirectXFont::DrawFormattedTextLine( RECT& rc, DWORD dwFlags, const WCHAR* strMsg, ... )
{
   WCHAR strBuffer[512];

   va_list args;
   va_start( args, strMsg );
   vswprintf_s( strBuffer, 512, strMsg, args );
   strBuffer[511] = L'\0';
   va_end( args );

   return DrawTextLine( rc, dwFlags, strBuffer );
}

HRESULT vaDirectXFont::DrawTextLine( RECT& rc, DWORD dwFlags, const WCHAR* strMsg )
{ 
   // vaDirectXCore::GetImmediateContext() should not be used...
   DrawText11DXUT( vaDirectXCore::GetDevice(), vaDirectXCore::GetImmediateContext(), strMsg, rc, 
      XMFLOAT4_FROM_U32( m_color ), (float)vaDirectXCore::GetBackBufferSurfaceDesc().Width,
      (float)vaDirectXCore::GetBackBufferSurfaceDesc().Height, false );
   m_posY += m_lineHeight;

   return S_OK;
}

void vaDirectXFont::Begin()
{
   assert( g_currentlyInUse == NULL );
   g_currentlyInUse = this;

   // vaDirectXCore::GetImmediateContext() should not be used...
   ID3D11DeviceContext * immediateContext = vaDirectXCore::GetImmediateContext();

   immediateContext->OMGetDepthStencilState( &g_crappyFontGlobals->m_pDepthStencilStateStored11, &g_crappyFontGlobals->m_StencilRefStored11 );
   immediateContext->RSGetState( &g_crappyFontGlobals->m_pRasterizerStateStored11 );
   immediateContext->OMGetBlendState( &g_crappyFontGlobals->m_pBlendStateStored11, g_crappyFontGlobals->m_BlendFactorStored11, &g_crappyFontGlobals->m_SampleMaskStored11 );
   immediateContext->PSGetSamplers( 0, 1, &g_crappyFontGlobals->m_pSamplerStateStored11 );

   // Shaders
   immediateContext->VSSetShader( g_crappyFontGlobals->m_pVSRenderUI11, NULL, 0 );
   immediateContext->HSSetShader( NULL, NULL, 0 );
   immediateContext->DSSetShader( NULL, NULL, 0 );
   immediateContext->GSSetShader( NULL, NULL, 0 );
   immediateContext->PSSetShader( g_crappyFontGlobals->m_pPSRenderUI11, NULL, 0 );

   // States
   immediateContext->OMSetDepthStencilState( g_crappyFontGlobals->m_pDepthStencilStateUI11, 0 );
   immediateContext->RSSetState( g_crappyFontGlobals->m_pRasterizerStateUI11 );
   float BlendFactor[4] = { 0, 0, 0, 0 };
   immediateContext->OMSetBlendState( g_crappyFontGlobals->m_pBlendStateUI11, BlendFactor, 0xFFFFFFFF );
   immediateContext->PSSetSamplers( 0, 1, &g_crappyFontGlobals->m_pSamplerStateUI11 );

   BeginText11();
}
void vaDirectXFont::End()
{
   assert( g_currentlyInUse == this );
   g_currentlyInUse = NULL;

   // vaDirectXCore::GetImmediateContext() should not be used...
   ID3D11DeviceContext * immediateContext = vaDirectXCore::GetImmediateContext();

   immediateContext->OMSetDepthStencilState( g_crappyFontGlobals->m_pDepthStencilStateStored11, g_crappyFontGlobals->m_StencilRefStored11 );
   immediateContext->RSSetState( g_crappyFontGlobals->m_pRasterizerStateStored11 );
   immediateContext->OMSetBlendState( g_crappyFontGlobals->m_pBlendStateStored11, g_crappyFontGlobals->m_BlendFactorStored11, g_crappyFontGlobals->m_SampleMaskStored11 );
   immediateContext->PSSetSamplers( 0, 1, &g_crappyFontGlobals->m_pSamplerStateStored11 );

   SAFE_RELEASE( g_crappyFontGlobals->m_pDepthStencilStateStored11 );
   SAFE_RELEASE( g_crappyFontGlobals->m_pRasterizerStateStored11 );
   SAFE_RELEASE( g_crappyFontGlobals->m_pBlendStateStored11 );
   SAFE_RELEASE( g_crappyFontGlobals->m_pSamplerStateStored11 );
}
