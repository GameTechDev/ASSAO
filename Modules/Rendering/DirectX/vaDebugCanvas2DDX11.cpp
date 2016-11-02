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

#include "vaDebugCanvas2DDX11.h"

using namespace VertexAsylum;

vaDebugCanvas2DDX11::vaDebugCanvas2DDX11( )
{
   D3D11_INPUT_ELEMENT_DESC inputElements[] =
   {
      { "SV_Position",  0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "COLOR",        0, DXGI_FORMAT_B8G8R8A8_UNORM,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEXCOORD",     0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEXCOORD",     1, DXGI_FORMAT_R32G32_FLOAT,        0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
   };

   m_vertexShader.CreateShaderAndILFromFile( L"vaCanvas.hlsl", "vs_5_0", "VS_Canvas2D", NULL, inputElements, _countof(inputElements) );
   m_pixelShader.CreateShaderFromFile( L"vaCanvas.hlsl", "ps_5_0", "PS_Canvas2D", NULL );

   m_vertexBufferCurrentlyUsed = 0;
   m_vertexBufferSize = 0;

   m_width = 0;
   m_height = 0;

   VA_DIRECTX_NOTIFY_TARGET_CONSTRUCT_PATCH( vaDebugCanvas2DDX11 );
}

vaDebugCanvas2DDX11::~vaDebugCanvas2DDX11( )
{
   VA_DIRECTX_NOTIFY_TARGET_DESTRUCT_PATCH( vaDebugCanvas2DDX11 );
}

void vaDebugCanvas2DDX11::OnResizedSwapChain( const DXGI_SURFACE_DESC & backBufferSurfaceDesc ) 
{ 
    m_width = backBufferSurfaceDesc.Width; m_height = backBufferSurfaceDesc.Height; 
}

#define vaDirectXCanvas2D_FORMAT_WSTR() \
   va_list args; \
   va_start(args, text); \
   int nBuf; \
   wchar_t szBuffer[2048]; \
   nBuf = _vsnwprintf_s(szBuffer, _countof(szBuffer), _countof(szBuffer)-1, text, args); \
   assert(nBuf < sizeof(szBuffer)); \
   va_end(args); \
   //
#define vaDirectXCanvas2D_FORMAT_STR() \
   va_list args; \
   va_start(args, text); \
   int nBuf; \
   char szBuffer[2048]; \
   nBuf = _vsnprintf_s(szBuffer, _countof(szBuffer), _countof(szBuffer)-1, text, args); \
   assert(nBuf < sizeof(szBuffer)); \
   va_end(args); \


void vaDebugCanvas2DDX11::DrawString( int x, int y, const wchar_t * text, ... )
{
   vaDirectXCanvas2D_FORMAT_WSTR();
   m_drawStringLines.push_back( DrawStringItem(x, y, 0xFF000000, 0x00000000, szBuffer ) );      
}
//
void vaDebugCanvas2DDX11::DrawString( int x, int y, unsigned int penColor, const wchar_t * text, ... )
{
   vaDirectXCanvas2D_FORMAT_WSTR();
   m_drawStringLines.push_back( DrawStringItem(x, y, penColor, 0x00000000, szBuffer ) );
}
//
void vaDebugCanvas2DDX11::DrawString( int x, int y, unsigned int penColor, unsigned int shadowColor, const wchar_t * text, ... )
{
   vaDirectXCanvas2D_FORMAT_WSTR();
   m_drawStringLines.push_back( DrawStringItem(x, y, penColor, shadowColor, szBuffer ) );
}
//
void vaDebugCanvas2DDX11::DrawString( int x, int y, const char * text, ... )
{
   vaDirectXCanvas2D_FORMAT_STR();
   m_drawStringLines.push_back( DrawStringItem(x, y, 0xFF000000, 0x00000000, vaStringTools::SimpleWiden( std::string(szBuffer) ).c_str() ) );      
}
//
void vaDebugCanvas2DDX11::DrawString( int x, int y, unsigned int penColor, const char * text, ... )
{
   vaDirectXCanvas2D_FORMAT_STR();
   m_drawStringLines.push_back( DrawStringItem(x, y, penColor, 0x00000000, vaStringTools::SimpleWiden( std::string(szBuffer) ).c_str() ) );
}
//
void vaDebugCanvas2DDX11::DrawString( int x, int y, unsigned int penColor, unsigned int shadowColor, const char * text, ... )
{
   vaDirectXCanvas2D_FORMAT_STR();
   m_drawStringLines.push_back( DrawStringItem(x, y, penColor, shadowColor, vaStringTools::SimpleWiden( std::string(szBuffer) ).c_str() ) );
}
//
void vaDebugCanvas2DDX11::DrawLine( float x0, float y0, float x1, float y1, unsigned int penColor )
{
   m_drawLines.push_back( DrawLineItem( x0, y0, x1, y1, penColor ) );
}
//
void vaDebugCanvas2DDX11::DrawRectangle( float x0, float y0, float width, float height, unsigned int penColor )
{
   DrawLine( x0 - 0.5f, y0, x0 + width, y0, penColor );
   DrawLine( x0 + width, y0, x0 + width, y0 + height, penColor );
   DrawLine( x0 + width, y0 + height, x0, y0 + height, penColor );
   DrawLine( x0, y0 + height, x0, y0, penColor );
}
//
void vaDebugCanvas2DDX11::FillRectangle( float x0, float y0, float width, float height, unsigned int brushColor )
{
   m_drawRectangles.push_back( DrawRectangleItem( x0, y0, width, height, brushColor ) );
}
//
void vaDebugCanvas2DDX11::DrawCircle( float x, float y, float radius, unsigned int penColor, float tess )
{
   tess = vaMath::Clamp( tess, 0.0f, 1.0f );

   float circumference = 2 * VA_PIf * radius;
   int steps = (int)(circumference / 4.0f * tess);
   steps = vaMath::Clamp( steps, 5, 32768 );

   float cxp = x + cos( 0 * 2 * VA_PIf ) * radius;
   float cyp = y + sin( 0 * 2 * VA_PIf ) * radius;

   for( int i = 1; i <= steps; i++ )
   {
      float p = i / (float)steps;
      float cx = x + cos( p * 2 * VA_PIf ) * radius;
      float cy = y + sin( p * 2 * VA_PIf ) * radius;
   
      DrawLine( cxp, cyp, cx, cy, penColor );

      cxp = cx;
      cyp = cy;
   }
}

void vaDebugCanvas2DDX11::CleanQueued()
{
   m_drawRectangles.clear( );
   m_drawLines.clear( );
   m_drawStringLines.clear( );
}

void vaDebugCanvas2DDX11::OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain )
{
   // Create vertex buffer
   {
      m_vertexBufferSize = 256 * 1024;

      //int vertexBufferSizeInBytes = m_vertexBufferSize * sizeof(CanvasVertex2D);
      //const CD3D11_BUFFER_DESC xDesc( vertexBufferSizeInBytes, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, 0, 0 );
      //CD3D11_BUFFER_DESC BDesc( vertexBufferSizeInBytes, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE );

      m_vertexBuffer.Create( m_vertexBufferSize, (const CanvasVertex2D *)(NULL), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE );

      m_vertexBufferCurrentlyUsed = 0;
   }

}

void vaDebugCanvas2DDX11::OnDeviceDestroyed( )
{
   m_vertexBuffer.Destroy();

}

void vaDebugCanvas2DDX11::Draw( ID3D11DeviceContext * context, bool bJustClearData )
{
   const int canvasWidth = GetWidth();
   const int canvasHeight = GetHeight();

   // Fill shapes first
   if( !bJustClearData )
   {
      uint32 rectsDrawn = 0;
      while( rectsDrawn < m_drawRectangles.size() )
      {
         if( (m_vertexBufferCurrentlyUsed+6) >= m_vertexBufferSize )
         {
            m_vertexBufferCurrentlyUsed = 0;
         }

         D3D11_MAP mapType = (m_vertexBufferCurrentlyUsed == 0)?(D3D11_MAP_WRITE_DISCARD):(D3D11_MAP_WRITE_NO_OVERWRITE);

         D3D11_MAPPED_SUBRESOURCE mappedSubresource;
         context->Map( m_vertexBuffer.GetBuffer(), 0, mapType, 0, &mappedSubresource );
         CanvasVertex2D * vertices = (CanvasVertex2D*)mappedSubresource.pData;

         int drawFromVertex = m_vertexBufferCurrentlyUsed;

         while( (rectsDrawn < m_drawRectangles.size()) && ((m_vertexBufferCurrentlyUsed+6) < m_vertexBufferSize) )
         {
            const int index = m_vertexBufferCurrentlyUsed;

            const DrawRectangleItem & rect = m_drawRectangles[rectsDrawn];

            vertices[index+0] = CanvasVertex2D( canvasWidth, canvasHeight, vaVector2( rect.x, rect.y ),                             rect.color );
            vertices[index+1] = CanvasVertex2D( canvasWidth, canvasHeight, vaVector2( rect.x + rect.width, rect.y ),                rect.color );
            vertices[index+2] = CanvasVertex2D( canvasWidth, canvasHeight, vaVector2( rect.x, rect.y + rect.height ),               rect.color );
            vertices[index+3] = CanvasVertex2D( canvasWidth, canvasHeight, vaVector2( rect.x, rect.y + rect.height ),               rect.color );
            vertices[index+4] = CanvasVertex2D( canvasWidth, canvasHeight, vaVector2( rect.x + rect.width, rect.y ),                rect.color );
            vertices[index+5] = CanvasVertex2D( canvasWidth, canvasHeight, vaVector2( rect.x + rect.width, rect.y + rect.height ),  rect.color );

            m_vertexBufferCurrentlyUsed += 6;
            rectsDrawn++;
         }
         int drawVertexCount = m_vertexBufferCurrentlyUsed - drawFromVertex;
         context->Unmap( m_vertexBuffer.GetBuffer(), 0 );

         context->OMSetDepthStencilState( vaDirectXTools::GetDSS_DepthDisabled_NoDepthWrite(), 0x00 );
         context->RSSetState( vaDirectXTools::GetRS_CullNone_Fill() );
         float blendFactor[4] = { 0, 0, 0, 0 };
         context->OMSetBlendState( vaDirectXTools::GetBS_AlphaBlend(), blendFactor, 0xFFFFFFFF );

         context->IASetInputLayout( m_vertexShader.GetInputLayout() );
         //unsigned int stride = sizeof(CanvasVertex2D); unsigned int offset = 0; //drawFromVertex * stride;
         m_vertexBuffer.SetToD3DContext( context );
         //context->IASetVertexBuffers( 0, 1, &m_vertexBuffer.GetBuffer(), &stride, &offset );
         context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
         context->VSSetShader( m_vertexShader, NULL, 0 );
         context->PSSetShader( m_pixelShader, NULL, 0 );
         context->Draw( drawVertexCount, drawFromVertex );
      }

      //u_int uVertexCount = g_xRectangles.GetSize() * 6;
      //for( u_int i = 0; i < g_xRectangles.GetSize(); i++ )
      //{
      //   DrawRectangleItem& xRect = g_xRectangles[i];
      //   SlightlyLessSimpleVertex* pVerts = &g_xDrawVertices[i*6];
      //   pVerts[0] = SlightlyLessSimpleVertex( Vector_2( xRect.fX, xRect.fY ), Vector_2( 0.0f, 0.0f ), Vector_2( 0.0f, 0.0f ), xRect.uColour );
      //   pVerts[1] = SlightlyLessSimpleVertex( Vector_2( xRect.fX + xRect.fWidth, xRect.fY ), Vector_2( 0.0f, 0.0f ), Vector_2( 0.0f, 0.0f ), xRect.uColour );
      //   pVerts[2] = SlightlyLessSimpleVertex( Vector_2( xRect.fX, xRect.fY + xRect.fHeight ), Vector_2( 0.0f, 0.0f ), Vector_2( 0.0f, 0.0f ), xRect.uColour );
      //   pVerts[3] = SlightlyLessSimpleVertex( Vector_2( xRect.fX, xRect.fY + xRect.fHeight ), Vector_2( 0.0f, 0.0f ), Vector_2( 0.0f, 0.0f ), xRect.uColour );
      //   pVerts[4] = SlightlyLessSimpleVertex( Vector_2( xRect.fX + xRect.fWidth, xRect.fY ), Vector_2( 0.0f, 0.0f ), Vector_2( 0.0f, 0.0f ), xRect.uColour );
      //   pVerts[5] = SlightlyLessSimpleVertex( Vector_2( xRect.fX + xRect.fWidth, xRect.fY + xRect.fHeight ), Vector_2( 0.0f, 0.0f ), Vector_2( 0.0f, 0.0f ), xRect.uColour );
      //}

      //g_pxEffect->Begin( false, 1 );
      //g_pxEffect->RenderAllPrimitivePassesUp( D3DPT_TRIANGLELIST, uVertexCount/3, g_xDrawVertices, sizeof( SlightlyLessSimpleVertex ) );
      //g_pxEffect->End( );
   }

   // Lines
   if( !bJustClearData )
   {
      uint32 linesDrawn = 0;
      while( linesDrawn < m_drawLines.size() )
      {
         if( (m_vertexBufferCurrentlyUsed+2) >= m_vertexBufferSize )
         {
            m_vertexBufferCurrentlyUsed = 0;
         }

         D3D11_MAP mapType = (m_vertexBufferCurrentlyUsed == 0)?(D3D11_MAP_WRITE_DISCARD):(D3D11_MAP_WRITE_NO_OVERWRITE);

         D3D11_MAPPED_SUBRESOURCE mappedSubresource;
         context->Map( m_vertexBuffer.GetBuffer(), 0, mapType, 0, &mappedSubresource );
         CanvasVertex2D * vertices = (CanvasVertex2D*)mappedSubresource.pData;

         int drawFromVertex = m_vertexBufferCurrentlyUsed;

         while( (linesDrawn < m_drawLines.size()) && ((m_vertexBufferCurrentlyUsed+2) < m_vertexBufferSize) )
         {
            const int index = m_vertexBufferCurrentlyUsed;

            vertices[index+0] = CanvasVertex2D( canvasWidth, canvasHeight, vaVector2( m_drawLines[linesDrawn].x0, m_drawLines[linesDrawn].y0 ), m_drawLines[linesDrawn].penColor );
            vertices[index+1] = CanvasVertex2D( canvasWidth, canvasHeight, vaVector2( m_drawLines[linesDrawn].x1, m_drawLines[linesDrawn].y1 ), m_drawLines[linesDrawn].penColor );

            m_vertexBufferCurrentlyUsed += 2;
            linesDrawn++;
         }
         int drawVertexCount = m_vertexBufferCurrentlyUsed - drawFromVertex;
         context->Unmap( m_vertexBuffer.GetBuffer(), 0 );

         context->OMSetDepthStencilState( vaDirectXTools::GetDSS_DepthDisabled_NoDepthWrite(), 0x00 );
         context->RSSetState( vaDirectXTools::GetRS_CullNone_Fill() );
         float blendFactor[4] = { 0, 0, 0, 0 };
         context->OMSetBlendState( vaDirectXTools::GetBS_AlphaBlend(), blendFactor, 0xFFFFFFFF );

         context->IASetInputLayout( m_vertexShader.GetInputLayout() );
         //unsigned int stride = sizeof(CanvasVertex2D); unsigned int offset = 0; //drawFromVertex * stride;
         m_vertexBuffer.SetToD3DContext( context );
         //context->IASetVertexBuffers( 0, 1, &m_vertexBuffer.GetBuffer(), &stride, &offset );
         context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
         context->VSSetShader( m_vertexShader, NULL, 0 );
         context->PSSetShader( m_pixelShader, NULL, 0 );
         context->Draw( drawVertexCount, drawFromVertex );
      }
   }

   // Text
   if( !bJustClearData && m_drawStringLines.size() > 0 )
   {
      m_font.Begin();
      m_font.SetInsertionPos( 5, 5 );
      m_font.SetForegroundColor( 0xFFFFFFFF );

      for( size_t i = 0; i < m_drawStringLines.size(); i++ )
      {
         if( (m_drawStringLines[i].shadowColor & 0xFF000000) == 0 ) continue;

         m_font.SetInsertionPos( m_drawStringLines[i].x+1, m_drawStringLines[i].y+1 );
         m_font.SetForegroundColor( m_drawStringLines[i].shadowColor );
         m_font.DrawTextLine( m_drawStringLines[i].text.c_str() );
      }

      for( size_t i = 0; i < m_drawStringLines.size(); i++ )
      {
         m_font.SetInsertionPos( m_drawStringLines[i].x, m_drawStringLines[i].y );
         m_font.SetForegroundColor( m_drawStringLines[i].penColor );
         m_font.DrawTextLine( m_drawStringLines[i].text.c_str() );
      }

      m_font.End();
   }

   CleanQueued( );
}

/*

      //struct Circle2D
      //{
      //   float fX, fY, fRadiusFrom, fRadiusTo;
      //   u_int uColour;

      //   Circle2D( )		{}
      //   Circle2D( float fX, float fY, float fRadiusFrom, float fRadiusTo, u_int uColour ) : fX(fX), fY(fY), fRadiusFrom(fRadiusFrom), fRadiusTo(fRadiusTo), uColour(uColour) {}
      //};

      //struct PolyLinePoint2D
      //{
      //   float fX, fY;
      //   float fThickness;
      //   u_int uColour;

      //   PolyLinePoint2D( )		{}
      //   PolyLinePoint2D( float fX, float fY, float fThickness, u_int uColour ) : fX(fX), fY(fY), fThickness(fThickness), uColour(uColour) { }
      //};

      //static const u_int														g_nItemBufferSize = 4096;

      //static Collection_Vector<Direct3D_2DRenderer::Circle2D>		g_xCircles;
      //static Collection_Vector<Direct3D_2DRenderer::Line2D>		g_xLines;

      //static Direct3D_2DRenderer::SlightlyLessSimpleVertex								g_xDrawVertices[g_nItemBufferSize*6];

*/

/*


void Direct3D_2DRenderer::Flush( )
{
   IDirect3DSurface9* pSurf = NULL;
   Direct3D::D3DDevice->GetRenderTarget( 0, &pSurf );

   D3DSURFACE_DESC xDesc;
   pSurf->GetDesc( &xDesc );

   SAFE_RELEASE( pSurf );

   Direct3D::D3DDevice->SetRenderState( D3DRS_SRGBWRITEENABLE, FALSE );

   Render::CurrentStates.RequestZBufferEnabled(false);
   Render::CurrentStates.RequestZBufferWriteEnabled(false);
   Render::CurrentStates.RequestCullMode( _CULLMODE_NONE );
   Render::CurrentStates.RequestWireFrameMode( false );
   Render::CurrentStates.RequestTranslucencyMode( _TRANSLUCENCY_NORMAL );
   Render::CurrentStates.RequestZBufferEnabled(false);

   g_pxEffect->SetParameterByName( "g_xScreenSize", D3DXVECTOR4( (float)xDesc.Width, (float)xDesc.Height, 1.0f / (float)xDesc.Width, 1.0f / (float)xDesc.Height ) );

   Direct3D::D3DDevice->SetVertexDeclaration( g_pxVertDecl );

   if( g_xLines.GetSize() > 0 )
   {
      u_int uVertexCount = g_xLines.GetSize() * 2;
      for( u_int i = 0; i < g_xLines.GetSize(); i++ )
      {
         Line2D& xLine = g_xLines[i];
         SlightlyLessSimpleVertex* pVerts = &g_xDrawVertices[i*2];
         pVerts[0] = SlightlyLessSimpleVertex( Vector_2( xLine.fXFrom, xLine.fYFrom ), Vector_2( 0.0f, 0.0f ), Vector_2( 0.0f, 0.0f ), xLine.uColour );
         pVerts[1] = SlightlyLessSimpleVertex( Vector_2( xLine.fXTo, xLine.fYTo ), Vector_2( 0.0f, 0.0f ), Vector_2( 0.0f, 0.0f ), xLine.uColour );
      }

      g_pxEffect->Begin( false, 2 );
      g_pxEffect->RenderAllPrimitivePassesUp( D3DPT_LINELIST, uVertexCount/2, g_xDrawVertices, sizeof( SlightlyLessSimpleVertex ) );
      g_pxEffect->End( );
   }

   if( g_xCircles.GetSize() > 0 )
   {
      u_int uVertexCount = g_xCircles.GetSize() * 6;
      for( u_int i = 0; i < g_xCircles.GetSize(); i++ )
      {
         Circle2D& xCircle = g_xCircles[i];
         SlightlyLessSimpleVertex* pVerts = &g_xDrawVertices[i*6];
         pVerts[0] = SlightlyLessSimpleVertex( Vector_2( xCircle.fX - xCircle.fRadiusTo - 1.0f, xCircle.fY - xCircle.fRadiusTo - 1.0f ), Vector_2( xCircle.fX, xCircle.fY ), Vector_2( xCircle.fRadiusFrom, xCircle.fRadiusTo ), xCircle.uColour );
         pVerts[1] = SlightlyLessSimpleVertex( Vector_2( xCircle.fX + xCircle.fRadiusTo + 1.0f, xCircle.fY - xCircle.fRadiusTo - 1.0f ), Vector_2( xCircle.fX, xCircle.fY ), Vector_2( xCircle.fRadiusFrom, xCircle.fRadiusTo ), xCircle.uColour );
         pVerts[2] = SlightlyLessSimpleVertex( Vector_2( xCircle.fX - xCircle.fRadiusTo - 1.0f, xCircle.fY + xCircle.fRadiusTo + 1.0f ), Vector_2( xCircle.fX, xCircle.fY ), Vector_2( xCircle.fRadiusFrom, xCircle.fRadiusTo ), xCircle.uColour );
         pVerts[3] = SlightlyLessSimpleVertex( Vector_2( xCircle.fX - xCircle.fRadiusTo - 1.0f, xCircle.fY + xCircle.fRadiusTo + 1.0f ), Vector_2( xCircle.fX, xCircle.fY ), Vector_2( xCircle.fRadiusFrom, xCircle.fRadiusTo ), xCircle.uColour );
         pVerts[4] = SlightlyLessSimpleVertex( Vector_2( xCircle.fX + xCircle.fRadiusTo + 1.0f, xCircle.fY - xCircle.fRadiusTo - 1.0f ), Vector_2( xCircle.fX, xCircle.fY ), Vector_2( xCircle.fRadiusFrom, xCircle.fRadiusTo ), xCircle.uColour );
         pVerts[5] = SlightlyLessSimpleVertex( Vector_2( xCircle.fX + xCircle.fRadiusTo + 1.0f, xCircle.fY + xCircle.fRadiusTo + 1.0f ), Vector_2( xCircle.fX, xCircle.fY ), Vector_2( xCircle.fRadiusFrom, xCircle.fRadiusTo ), xCircle.uColour );
      }

      g_pxEffect->Begin( false, 0 );
      g_pxEffect->RenderAllPrimitivePassesUp( D3DPT_TRIANGLELIST, uVertexCount/3, g_xDrawVertices, sizeof( SlightlyLessSimpleVertex ) );
      g_pxEffect->End( );
   }

   g_xLines.Clear();
   g_xCircles.Clear();
   
   const bool bSRGB = Direct3D::UseGammaCorrection();
   Direct3D::D3DDevice->SetRenderState( D3DRS_SRGBWRITEENABLE, bSRGB );
}

void Direct3D_2DRenderer::DrawPolyline( PolyLinePoint2D* axPoints, int iPointCount )
{
   const int iMaxPolylinePointCount = g_nItemBufferSize-2;
   Assert( iPointCount < iMaxPolylinePointCount, "Direct3D_2DRenderer::DrawPolyline does not support as many points (will be clamped)" );
   iPointCount = Maths::Min( iPointCount, iMaxPolylinePointCount );

   IDirect3DSurface9* pSurf = NULL;
   Direct3D::D3DDevice->GetRenderTarget( 0, &pSurf );

   D3DSURFACE_DESC xDesc;
   pSurf->GetDesc( &xDesc );

   SAFE_RELEASE( pSurf );

   Direct3D::D3DDevice->SetRenderState( D3DRS_SRGBWRITEENABLE, FALSE );

   Render::CurrentStates.RequestZBufferEnabled(false);
   Render::CurrentStates.RequestZBufferWriteEnabled(false);
   Render::CurrentStates.RequestCullMode( _CULLMODE_NONE );
   Render::CurrentStates.RequestWireFrameMode( false );
   Render::CurrentStates.RequestTranslucencyMode( _TRANSLUCENCY_NORMAL );
   Render::CurrentStates.RequestZBufferEnabled(false);

   g_pxEffect->SetParameterByName( "g_xScreenSize", D3DXVECTOR4( (float)xDesc.Width, (float)xDesc.Height, 1.0f / (float)xDesc.Width, 1.0f / (float)xDesc.Height ) );

   Direct3D::D3DDevice->SetVertexDeclaration( g_pxVertDecl );

   u_int uVertexCount = (iPointCount-1) * 6;

   Vector_2 xDirPrev;
   Vector_2 xDirCurr;
   for( int i = -1; i < (iPointCount-1); i++ )
   {
      Vector_2 xDirNext;
      const PolyLinePoint2D& xPtNext		= axPoints[i+1];
      
      
      if( i < (iPointCount-2) )
      {
         const PolyLinePoint2D& xPtNextNext		= axPoints[i+2];
         xDirNext = Vector_2( xPtNextNext.fX, xPtNextNext.fY ) - Vector_2( xPtNext.fX, xPtNext.fY );
         xDirNext.Normalise();
      }

      if( i >= 0 )
      {
         const PolyLinePoint2D& xPtCurrent	= axPoints[i];

         float fThicknessIn	= xPtCurrent.fThickness;
         float fThicknessOut = xPtNext.fThickness;

         float fDotIn = xDirPrev * xDirCurr;
         float fDotOut = xDirCurr * xDirNext;

         float fInAngle	= Maths::ArcCosine( Maths::ClampToRange( fDotIn,	-0.9999f, 1.0f ) );
         float fOutAngle = Maths::ArcCosine( Maths::ClampToRange( fDotOut,	-0.9999f, 1.0f ) );
         float fInDist	= Maths::Tangent( fInAngle*0.5f );
         float fOutDist	= Maths::Tangent( fOutAngle*0.5f );

         Vector_2 xDirCurrLeft = Vector_2( +xDirCurr.y, -xDirCurr.x );

         Vector_2 xFrom( xPtCurrent.fX, xPtCurrent.fY );
         Vector_2 xTo( xPtNext.fX, xPtNext.fY );

         float fThicknessInMod = fThicknessIn * 0.5f + 1.0f;
         float fThicknessOutMod = fThicknessOut * 0.5f + 1.0f;

         fInDist		*= Maths::Sign( xDirCurrLeft * xDirPrev );
         fOutDist	*= Maths::Sign( xDirCurrLeft * xDirNext );
         

         Vector_2 xCFromLeft	= xFrom 		- xDirCurr * (fThicknessInMod * fInDist);
         Vector_2 xCFromRight	= xFrom 		+ xDirCurr * (fThicknessInMod * fInDist);
         Vector_2 xCToLeft		= xTo			- xDirCurr * (fThicknessOutMod * fOutDist);
         Vector_2 xCToRight	= xTo			+ xDirCurr * (fThicknessOutMod * fOutDist);
         Vector_2 xFromLeft	= xCFromLeft	+ xDirCurrLeft * fThicknessInMod;
         Vector_2 xFromRight	= xCFromRight	- xDirCurrLeft * fThicknessInMod;
         Vector_2 xToLeft		= xCToLeft		+ xDirCurrLeft * fThicknessOutMod;
         Vector_2 xToRight		= xCToRight		- xDirCurrLeft * fThicknessOutMod;

         SlightlyLessSimpleVertex* pVerts = &g_xDrawVertices[i*6];
         pVerts[0] = SlightlyLessSimpleVertex( xFromLeft,		xCFromLeft,		Vector_2( fThicknessIn * 0.5f, 0.0f ),	xPtCurrent.uColour );
         pVerts[1] = SlightlyLessSimpleVertex( xToLeft,			xCToLeft,		Vector_2( fThicknessOut * 0.5f, 0.0f ),	xPtNext.uColour );
         pVerts[2] = SlightlyLessSimpleVertex( xFromRight,		xCFromRight,	Vector_2( fThicknessIn * 0.5f, 0.0f ),	xPtCurrent.uColour );
         pVerts[3] = SlightlyLessSimpleVertex( xFromRight,		xCFromRight,	Vector_2( fThicknessIn * 0.5f, 0.0f ),	xPtCurrent.uColour );
         pVerts[4] = SlightlyLessSimpleVertex( xToLeft,			xCToLeft,		Vector_2( fThicknessOut * 0.5f, 0.0f ), 	xPtNext.uColour );
         pVerts[5] = SlightlyLessSimpleVertex( xToRight,			xCToRight,		Vector_2( fThicknessOut * 0.5f, 0.0f ), 	xPtNext.uColour );
      }
      else
      {
         xDirCurr = xDirNext;
      }

      xDirPrev = xDirCurr;
      xDirCurr = xDirNext;
   }

   g_pxEffect->Begin( false, 3 );
   g_pxEffect->RenderAllPrimitivePassesUp( D3DPT_TRIANGLELIST, uVertexCount/3, g_xDrawVertices, sizeof( SlightlyLessSimpleVertex ) );
   g_pxEffect->End( );

   const bool bSRGB = Direct3D::UseGammaCorrection();
   Direct3D::D3DDevice->SetRenderState( D3DRS_SRGBWRITEENABLE, bSRGB );
}

#endif 

*/


/*
float4	g_xScreenSize;

void VShader( inout float4 xColour : COLOR, inout float4 xPos : Position, inout float4 xUV : TEXCOORD0, out float2 xOrigScreenPos : TEXCOORD1 )
{
   xOrigScreenPos = xPos.xy;

   xPos.xy *= g_xScreenSize.zw;
   xPos.xy *= float2( 2.0, -2.0 );
   xPos.xy += float2( -1.0, 1.0 );
}

void PShader_Circle( inout float4 xColour : COLOR, in float4 xUV : TEXCOORD0, in float2 xOrigScreenPos : TEXCOORD1 )
{
   float2 xDelta = xOrigScreenPos.xy - xUV.xy;
   float fDistSq = dot( xDelta, xDelta );
   float fRadius1 = xUV.z;
   float fRadius2 = xUV.w;

   if( !((fDistSq >= fRadius1*fRadius1) && (fDistSq < fRadius2*fRadius2)) )
      discard;
}

void PShader_Rectangle( inout float4 xColour : COLOR, in float4 xUV : TEXCOORD0, in float2 xOrigScreenPos : TEXCOORD1 )
{
}

void PShader_Line( inout float4 xColour : COLOR, in float4 xUV : TEXCOORD0, in float2 xOrigScreenPos : TEXCOORD1 )
{

}

void PShader_LineAA( inout float4 xColour : COLOR, in float4 xUV : TEXCOORD0, in float2 xOrigScreenPos : TEXCOORD1 )
{
   float2 xDist = xOrigScreenPos - xUV.xy;

   xColour.a *= saturate( xUV.z - length( xDist ) + 0.5 );
}

technique Circle
{
    pass p0
    {		
      VertexShader	= compile vs_3_0 VShader();
      PixelShader		= compile ps_3_0 PShader_Circle();
    }
}

technique Rectangle
{
    pass p0
    {		
      VertexShader	= compile vs_3_0 VShader();
      PixelShader		= compile ps_3_0 PShader_Rectangle();
    }
}

technique Line
{
    pass p0
    {		
      VertexShader	= compile vs_3_0 VShader();
      PixelShader		= compile ps_3_0 PShader_Line();
    }
}

technique LineAA
{
    pass p0
    {		
      VertexShader	= compile vs_3_0 VShader();
      PixelShader		= compile ps_3_0 PShader_LineAA();
    }
}

*/