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

#include "vaRendering.h"

#include "vaStandardRenderingVertexTypes.h"

namespace VertexAsylum
{
    class vaTriangleMeshTools
    {
        vaTriangleMeshTools( ) { }

    public:

        template< class VertexType >
        static inline int AddVertex( std::vector<VertexType> & outVertices, const VertexType & vert )
        {
            outVertices.push_back( vert );
            return (int)outVertices.size( ) - 1;
        }

        template< class VertexType >
        static inline int FindOrAdd( std::vector<VertexType> & vertices, const VertexType & vert, int searchForDuplicates )
        {
            for( int i = (int)( vertices.size( ) ) - 1; ( i >= 0 ) && ( i >= (int)vertices.size( ) - searchForDuplicates ); i-- )
            {
                if( vertices[i] == vert )
                    return i;
            }

            vertices.push_back( vert );
            return (int)( vertices.size( ) ) - 1;
        }

        static inline void AddTriangle( std::vector<uint32> & outIndices, int a, int b, int c )
        {
            assert( ( a >= 0 ) && ( b >= 0 ) && ( c >= 0 ) );
            outIndices.push_back( a );
            outIndices.push_back( b );
            outIndices.push_back( c );
        }

        template< class VertexType >
        static inline void AddTriangle( std::vector<VertexType> & outVertices, std::vector<uint32> & outIndices, const VertexType & v0, const VertexType & v1, const VertexType & v2 )
        {
            int i0 = AddVertex( outVertices, v0 );
            int i1 = AddVertex( outVertices, v1 );
            int i2 = AddVertex( outVertices, v2 );

            AddTriangle( outIndices, i0, i1, i2 );
        }

        template< class VertexType >
        static inline void AddTriangle_MergeSamePositionVertices( std::vector<VertexType> & outVertices, std::vector<uint32> & outIndices, const VertexType & v0, const VertexType & v1, const VertexType & v2, int vertexMergingLookFromVertexOffset = 0 )
        {
            int i0 = FindOrAdd( outVertices, v0, (int)outVertices.size() - vertexMergingLookFromVertexOffset );
            int i1 = FindOrAdd( outVertices, v1, (int)outVertices.size() - vertexMergingLookFromVertexOffset );
            int i2 = FindOrAdd( outVertices, v2, (int)outVertices.size() - vertexMergingLookFromVertexOffset );

            AddTriangle( outIndices, i0, i1, i2 );
        }

        // This adds quad triangles in strip order ( (0, 0), (1, 0), (0, 1), (1, 1) ) - so swap the last two if doing clockwise/counterclockwise
        // (this is a bit inconsistent with AddPentagon below)
        static inline void AddQuad( std::vector<uint32> & outIndices, int i0, int i1, int i2, int i3 )
        {
            outIndices.push_back( i0 );
            outIndices.push_back( i1 );
            outIndices.push_back( i2 );
            outIndices.push_back( i1 );
            outIndices.push_back( i3 );
            outIndices.push_back( i2 );
        }

        // This adds quad triangles in strip order ( (0, 0), (1, 0), (0, 1), (1, 1) ) - so swap the last two if doing clockwise/counterclockwise
        // (this is a bit inconsistent with AddPentagon below)
        template< class VertexType >
        static inline void AddQuad( std::vector<VertexType> & outVertices, std::vector<uint32> & outIndices, const VertexType & v0, const VertexType & v1, const VertexType & v2, const VertexType & v3 )
        {
            int i0 = AddVertex( outVertices, v0 );
            int i1 = AddVertex( outVertices, v1 );
            int i2 = AddVertex( outVertices, v2 );
            int i3 = AddVertex( outVertices, v3 );

            AddQuad( outIndices, i0, i1, i2, i3 );
        }

        // This adds triangles in clockwise, fan-like order
        // (this is a bit inconsistent with AddQuad above)
        static inline void AddPentagon( std::vector<uint32> & outIndices, int i0, int i1, int i2, int i3, int i4 )
        {
            AddTriangle( outIndices, i0, i1, i2 );
            AddTriangle( outIndices, i0, i2, i3 );
            AddTriangle( outIndices, i0, i3, i4 );
        }

        // This adds triangles in clockwise, fan-like order
        // (this is a bit inconsistent with AddQuad above)
        template< class VertexType >
        static inline void AddPentagon( std::vector<VertexType> & outVertices, std::vector<uint32> & outIndices, const VertexType & v0, const VertexType & v1, const VertexType & v2, const VertexType & v3, const VertexType & v4 )
        {
            int i0 = AddVertex( outVertices, v0 );
            int i1 = AddVertex( outVertices, v1 );
            int i2 = AddVertex( outVertices, v2 );
            int i3 = AddVertex( outVertices, v3 );
            int i4 = AddVertex( outVertices, v4 );

            AddTriangle( outIndices, i0, i1, i2 );
            AddTriangle( outIndices, i0, i2, i3 );
            AddTriangle( outIndices, i0, i3, i4 );
        }

        template< class VertexType >
        static inline void TransformPositions( std::vector<VertexType> & vertices, const vaMatrix4x4 & transform )
        {
            for( int i = 0; i < (int)vertices.size( ); i++ )
                vertices[i].Position = vaVector3::TransformCoord( vertices[i].Position, transform );
        }

        static inline void GenerateNormals( std::vector<vaVector3> & outNormals, const std::vector<vaVector3> & vertices, const std::vector<uint32> & indices, bool counterClockwise, int indexFrom = 0, int indexCount = -1, bool fixBrokenNormals = true )
        {
            assert( outNormals.size() == vertices.size() );
            if( indexCount == -1 )
                indexCount = (int)indices.size( );

            for( int i = 0; i < (int)vertices.size( ); i++ )
                outNormals[i] = vaVector3( 0, 0, 0 );

            for( int i = indexFrom; i < indexCount; i += 3 )
            {
                const vaVector3 & a = vertices[indices[i + 0]];
                const vaVector3 & b = vertices[indices[i + 1]];
                const vaVector3 & c = vertices[indices[i + 2]];

                vaVector3 norm;
                if( counterClockwise )
                    norm = vaVector3::Cross( c - a, b - a );
                else
                    norm = vaVector3::Cross( b - a, c - a );

                float triAreaX2 = norm.Length( );
                if( triAreaX2 < VA_EPSf ) 
                {
                    if( !fixBrokenNormals )
                        continue;

                    if( triAreaX2 != 0.0f )
                        norm /= triAreaX2 * 10000.0f;
                }

                // don't normalize, leave it weighted by area
                outNormals[indices[i + 0]] += norm;
                outNormals[indices[i + 1]] += norm;
                outNormals[indices[i + 2]] += norm;
            }

            for( int i = 0; i < (int)vertices.size( ); i++ )
            {
                float length = outNormals[i].Length();

                if( length < VA_EPSf )
                    outNormals[i] = vaVector3( 0.0f, 0.0f, (fixBrokenNormals)?(1.0f):(0.0f) );
                else
                    outNormals[i] *= 1.0f / length;
            }
        }

        template< class VertexType >
        static inline void GenerateNormals( std::vector<VertexType> & vertices, const std::vector<uint32> & indices, bool counterClockwise, int indexFrom = 0, int indexCount = -1 )
        {
            if( indexCount == -1 )
                indexCount = (int)indices.size();
            for( int i = 0; i < (int)vertices.size( ); i++ )
                vertices[i].Normal = vaVector4( 0, 0, 0, 0 );

            for( int i = indexFrom; i < indexCount; i += 3 )
            {
                const vaVector3 & a = vertices[indices[i + 0]].Position;
                const vaVector3 & b = vertices[indices[i + 1]].Position;
                const vaVector3 & c = vertices[indices[i + 2]].Position;

                vaVector3 norm;
                if( counterClockwise )
                    norm = vaVector3::Cross( c - a, b - a );
                else
                    norm = vaVector3::Cross( b - a, c - a );

                float triAreaX2 = norm.Length( );
                if( triAreaX2 < VA_EPSf ) continue;

                // don't normalize, leave it weighted by area
                vertices[indices[i + 0]].Normal.AsVec3( ) += norm;
                vertices[indices[i + 1]].Normal.AsVec3( ) += norm;
                vertices[indices[i + 2]].Normal.AsVec3( ) += norm;
            }

            for( int i = 0; i < (int)vertices.size( ); i++ )
                vertices[i].Normal = vertices[i].Normal.Normalize( );
        }

        // based on http://www.terathon.com/code/tangent.html
        static void GenerateTangents( std::vector<vaVector4> & outTangents, const std::vector<vaVector3> & vertices, const std::vector<vaVector3> & normals, const std::vector<vaVector2> & UVs, const std::vector<uint32> & indices ) 
        {
            assert( false ); // code not tested

            assert( outTangents.size( ) == vertices.size( ) );

            std::vector<vaVector3> tempTans;
            tempTans.resize( vertices.size() * 2, vaVector3( 0.0f, 0.0f, 0.0f ) );

            vaVector3 * tan1 = &tempTans[0];
            vaVector3 * tan2 = &tempTans[vertices.size()];

            assert( (indices.size() % 3) == 0 );
            int triangleCount = (int)indices.size() / 3;

            for( long a = 0; a < triangleCount; a++ )
            {
                long i1 = indices[a*3+0];
                long i2 = indices[a*3+1];
                long i3 = indices[a*3+2];

                const vaVector3 & v1 = vertices[i1];
                const vaVector3 & v2 = vertices[i2];
                const vaVector3 & v3 = vertices[i3];

                const vaVector2 & w1 = UVs[i1];
                const vaVector2 & w2 = UVs[i2];
                const vaVector2 & w3 = UVs[i3];

                float x1 = v2.x - v1.x;
                float x2 = v3.x - v1.x;
                float y1 = v2.y - v1.y;
                float y2 = v3.y - v1.y;
                float z1 = v2.z - v1.z;
                float z2 = v3.z - v1.z;

                float s1 = w2.x - w1.x;
                float s2 = w3.x - w1.x;
                float t1 = w2.y - w1.y;
                float t2 = w3.y - w1.y;

                float r = 1.0F / ( s1 * t2 - s2 * t1 );
                vaVector3 sdir( ( t2 * x1 - t1 * x2 ) * r, ( t2 * y1 - t1 * y2 ) * r, ( t2 * z1 - t1 * z2 ) * r );
                vaVector3 tdir( ( s1 * x2 - s2 * x1 ) * r, ( s1 * y2 - s2 * y1 ) * r, ( s1 * z2 - s2 * z1 ) * r );

                tan1[i1] += sdir;
                tan1[i2] += sdir;
                tan1[i3] += sdir;

                tan2[i1] += tdir;
                tan2[i2] += tdir;
                tan2[i3] += tdir;
            }

            for( long a = 0; a < vertices.size(); a++ )
            {
                const vaVector3 & n = normals[a];
                const vaVector3 & t = tan1[a];

                // Gram-Schmidt orthogonalize
                outTangents[a] = vaVector4( ( t - n * vaVector3::Dot( n, t ) ).Normalize( ), 1.0f );

                // Calculate handedness
                outTangents[a].w = (vaVector3::Dot( vaVector3::Cross( n, t ), tan2[a] ) < 0.0f) ? ( -1.0f ) : ( 1.0f );
            }
        }


        template< typename VertexType >
        static inline vaBoundingBox CalculateBounds( const std::vector<VertexType> & vertices )
        {
            vaVector3 bmin( FLT_MAX, FLT_MAX, FLT_MAX ), bmax( FLT_MIN, FLT_MIN, FLT_MIN );

            for( size_t i = 0; i < vertices.size( ); i++ )
            {
                bmin = vaVector3::ComponentMin( bmin, vertices[i].Position );
                bmax = vaVector3::ComponentMax( bmax, vertices[i].Position );
            }

            return vaBoundingBox( bmin, bmax - bmin );
        }

        template< class VertexType >
        static inline void ConcatenatePositionOnlyMesh( std::vector<VertexType> & outVertices, std::vector<uint32> & outIndices, std::vector<vaVector3> & inVertices, std::vector<uint32> & inIndices )
        {
            uint32 startingVertex = (uint32)outVertices.size();

            for( int i = 0; i < inVertices.size(); i++ )
            {
                VertexType nv;
                nv.Position = inVertices[i];
                outVertices.push_back( nv );
            }
            for( int i = 0; i < inIndices.size( ); i++ )
                outIndices.push_back( inIndices[i] + startingVertex );
        }

    };

    template< class VertexType >
    class vaTriangleMesh : public vaRenderingModule
    {
        VA_RENDERING_MODULE_MAKE_FRIENDS( );

    public:
        std::vector<VertexType>             Vertices;
        std::vector<uint32>                 Indices;

    protected:
        vaTriangleMesh( );

    public:
        virtual     ~vaTriangleMesh( )    { }

        void        Reset( )
        {
            Vertices.clear( );
            Indices.clear( );
        }

        int                                 AddVertex( const VertexType & vert )
        {
            return vaTriangleMeshTools::AddVertex( Vertices, vert );
        }

        void                                AddTriangle( int a, int b, int c )
        {
            vaTriangleMeshTools::AddTriangle( Indices, a, b, c );
        }

        template< class VertexType >
        inline void                         AddQuad( const VertexType & v0, const VertexType & v1, const VertexType & v2, const VertexType & v3 )
        {
            vaTriangleMeshTools::AddQuad( Vertices, Indices, v0, v1, v2, v3 );
        }

        template< class VertexType >
        inline void                         AddTriangle( const VertexType & v0, const VertexType & v1, const VertexType & v2 )
        {
            vaTriangleMeshTools::AddTriangle( Vertices, Indices, v0, v1, v2 );
        }

        //template< typename VertexType >
        inline vaBoundingBox                CalculateBounds( )
        {
            return vaTriangleMeshTools::CalculateBounds( Vertices );
        }

        void                                GenerateNormals( bool counterClockwise )
        {
            vaTriangleMeshTools::GenerateNormals( Vertices, Indices, counterClockwise );
        }

        virtual void                        SetDataDirty( )                                                                                            = 0;
    };

    typedef vaTriangleMesh<vaPositionColorNormalTangentTexcoord1Vertex> vaTriangleMesh_PositionColorNormalTangentTexcoord1Vertex;

    inline vaTriangleMesh<vaPositionColorNormalTangentTexcoord1Vertex>::vaTriangleMesh( )
    {
        assert( vaRenderingCore::IsInitialized() );
    }

}