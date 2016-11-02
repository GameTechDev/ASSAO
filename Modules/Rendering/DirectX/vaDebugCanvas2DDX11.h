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


namespace VertexAsylum
{

    class vaDebugCanvas2DDX11 : public vaDebugCanvas2DBase, protected vaDirectXNotifyTarget
    {
    private:
        // Types
        struct CanvasVertex2D
        {
            vaVector4   pos;
            uint32      color;
            vaVector2   uv0;
            vaVector2   uv1;
            vaVector2   screenPos;

            CanvasVertex2D( float x, float y, float z, float w, uint32 color, float sx, float sy ) : pos( x, y, z, w ), color( color ), uv0( 0.0f, 0.0f ), uv1( 0.0f, 0.0f ), screenPos( sx, sy ) { }
            CanvasVertex2D( float x, float y, float z, uint32 color, const vaMatrix4x4 * viewProj, float sx, float sy )
                : color( color ), uv0( 0.0f, 0.0f ), uv1( 0.0f, 0.0f ), screenPos( sx, sy )
            {
                pos = vaVector4::Transform( vaVector4( x, y, z, 1.0f ), *viewProj );
            }

            //CanvasVertex2D( const DirectX::XMFLOAT3 & vec, uint32 color, const vaMatrix4x4 * viewProj, float sx, float sy )
            //    : color( color ), uv0( 0.0f, 0.0f ), uv1( 0.0f, 0.0f ), screenPos( sx, sy )
            //{
            //    pos = vaVector4::Transform( vaVector4( vec.x, vec.y, vec.z, 1.0f ), *viewProj );
            //}

            CanvasVertex2D( int screenWidth, int screenHeight, const vaVector2 & screenPos, uint32 color, const vaVector2 & uv0 = vaVector2( 0.0f, 0.0f ), const vaVector2 & uv1 = vaVector2( 0.0f, 0.0f ) )
            {
                this->pos.x = ( screenPos.x + 0.5f ) / screenWidth * 2.0f - 1.0f;
                this->pos.y = 1.0f - ( screenPos.y + 0.5f ) / screenHeight * 2.0f;
                this->pos.z = 0.5f;
                this->pos.w = 1.0f;
                this->color = color;
                this->uv0 = uv0;
                this->uv1 = uv1;
                this->screenPos = screenPos;
            }
        };
        //
        struct DrawRectangleItem
        {
            float x, y, width, height;
            uint32 color;

            DrawRectangleItem( )	{}
            DrawRectangleItem( float x, float y, float width, float height, uint32 color ) : x( x ), y( y ), width( width ), height( height ), color( color ) {}
        };
        //
        struct DrawStringItem
        {
            int            x, y;
            unsigned int   penColor;
            unsigned int   shadowColor;
            std::wstring   text;
            DrawStringItem( int x, int y, unsigned int penColor, unsigned int shadowColor, const wchar_t * text )
                : x( x ), y( y ), penColor( penColor ), shadowColor( shadowColor ), text( text ) {}
        };
        //
        struct DrawLineItem
        {
            float x0;
            float y0;
            float x1;
            float y1;
            unsigned int penColor;

            DrawLineItem( float x0, float y0, float x1, float y1, unsigned int penColor )
                : x0( x0 ), y0( y0 ), x1( x1 ), y1( y1 ), penColor( penColor ) { }
        };

    private:
        // 
        //vaViewport                  m_viewport;
        int                         m_width;
        int                         m_height;

        // buffers for queued render calls
        std::vector<DrawStringItem>      m_drawStringLines;
        std::vector<DrawLineItem>        m_drawLines;
        std::vector<DrawRectangleItem>m_drawRectangles;

        // GPU buffers
        vaDirectXVertexBuffer < CanvasVertex2D >
            m_vertexBuffer;
        uint32				           m_vertexBufferCurrentlyUsed;
        uint32				           m_vertexBufferSize;

        // other
        vaDirectXFont                    m_font;

        // shaders
        vaDirectXPixelShader             m_pixelShader;
        vaDirectXVertexShader            m_vertexShader;

    private:
        vaDebugCanvas2DDX11( const vaDebugCanvas2DDX11 & c )                                { assert( false ); }    // to prevent warnings (and also this object doesn't support copying/assignment)
        void operator = ( const vaDebugCanvas2DDX11 & )                                     { assert( false ); }    // to prevent warnings (and also this object doesn't support copying/assignment)

    public:
        vaDebugCanvas2DDX11( );
        ~vaDebugCanvas2DDX11( );

        void                 CleanQueued( );
        void                 Draw( ID3D11DeviceContext * context, bool bJustClearData = false );

    protected:
        // vaDebugCanvas2DBase
        virtual void         DrawString( int x, int y, const wchar_t * text, ... );
        virtual void         DrawString( int x, int y, unsigned int penColor, const wchar_t * text, ... );
        virtual void         DrawString( int x, int y, unsigned int penColor, unsigned int shadowColor, const wchar_t * text, ... );
        virtual void         DrawString( int x, int y, const char * text, ... );
        virtual void         DrawString( int x, int y, unsigned int penColor, const char * text, ... );
        virtual void         DrawString( int x, int y, unsigned int penColor, unsigned int shadowColor, const char * text, ... );
        //virtual void         GetTextWidth( const char * text );
        //virtual void         GetTextWidth( const wchar_t * text );
        virtual void         DrawLine( float x0, float y0, float x1, float y1, unsigned int penColor );
        virtual void         DrawRectangle( float x0, float y0, float width, float height, unsigned int penColor );
        virtual void         DrawCircle( float x, float y, float radius, unsigned int penColor, float tess = 0.6f );
        virtual void         FillRectangle( float x0, float y0, float width, float height, unsigned int brushColor );
        //
        virtual int          GetWidth( )       { return m_width; }
        virtual int          GetHeight( )      { return m_height; }

    protected:
        // vaDirectXNotifyTarget
        virtual void         OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain );
        virtual void         OnDeviceDestroyed( );
        virtual void         OnReleasingSwapChain( )                                                 { }
        virtual void         OnResizedSwapChain( const DXGI_SURFACE_DESC & backBufferSurfaceDesc );

    };


}
