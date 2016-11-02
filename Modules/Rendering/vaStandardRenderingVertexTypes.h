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

    struct vaPositionColorNormalTangentTexcoord1Vertex
    {
        vaVector3   Position;
        uint32_t    Color;
        vaVector4   Normal;
        vaVector4   Tangent;
        vaVector4   TexCoord0;
        //vaVector4   TexCoord1;

        vaPositionColorNormalTangentTexcoord1Vertex( ) { }
        //vaPositionColorNormalTangentTexcoord1Vertex( const PosColorVertex & v ) : Position( v.m_x, v.m_y, v.m_z ), Normal( 0, 1, 0, 0 ), Color( v.m_abgr ), Tangent( 0, 0, 0, 0 ), TexCoord0( 0, 0, 0, 0 ) { }
        vaPositionColorNormalTangentTexcoord1Vertex( const vaVector3 & position ) : Position( position ), Normal( vaVector4( 0, 1, 0, 0 ) ), Color( 0xFF808080 ), Tangent( 0, 0, 0, 0 ), TexCoord0( 0, 0, 0, 0 ) {}
        vaPositionColorNormalTangentTexcoord1Vertex( const vaVector3 & position, const uint32_t & color ) : Position( position ), Normal( vaVector4( 0, 1, 0, 0 ) ), Tangent( 0, 0, 0, 0 ), TexCoord0( 0, 0, 0, 0 ), Color( color ) { }
        vaPositionColorNormalTangentTexcoord1Vertex( const vaVector3 & position, const vaVector4 & normal, const uint32_t & color ) : Position( position ), Normal( normal ), Color( color ), Tangent( 0, 0, 0, 0 ), TexCoord0( 0, 0, 0, 0 ) { }
        vaPositionColorNormalTangentTexcoord1Vertex( const vaVector3 & position, const vaVector4 & normal, const vaVector4 & tangent, const vaVector4 & texCoord0, const uint32_t & color ) : Position( position ), Normal( normal ), Tangent( tangent ), TexCoord0( texCoord0 ), Color( color ) { } // , TexCoord1( vaVector4(1.0f, 0.0f, 1.0f, 0.0f) )

        bool operator ==( const vaPositionColorNormalTangentTexcoord1Vertex & cmpAgainst )
        {
            return      ( Position == cmpAgainst.Position )
                && ( Normal == cmpAgainst.Normal )
                && ( Tangent == cmpAgainst.Tangent )
                && ( TexCoord0 == cmpAgainst.TexCoord0 )
                && ( Color == cmpAgainst.Color );
        }
    };

    struct vaBillboardSprite
    {
        vaVector4   Position_CreationID;
        vaVector4   Transform2D;
        uint32      Color;
    };

    typedef vaPositionColorNormalTangentTexcoord1Vertex vaGenericSceneVertex;

}