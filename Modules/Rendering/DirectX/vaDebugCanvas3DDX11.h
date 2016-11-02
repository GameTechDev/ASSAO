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

#pragma once

#include "Core/vaCoreIncludes.h"

#include "Rendering/DirectX/vaDirectXCore.h"
#include "Rendering/DirectX/vaDirectXShader.h"
#include "Rendering/DirectX/vaDirectXTools.h"
#include "Rendering/DirectX/vaDirectXFont.h"

#include "Rendering/vaDebugCanvas.h"

#include "Rendering/vaRendering.h"


namespace VertexAsylum
{

   class vaDebugCanvas3DDX11 : public vaDebugCanvas3DBase, protected vaDirectXNotifyTarget
   {
   private:
      // Types
      struct CanvasVertex3D
      {
         vaVector4   pos;
         uint32      color;
         //vaVector2   uv0;
         //vaVector2   uv1;
         //vaVector2   screenPos;

         CanvasVertex3D( float x, float y, float z, float w, uint32 color /*, float sx, float sy*/ ) : pos( x, y, z, w ), color(color) { } //, uv0( 0.0f, 0.0f ), uv1( 0.0f, 0.0f ), screenPos( sx, sy ) { }
         CanvasVertex3D( float x, float y, float z, uint32 color, const vaMatrix4x4 * viewProj )// , float sx, float sy )
            : color(color) //, uv0( 0.0f, 0.0f ), uv1( 0.0f, 0.0f ), screenPos( sx, sy )
         { 
            pos = vaVector4::Transform( vaVector4( x, y, z, 1.0f ), *viewProj );
         }

         CanvasVertex3D( const vaVector3 & vec, uint32 color, const vaMatrix4x4 * viewProj ) //, float sx, float sy )
            : color(color) //, uv0( 0.0f, 0.0f ), uv1( 0.0f, 0.0f ), screenPos( sx, sy )
         { 
            pos = vaVector4::Transform( vaVector4( vec.x, vec.y, vec.z, 1.0f ), *viewProj );
         }

         CanvasVertex3D( const vaVector4 & vec, uint32 color )
             : color( color ), pos( vec )
         {
         }

         CanvasVertex3D( int screenWidth, int screenHeight, const vaVector2 & screenPos, uint32 color ) //, const vaVector2 & uv0 = vaVector2( 0.0f, 0.0f ), const vaVector2 & uv1 = vaVector2( 0.0f, 0.0f ) )
         {
            this->pos.x       = (screenPos.x+0.5f) / screenWidth * 2.0f - 1.0f;
            this->pos.y       = 1.0f - (screenPos.y+0.5f) / screenHeight * 2.0f;
            this->pos.z       = 0.5f;
            this->pos.w       = 1.0f;
            this->color       = color;
            //this->uv0         = uv0;
            //this->uv1         = uv1;
            //this->screenPos   = screenPos;
         }
      };
      //
      enum DrawItemType
      {
         Triangle,
         Box,
         Sphere,
      };
      //
      struct DrawLineItem
      {
         vaVector3         v0;
         vaVector3         v1;
         unsigned int      penColor;

         DrawLineItem( const vaVector3 &v0, const vaVector3 &v1, unsigned int penColor ) : v0(v0), v1(v1), penColor(penColor) { }
      };
      struct DrawItem
      {
         vaVector3         v0;
         vaVector3         v1;
         vaVector3         v2;
         unsigned int      penColor;
         unsigned int      brushColor;
         DrawItemType      type;
         int               transformIndex;

         DrawItem( const vaVector3 &v0, const vaVector3 &v1, const vaVector3 &v2, unsigned int penColor, unsigned int brushColor, DrawItemType type, int transformIndex = -1 ) 
            : v0(v0), v1(v1), v2(v2), penColor(penColor), brushColor(brushColor), type(type), transformIndex(transformIndex) { }
      };
      //
   private:
      // 
      //const vaViewport &         m_viewport;
      //int                              m_width;
      //int                              m_height;

      // buffers for queued render calls
      std::vector<DrawItem>            m_drawItems;
      std::vector<vaMatrix4x4>         m_drawItemsTransforms;
      std::vector<DrawLineItem>        m_drawLines;
      //
      // GPU buffers
      vaDirectXVertexBuffer<CanvasVertex3D>
                                       m_triVertexBuffer;
      uint32				               m_triVertexBufferStart;
      uint32				               m_triVertexBufferCurrentlyUsed;
      uint32				               m_triVertexBufferSizeInVerts;
      vaDirectXVertexBuffer<CanvasVertex3D>
                                       m_lineVertexBuffer;
      uint32				               m_lineVertexBufferStart;
      uint32				               m_lineVertexBufferCurrentlyUsed;
      uint32				               m_lineVertexBufferSizeInVerts;

      // shaders
      vaDirectXPixelShader             m_pixelShader;
      vaDirectXVertexShader            m_vertexShader;

      std::vector<vaVector3>            m_sphereVertices;
      std::vector<uint32>               m_sphereIndices;

   public:
      vaDebugCanvas3DDX11( );
      ~vaDebugCanvas3DDX11( );

      void                 CleanQueued();
      void                 Draw( vaDrawContext & drawContext, const vaMatrix4x4 & viewProj, bool bJustClearData = false );

   protected:
      // vaDebugCanvas3DBase
      virtual void         DrawLine( const vaVector3 & v0, const vaVector3 & v1, unsigned int penColor )
      {
         m_drawLines.push_back( DrawLineItem( v0, v1, penColor ) );
      }
      virtual void         DrawAxis( const vaVector3 & _v0, float size, const vaMatrix4x4 * transform = NULL, float alpha = 0.5f )
      {
         vaVector3 v0 = _v0;
         vaVector3 vX = v0 + vaVector3( size, 0.0f, 0.0f );
         vaVector3 vY = v0 + vaVector3( 0.0f, size, 0.0f );
         vaVector3 vZ = v0 + vaVector3( 0.0f, 0.0f, size );

         if( transform != NULL )
         {
            v0 = vaVector3::TransformCoord( v0, *transform );
            vX = vaVector3::TransformCoord( vX, *transform );
            vY = vaVector3::TransformCoord( vY, *transform );
            vZ = vaVector3::TransformCoord( vZ, *transform );
         }

         // X, red
         DrawLine( v0, vX, vaVector4::ToBGRA( vaVector4( 1.0f, 0.0f, 0.0f, alpha ) ) );
         // Y, green
         DrawLine( v0, vY, vaVector4::ToBGRA( vaVector4( 0.0f, 1.0f, 0.0f, alpha ) ) );
         // Z, blue
         DrawLine( v0, vZ, vaVector4::ToBGRA( vaVector4( 0.0f, 0.0f, 1.0f, alpha ) ) );
      }
      virtual void   DrawBox( const vaVector3 & v0, const vaVector3 & v1, unsigned int penColor, unsigned int brushColor, const vaMatrix4x4 * transform )
      {
         int transformIndex = -1;
         if( transform != NULL )
         {
            transformIndex = (int)m_drawItemsTransforms.size();
            m_drawItemsTransforms.push_back( *transform );
         }
         m_drawItems.push_back( DrawItem( v0, v1, vaVector3( 0.0f, 0.0f, 0.0f ), penColor, brushColor, Box, transformIndex ) );
      }
      virtual void   DrawTriangle( const vaVector3 & v0, const vaVector3 & v1, const vaVector3 & v2, unsigned int penColor, unsigned int brushColor, const vaMatrix4x4 * transform )
      {
         int transformIndex = -1;
         if( transform != NULL )
         {
            transformIndex = (int)m_drawItemsTransforms.size();
            m_drawItemsTransforms.push_back( *transform );
         }

         m_drawItems.push_back( DrawItem( v0, v1, v2, penColor, brushColor, Triangle, transformIndex ) );
      }
      //
      virtual void   DrawQuad( const vaVector3 & v0, const vaVector3 & v1, const vaVector3 & v2, const vaVector3 & v3, unsigned int penColor, unsigned int brushColor, const vaMatrix4x4 * transform )
      {
         int transformIndex = -1;
         if( transform != NULL )
         {
            transformIndex = (int)m_drawItemsTransforms.size();
            m_drawItemsTransforms.push_back( *transform );
         }

         m_drawItems.push_back( DrawItem( v0, v1, v2, penColor, brushColor, Triangle, transformIndex ) );
         m_drawItems.push_back( DrawItem( v2, v1, v3, penColor, brushColor, Triangle, transformIndex ) );
      }
      virtual void  DrawSphere( const vaVector3 & center, float radius, unsigned int penColor, unsigned int brushColor = 0x000000 )
      {
          m_drawItems.push_back( DrawItem( center, vaVector3( radius, 0.0f, 0.0f ), vaVector3( 0.0f, 0.0f, 0.0f ), penColor, brushColor, Sphere ) );
      }
      //
   
   private:
      void RenderTriangle( vaDrawContext & drawContext, ID3D11DeviceContext * context, const CanvasVertex3D & a, const CanvasVertex3D & b, const CanvasVertex3D & c );
      void FlushTriangles( vaDrawContext & drawContext, ID3D11DeviceContext * context );

      void RenderLine( vaDrawContext & drawContext, ID3D11DeviceContext * context, const CanvasVertex3D & a, const CanvasVertex3D & b );
      void RenderLineBatch( vaDrawContext & drawContext, ID3D11DeviceContext * context, DrawLineItem * itemFrom, size_t count, const vaMatrix4x4 & viewProj );
      void FlushLines( vaDrawContext & drawContext, ID3D11DeviceContext * context );


   protected:

      // vaDirectXNotifyTarget
      virtual void         OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain );
      virtual void         OnDeviceDestroyed( );
      virtual void         OnReleasingSwapChain( )                                                 { }
      virtual void         OnResizedSwapChain( const DXGI_SURFACE_DESC & backBufferSurfaceDesc )   {  } //m_width = m_viewport.Width; m_height = m_viewport.Height; }
   };


}
