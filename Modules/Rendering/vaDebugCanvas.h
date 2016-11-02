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


namespace VertexAsylum
{
    // FS: Decided to make these two singletons as I can't think of a use where they wouldn't be at the moment. If need
    // arises, one option is to split vaDebugCanvas2DBase into ICanvas2D interface and then inherit vaCanvas2DSingleton from it.

    class vaDebugCanvas2DBase : public vaSingletonBase < vaDebugCanvas2DBase >
    {
    protected:
        virtual ~vaDebugCanvas2DBase( ) {}
        //
    public:
        // would've used DrawText but thats #defined somewhere in windows.h or related headers :/
        virtual void        DrawString( int x, int y, const wchar_t * text, ... ) = 0;
        virtual void        DrawString( int x, int y, unsigned int penColor, const wchar_t * text, ... ) = 0;
        virtual void        DrawString( int x, int y, unsigned int penColor, unsigned int shadowColor, const wchar_t * text, ... ) = 0;
        virtual void        DrawString( int x, int y, const char * text, ... ) = 0;
        virtual void        DrawString( int x, int y, unsigned int penColor, const char * text, ... ) = 0;
        virtual void        DrawString( int x, int y, unsigned int penColor, unsigned int shadowColor, const char * text, ... ) = 0;
        //virtual void        GetTextWidth( const char * text );
        //virtual void        GetTextWidth( const wchar_t * text );
        virtual void        DrawLine( float x0, float y0, float x1, float y1, unsigned int penColor ) = 0;
        virtual void        DrawLineArrowhead( float x0, float y0, float x1, float y1, float arrowHeadSize, unsigned int penColor );
        virtual void        DrawRectangle( float x0, float y0, float width, float height, unsigned int penColor ) = 0;
        virtual void        DrawCircle( float x, float y, float radius, unsigned int penColor, float tess = 0.6f ) = 0;
        virtual void        FillRectangle( float x0, float y0, float width, float height, unsigned int brushColor ) = 0;
        //
        virtual int         GetWidth( ) = 0;
        virtual int         GetHeight( ) = 0;
        //
        inline void         DrawLine( const vaVector2 & a, vaVector2 & b, unsigned int penColor )                                                   { DrawLine( a.x, a.y, b.x, b.y, penColor ); }
        inline void         DrawLineArrowhead( const vaVector2 & a, vaVector2 & b, float arrowHeadSize, unsigned int penColor )                     { DrawLineArrowhead( a.x, a.y, b.x, b.y, arrowHeadSize, penColor ); }
    };

    class vaDebugCanvas3DBase : public vaSingletonBase < vaDebugCanvas3DBase >
    {
    protected:
        virtual ~vaDebugCanvas3DBase( ) {}
        //
    public:
        //
        virtual void        DrawLine( const vaVector3 & v0, const vaVector3 & v1, unsigned int penColor ) = 0;
        virtual void        DrawAxis( const vaVector3 & v0, float size, const vaMatrix4x4 * transform = NULL, float alpha = 0.5f ) = 0;

        virtual void        DrawBox( const vaVector3 & v0, const vaVector3 & v1, unsigned int penColor, unsigned int brushColor = 0x000000, const vaMatrix4x4 * transform = NULL ) = 0;
        virtual void        DrawTriangle( const vaVector3 & v0, const vaVector3 & v1, const vaVector3 & v2, unsigned int penColor, unsigned int brushColor = 0x000000, const vaMatrix4x4 * transform = NULL ) = 0;
        virtual void        DrawQuad( const vaVector3 & v0, const vaVector3 & v1, const vaVector3 & v2, const vaVector3 & v3, unsigned int penColor, unsigned int brushColor = 0x000000, const vaMatrix4x4 * transform = NULL ) = 0;
        virtual void        DrawSphere( const vaVector3 & center, float radius, unsigned int penColor, unsigned int brushColor = 0x000000 ) = 0;
        //
        void                DrawBox( const vaBoundingBox & aabb, unsigned int penColor, unsigned int brushColor = 0x000000, const vaMatrix4x4 * transform = NULL  );
        void                DrawBox( const vaOrientedBoundingBox & obb, unsigned int penColor, unsigned int brushColor = 0x000000 );
        void                DrawSphere( const vaBoundingSphere & bs, unsigned int penColor, unsigned int brushColor = 0x000000 );
    };

    //////////////////////////////////////////////////////////////////////////
    // Inline
    //////////////////////////////////////////////////////////////////////////

    inline void vaDebugCanvas2DBase::DrawLineArrowhead( float x0, float y0, float x1, float y1, float arrowHeadSize, unsigned int penColor )
    {
        vaVector2 lineDir = vaVector2( x1 - x0, y1 - y0 );
        float lengthSq = lineDir.LengthSq();
        if( lengthSq < VA_EPSf )
            return;

        lineDir /= vaMath::Sqrt( lengthSq );
        vaVector2 lineDirOrt = vaVector2( lineDir.y, -lineDir.x );

        vaVector2 ptFrom = vaVector2( x1, y1 );
        vaVector2 ptA   = ptFrom - lineDir * arrowHeadSize + lineDirOrt * arrowHeadSize;
        vaVector2 ptB   = ptFrom - lineDir * arrowHeadSize - lineDirOrt * arrowHeadSize;

        ptFrom += lineDir * 1.5f;

        DrawLine( ptFrom, ptA, penColor );
        DrawLine( ptFrom, ptB, penColor );
    }

    inline void vaDebugCanvas3DBase::DrawSphere( const vaBoundingSphere & bs, unsigned int penColor, unsigned int brushColor )
    {
        DrawSphere( bs.Center, bs.Radius, penColor, brushColor );
    }

    inline void vaDebugCanvas3DBase::DrawBox( const vaBoundingBox & aabb, unsigned int penColor, unsigned int brushColor, const vaMatrix4x4 * transform )
    {
        DrawBox( aabb.Min, aabb.Max(), penColor, brushColor, transform );
    }

    inline void vaDebugCanvas3DBase::DrawBox( const vaOrientedBoundingBox & obb, unsigned int penColor, unsigned int brushColor )
    {
        vaBoundingBox bb;
        vaMatrix4x4 transform;
        obb.ToAABBAndTransform( bb, transform );
        DrawBox( bb.Min, bb.Max( ), penColor, brushColor, &transform );
    }

}