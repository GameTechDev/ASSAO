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

#include "vaDebugCanvas3DDX11.h"

#include "Rendering/vaStandardShapes.h"

#include "Rendering/DirectX/vaRenderDeviceDX11.h"
#include "Rendering/DirectX/vaRenderDeviceContextDX11.h"


using namespace VertexAsylum;

vaDebugCanvas3DDX11::vaDebugCanvas3DDX11( )
//   : m_viewport( viewport )
{
   D3D11_INPUT_ELEMENT_DESC inputElements[] =
   {
      { "SV_Position",  0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "COLOR",        0, DXGI_FORMAT_B8G8R8A8_UNORM,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
   };

   //m_vertexShader.CreateShaderAndILFromBuffer( vertexShaderCode, "vs_5_0", "main", NULL, inputElements, _countof(inputElements) );
   //m_pixelShader.CreateShaderFromBuffer( pixelShaderCode, "ps_5_0", "main", NULL );

   m_vertexShader.CreateShaderAndILFromFile( L"vaCanvas.hlsl", "vs_5_0", "VS_Canvas3D", NULL, inputElements, _countof(inputElements) );
   m_pixelShader.CreateShaderFromFile( L"vaCanvas.hlsl", "ps_5_0", "PS_Canvas3D", NULL );

   m_triVertexBufferCurrentlyUsed   = 0;
   m_triVertexBufferSizeInVerts     = 0;
   m_triVertexBufferStart           = 0;

   m_lineVertexBufferCurrentlyUsed  = 0;
   m_lineVertexBufferSizeInVerts    = 0;
   m_lineVertexBufferStart          = 0;

   vaStandardShapes::CreateSphere( m_sphereVertices, m_sphereIndices, 2, true );

   VA_DIRECTX_NOTIFY_TARGET_CONSTRUCT_PATCH( vaDebugCanvas3DDX11 );
}

vaDebugCanvas3DDX11::~vaDebugCanvas3DDX11( )
{
   VA_DIRECTX_NOTIFY_TARGET_DESTRUCT_PATCH( vaDebugCanvas3DDX11 );
}

void vaDebugCanvas3DDX11::CleanQueued()
{
   m_drawItems.clear();
   m_drawItemsTransforms.clear();
   m_drawLines.clear();
}

void vaDebugCanvas3DDX11::OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain )
{
   // Create triangle vertex buffer
   {
      m_triVertexBufferSizeInVerts = 1024*1024*2;
      //int vertexBufferSizeInBytes = m_triVertexBufferSizeInVerts * sizeof(CanvasVertex3D);
      //const CD3D11_BUFFER_DESC xDesc( vertexBufferSizeInBytes, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, 0, 0 );
      //CD3D11_BUFFER_DESC BDesc( vertexBufferSizeInBytes, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE );
      //V( vaDirectXCore::GetDevice()->CreateBuffer( &BDesc, NULL, &m_triVertexBuffer ) );
      m_triVertexBuffer.Create( m_triVertexBufferSizeInVerts, (const CanvasVertex3D *)(NULL), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE );
      m_triVertexBufferCurrentlyUsed   = 0;
      m_triVertexBufferStart           = 0;
   }

   // Create line vertex buffer
   {
      m_lineVertexBufferSizeInVerts = 1024*1024*2;
      //int vertexBufferSizeInBytes = m_lineVertexBufferSizeInVerts * sizeof(CanvasVertex3D);
      //const CD3D11_BUFFER_DESC xDesc( vertexBufferSizeInBytes, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, 0, 0 );
      //CD3D11_BUFFER_DESC BDesc( vertexBufferSizeInBytes, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE );
      //V( vaDirectXCore::GetDevice()->CreateBuffer( &BDesc, NULL, &m_lineVertexBuffer ) );
      m_lineVertexBuffer.Create( m_lineVertexBufferSizeInVerts, (const CanvasVertex3D *)(NULL), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE );
      m_lineVertexBufferCurrentlyUsed  = 0;
      m_lineVertexBufferStart          = 0;
   }         
}

void vaDebugCanvas3DDX11::OnDeviceDestroyed( )
{
   m_triVertexBuffer.Destroy();
   m_triVertexBufferCurrentlyUsed   = 0;
   m_triVertexBufferSizeInVerts     = 0;
   m_triVertexBufferStart           = 0;

   m_lineVertexBuffer.Destroy();
   m_lineVertexBufferCurrentlyUsed  = 0;
   m_lineVertexBufferSizeInVerts    = 0;
   m_lineVertexBufferStart          = 0;
}

void vaDebugCanvas3DDX11::RenderLine( vaDrawContext & drawContext, ID3D11DeviceContext * context, const CanvasVertex3D & a, const CanvasVertex3D & b )
{
   if( (m_lineVertexBufferCurrentlyUsed+2) >= m_lineVertexBufferSizeInVerts )
   {
      FlushLines( drawContext, context );
      m_lineVertexBufferCurrentlyUsed = 0;
      m_lineVertexBufferStart = 0;
   }

   D3D11_MAP mapType = (m_lineVertexBufferCurrentlyUsed == 0)?(D3D11_MAP_WRITE_DISCARD):(D3D11_MAP_WRITE_NO_OVERWRITE);

   D3D11_MAPPED_SUBRESOURCE mappedSubresource;
   context->Map( m_lineVertexBuffer.GetBuffer(), 0, mapType, 0, &mappedSubresource );
   CanvasVertex3D * vertices = (CanvasVertex3D*)mappedSubresource.pData;

   vertices[m_lineVertexBufferCurrentlyUsed++] = a;
   vertices[m_lineVertexBufferCurrentlyUsed++] = b;

   context->Unmap( m_lineVertexBuffer.GetBuffer(), 0 );
}

void vaDebugCanvas3DDX11::RenderLineBatch( vaDrawContext & drawContext, ID3D11DeviceContext * context, DrawLineItem * itemFrom, size_t count, const vaMatrix4x4 & viewProj )
{
   if( (m_lineVertexBufferCurrentlyUsed+count*2) >= m_lineVertexBufferSizeInVerts )
   {
      FlushLines( drawContext, context );
      m_lineVertexBufferCurrentlyUsed = 0;
      m_lineVertexBufferStart = 0;
   }

   D3D11_MAP mapType = (m_lineVertexBufferCurrentlyUsed == 0)?(D3D11_MAP_WRITE_DISCARD):(D3D11_MAP_WRITE_NO_OVERWRITE);

   D3D11_MAPPED_SUBRESOURCE mappedSubresource;
   context->Map( m_lineVertexBuffer.GetBuffer(), 0, mapType, 0, &mappedSubresource );
   CanvasVertex3D * vertices = (CanvasVertex3D*)mappedSubresource.pData;

   for( size_t i = 0; i < count; i++ )
   {
       DrawLineItem & line = itemFrom[i];
   
       vertices[m_lineVertexBufferCurrentlyUsed++] = CanvasVertex3D( line.v0, line.penColor, &viewProj );
       vertices[m_lineVertexBufferCurrentlyUsed++] = CanvasVertex3D( line.v1, line.penColor, &viewProj );
   }


   context->Unmap( m_lineVertexBuffer.GetBuffer(), 0 );
}

void vaDebugCanvas3DDX11::FlushLines( vaDrawContext & drawContext, ID3D11DeviceContext * context )
{
   int verticesToRender = m_lineVertexBufferCurrentlyUsed - m_lineVertexBufferStart;

   if( verticesToRender > 0 )
   {
      context->OMSetDepthStencilState( (drawContext.Camera.GetUseReversedZ())?( vaDirectXTools::GetDSS_DepthEnabledG_NoDepthWrite( ) ):( vaDirectXTools::GetDSS_DepthEnabledL_NoDepthWrite( ) ), 0x00 );
      context->RSSetState( vaDirectXTools::GetRS_CullNone_Fill() );
      float blendFactor[4] = { 0, 0, 0, 0 };
      context->OMSetBlendState( vaDirectXTools::GetBS_AlphaBlend(), blendFactor, 0xFFFFFFFF );
      context->IASetInputLayout( m_vertexShader.GetInputLayout() );
      //unsigned int stride = sizeof(CanvasVertex3D); unsigned int offset = 0; //drawFromVertex * stride;
      //context->IASetVertexBuffers( 0, 1, &m_lineVertexBuffer, &stride, &offset );
      m_lineVertexBuffer.SetToD3DContext( context );
      context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
      context->VSSetShader( m_vertexShader, NULL, 0 );
      context->PSSetShader( m_pixelShader, NULL, 0 );
      context->Draw( verticesToRender, m_lineVertexBufferStart );
   }
   m_lineVertexBufferStart = m_lineVertexBufferCurrentlyUsed;
}

void vaDebugCanvas3DDX11::RenderTriangle( vaDrawContext & drawContext, ID3D11DeviceContext * context, const CanvasVertex3D & a, const CanvasVertex3D & b, const CanvasVertex3D & c )
{
   if( (m_triVertexBufferCurrentlyUsed+3) >= m_triVertexBufferSizeInVerts )
   {
       FlushTriangles( drawContext, context );
       m_triVertexBufferCurrentlyUsed = 0;
       m_triVertexBufferStart = 0;
   }

   D3D11_MAP mapType = (m_triVertexBufferCurrentlyUsed == 0)?(D3D11_MAP_WRITE_DISCARD):(D3D11_MAP_WRITE_NO_OVERWRITE);

   D3D11_MAPPED_SUBRESOURCE mappedSubresource;
   context->Map( m_triVertexBuffer.GetBuffer(), 0, mapType, 0, &mappedSubresource );
   CanvasVertex3D * vertices = (CanvasVertex3D*)mappedSubresource.pData;

   vertices[m_triVertexBufferCurrentlyUsed++] = a;
   vertices[m_triVertexBufferCurrentlyUsed++] = b;
   vertices[m_triVertexBufferCurrentlyUsed++] = c;

   context->Unmap( m_triVertexBuffer.GetBuffer(), 0 );
}

void vaDebugCanvas3DDX11::FlushTriangles( vaDrawContext & drawContext, ID3D11DeviceContext * context )
{
   int verticesToRender = m_triVertexBufferCurrentlyUsed - m_triVertexBufferStart;

   if( verticesToRender > 0 )
   {
      context->OMSetDepthStencilState( (drawContext.Camera.GetUseReversedZ())?( vaDirectXTools::GetDSS_DepthEnabledG_NoDepthWrite( ) ):( vaDirectXTools::GetDSS_DepthEnabledL_NoDepthWrite( ) ), 0x00 );
      context->RSSetState( vaDirectXTools::GetRS_CullNone_Fill() );
      float blendFactor[4] = { 0, 0, 0, 0 };
      context->OMSetBlendState( vaDirectXTools::GetBS_AlphaBlend(), blendFactor, 0xFFFFFFFF );
      context->IASetInputLayout( m_vertexShader.GetInputLayout() );
      //unsigned int stride = sizeof(CanvasVertex3D); unsigned int offset = 0; //drawFromVertex * stride;
      //context->IASetVertexBuffers( 0, 1, &m_triVertexBuffer, &stride, &offset );
      m_triVertexBuffer.SetToD3DContext( context );
      context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
      context->VSSetShader( m_vertexShader, NULL, 0 );
      context->PSSetShader( m_pixelShader, NULL, 0 );
      context->Draw( verticesToRender, m_triVertexBufferStart );
   }
   m_triVertexBufferStart = m_triVertexBufferCurrentlyUsed;
}

void vaDebugCanvas3DDX11::Draw( vaDrawContext & drawContext, const vaMatrix4x4 & viewProj, bool bJustClearData )
{
    vaRenderDeviceContextDX11 * apiContext = drawContext.APIContext.SafeCast<vaRenderDeviceContextDX11*>( );
    ID3D11DeviceContext * context = apiContext->GetDXImmediateContext( );

   if( !bJustClearData )
   {
      vaMatrix4x4 tempMat;
      for( size_t i = 0; i < m_drawItems.size(); i++ )
      {
         DrawItem & item = m_drawItems[i];

         // use viewProj by default
         const vaMatrix4x4 * trans = &viewProj;
         
         // or if the object has its own transform matrix, 'add' it to the viewProj
         if( item.transformIndex != -1 )
         {
            vaMatrix4x4 &local = m_drawItemsTransforms[item.transformIndex];
            tempMat = local * viewProj;
            trans = &tempMat;
         }

         if( item.type == Triangle )
         {
            CanvasVertex3D a0( item.v0, item.brushColor, trans );
            CanvasVertex3D a1( item.v1, item.brushColor, trans );
            CanvasVertex3D a2( item.v2, item.brushColor, trans );

            if( (item.brushColor & 0xFF000000) != 0 )
            {
               RenderTriangle( drawContext, context, a0, a1, a2 );
            }

            if( (item.penColor & 0xFF000000) != 0 )
            {
               a0.color = item.penColor;
               a1.color = item.penColor;
               a2.color = item.penColor;

               RenderLine( drawContext, context, a0, a1 );
               RenderLine( drawContext, context, a1, a2 );
               RenderLine( drawContext, context, a2, a0 );
            }

         }

         if( item.type == Box )
         {

            const vaVector3 & boxMin = item.v0;
            const vaVector3 & boxMax = item.v1;

            CanvasVertex3D a0( boxMin.x, boxMin.y, boxMin.z, item.brushColor, trans );
            CanvasVertex3D a1( boxMax.x, boxMin.y, boxMin.z, item.brushColor, trans );
            CanvasVertex3D a2( boxMax.x, boxMax.y, boxMin.z, item.brushColor, trans );
            CanvasVertex3D a3( boxMin.x, boxMax.y, boxMin.z, item.brushColor, trans );
            CanvasVertex3D b0( boxMin.x, boxMin.y, boxMax.z, item.brushColor, trans );
            CanvasVertex3D b1( boxMax.x, boxMin.y, boxMax.z, item.brushColor, trans );
            CanvasVertex3D b2( boxMax.x, boxMax.y, boxMax.z, item.brushColor, trans );
            CanvasVertex3D b3( boxMin.x, boxMax.y, boxMax.z, item.brushColor, trans );

            if( (item.brushColor & 0xFF000000) != 0 )
            {
               RenderTriangle( drawContext, context, a0, a2, a1 );
               RenderTriangle( drawContext, context, a2, a0, a3 );
               
               RenderTriangle( drawContext, context, b0, b1, b2 );
               RenderTriangle( drawContext, context, b2, b3, b0 );

               RenderTriangle( drawContext, context, a0, a1, b1 );
               RenderTriangle( drawContext, context, b1, b0, a0 );

               RenderTriangle( drawContext, context, a1, a2, b2 );
               RenderTriangle( drawContext, context, b1, a1, b2 );
               
               RenderTriangle( drawContext, context, a2, a3, b3 );
               RenderTriangle( drawContext, context, b3, b2, a2 );
               
               RenderTriangle( drawContext, context, a3, a0, b0 );
               RenderTriangle( drawContext, context, b0, b3, a3 );
            }

            if( (item.penColor & 0xFF000000) != 0 )
            {
               a0.color = item.penColor;
               a1.color = item.penColor;
               a2.color = item.penColor;
               a3.color = item.penColor;
               b0.color = item.penColor;
               b1.color = item.penColor;
               b2.color = item.penColor;
               b3.color = item.penColor;

               RenderLine( drawContext, context, a0, a1 );
               RenderLine( drawContext, context, a1, a2 );
               RenderLine( drawContext, context, a2, a3 );
               RenderLine( drawContext, context, a3, a0 );
               RenderLine( drawContext, context, a0, b0 );
               RenderLine( drawContext, context, a1, b1 );
               RenderLine( drawContext, context, a2, b2 );
               RenderLine( drawContext, context, a3, b3 );
               RenderLine( drawContext, context, b0, b1 );
               RenderLine( drawContext, context, b1, b2 );
               RenderLine( drawContext, context, b2, b3 );
               RenderLine( drawContext, context, b3, b0 );
            }
         }

         if( item.type == Sphere )
         {
             if( ( item.brushColor & 0xFF000000 ) != 0 )
             {
                 for( size_t i = 0; i < m_sphereIndices.size(); i += 3 )
                 {
                     vaVector3 sCenter  = item.v0;
                     float sRadius      = item.v1.x;

                     CanvasVertex3D a0( m_sphereVertices[m_sphereIndices[i + 0]] * sRadius + sCenter, item.brushColor, trans );
                     CanvasVertex3D a1( m_sphereVertices[m_sphereIndices[i + 1]] * sRadius + sCenter, item.brushColor, trans );
                     CanvasVertex3D a2( m_sphereVertices[m_sphereIndices[i + 2]] * sRadius + sCenter, item.brushColor, trans );

                     RenderTriangle( drawContext, context, a0, a1, a2 );
                 }
             }

             if( ( item.penColor & 0xFF000000 ) != 0 )
             {
                 for( size_t i = 0; i < m_sphereIndices.size( ); i += 3 )
                 {
                     vaVector3 sCenter = item.v0;
                     float sRadius = item.v1.x;

                     CanvasVertex3D a0( m_sphereVertices[m_sphereIndices[i + 0]] * sRadius + sCenter, item.penColor, trans );
                     CanvasVertex3D a1( m_sphereVertices[m_sphereIndices[i + 1]] * sRadius + sCenter, item.penColor, trans );
                     CanvasVertex3D a2( m_sphereVertices[m_sphereIndices[i + 2]] * sRadius + sCenter, item.penColor, trans );

                     RenderLine( drawContext, context, a0, a1 );
                     RenderLine( drawContext, context, a1, a2 );
                     RenderLine( drawContext, context, a2, a0 );
                 }
             }
         }
      }

      size_t batchSize = 256;
      for( size_t i = 0; i < m_drawLines.size(); i += batchSize )
         RenderLineBatch( drawContext, context, m_drawLines.data()+i, vaMath::Min(batchSize, m_drawLines.size() - i), viewProj );

      //for( i = 0; i < m_drawLines.size(); i++ )
      //{
      //   DrawLineItem & line = m_drawLines[i];
      //
      //   RenderLine( drawContext, context, CanvasVertex3D( line.v0, line.penColor, &viewProj ), CanvasVertex3D( line.v1, line.penColor, &viewProj ) );
      //}

      FlushTriangles( drawContext, context );
      FlushLines( drawContext, context );
   }
   CleanQueued( );
}

