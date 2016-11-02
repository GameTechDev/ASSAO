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

#include "vaTriangleMesh.h"
#include "vaTexture.h"

#include "Rendering/Media/Shaders/vaSharedTypes.h"

#include "IntegratedExternals/vaImguiIntegration.h"

#include "vaRenderMaterial.h"

// vaRenderMesh and vaRenderMeshManager are a generic render mesh implementation

namespace VertexAsylum
{
    class vaRenderMeshManager;

    class vaRenderMesh : public vaAssetResource
    {
    public:
        // only standard mesh storage supported at the moment
        struct StandardVertex
        {
            // first 4 bytes
            vaVector3   Position;
            uint32_t    Color;

            // next 4 bytes (.w not encoded - can be used for skinning indices for example; should probably be compressed to 16bit floats on the rendering side)
            vaVector4   Normal;

            // next 4 bytes (.w stores -1/1 handedness for determining bitangent)
            vaVector4   Tangent;

            // next 2 bytes (first UVs; should probably be compressed to 16bit floats on the rendering side)
            vaVector2   TexCoord0;

            // next 2 bytes (second UVs; should probably be compressed to 16bit floats on the rendering side)
            vaVector2   TexCoord1;

            StandardVertex( ) { }
            StandardVertex( const vaVector3 & position ) : Position( position ), Normal( vaVector4( 0, 1, 0, 0 ) ), Color( 0xFF808080 ), Tangent( 0, 0, 0, 0 ), TexCoord0( 0, 0 ), TexCoord1( 0, 0 ) {}
            StandardVertex( const vaVector3 & position, const uint32_t & color ) : Position( position ), Normal( vaVector4( 0, 1, 0, 0 ) ), Tangent( 0, 0, 0, 0 ), TexCoord0( 0, 0 ), TexCoord1( 0, 0 ), Color( color ) { }
            StandardVertex( const vaVector3 & position, const vaVector4 & normal, const uint32_t & color ) : Position( position ), Normal( normal ), Color( color ), Tangent( 0, 0, 0, 0 ), TexCoord0( 0, 0 ), TexCoord1( 0, 0 ) { }
            StandardVertex( const vaVector3 & position, const vaVector4 & normal, const vaVector4 & tangent, const vaVector2 & texCoord0, const uint32_t & color ) : Position( position ), Normal( normal ), Tangent( tangent ), TexCoord0( texCoord0 ), TexCoord1( 0, 0 ), Color( color ) { }
            StandardVertex( const vaVector3 & position, const vaVector4 & normal, const vaVector4 & tangent, const vaVector2 & texCoord0, const vaVector2 & texCoord1, const uint32_t & color ) : Position( position ), Normal( normal ), Tangent( tangent ), TexCoord0( texCoord0 ), TexCoord1( texCoord1 ), Color( color ) { }

            bool operator ==( const StandardVertex & cmpAgainst )
            {
                return      ( Position == cmpAgainst.Position ) && ( Normal == cmpAgainst.Normal ) && ( Tangent == cmpAgainst.Tangent ) && ( TexCoord0 == cmpAgainst.TexCoord0 ) && ( TexCoord1 == cmpAgainst.TexCoord1 ) && ( Color == cmpAgainst.Color );
            }
        };

        struct StandardVertexAnimationPart
        {
            uint32      Indices;    // (8888_UINT)
            uint32      Weights;    // (8888_UNORM)
        };

        //static const int                                c_maxSubParts   = 32;

        // This will most likely go out in VA_02; only one part per mesh will be supported in order to increase simplicity on the rendering backend, allow for easier instancing, etc.
        struct SubPart
        {
            // not sure if subpart name is needed - Mesh and Material will have names, should be enough? still, I'll leave it here for future
            // static const int                            cNameMaxLength          = 32;
            // char                                        Name[cNameMaxLength];

            int                                         IndexStart;
            int                                         IndexCount;
            weak_ptr<vaRenderMaterial>                  Material;
            vaGUID                                      MaterialID;     // used during loading - could be moved into a separate structure and disposed of after loading

            SubPart( ) { }
            SubPart( int indexStart, int indexCount, const weak_ptr<vaRenderMaterial> & material ) : IndexStart( indexStart ), IndexCount( indexCount ), Material( material ) { }
        };

        typedef vaTriangleMesh<StandardVertex>          StandardTriangleMesh;


    private:
        // wstring const                                   m_name;                 // unique (within renderMeshManager) name
        vaTT_Trackee< vaRenderMesh * >                  m_trackee;
        vaRenderMeshManager &                           m_renderMeshManager;

        vaWindingOrder                                  m_frontFaceWinding;
        bool                                            m_tangentBitangentValid;

        shared_ptr<StandardTriangleMesh>                m_triangleMesh;

        // This will most likely go out in VA_02; only one part per mesh will be supported in order to increase simplicity on the rendering backend, allow for easier instancing, etc.
        vector<SubPart>                                 m_parts;

        vaBoundingBox                                   m_boundingBox;      // local bounding box around the mesh

    protected:
        friend class vaRenderMeshManager;
        vaRenderMesh( vaRenderMeshManager & renderMeshManager, const vaGUID & uid );
    public:
        virtual ~vaRenderMesh( )                        { }

    public:
        //const wstring &                                 GetName( ) const                                    { return m_name; };

        vaRenderMeshManager &                           GetManager( ) const                                 { return m_renderMeshManager; }
        int                                             GetListIndex( ) const                               { return m_trackee.GetIndex( ); }

        const vaBoundingBox &                           GetAABB( ) const                                    { return m_boundingBox; }

        // This will most likely go out in VA_02; only one part per mesh will be supported in order to increase simplicity on the rendering backend, allow for easier instancing, etc.
        const vector<SubPart> &                         GetParts( ) const                                   { return m_parts; }
        void                                            SetParts( const vector<SubPart> & parts )           { m_parts = parts; } //assert( parts.size() <= c_maxSubParts ); }

        const shared_ptr<StandardTriangleMesh> &        GetTriangleMesh(  ) const                           { return m_triangleMesh; }
        void                                            SetTriangleMesh( const shared_ptr<StandardTriangleMesh> & mesh );
        void                                            CreateTriangleMesh( const vector<StandardVertex> & vertices, const vector<uint32> & indices );

        vaWindingOrder                                  GetFrontFaceWindingOrder( ) const                   { return m_frontFaceWinding; }
        void                                            SetFrontFaceWindingOrder( vaWindingOrder winding )  { m_frontFaceWinding = winding; }

        bool                                            GetTangentBitangentValid( ) const                   { return m_tangentBitangentValid; }
        void                                            SetTangentBitangentValid( bool value )              { m_tangentBitangentValid = value; }

        void                                            UpdateAABB( );

        bool                                            Save( vaStream & outStream );
        bool                                            Load( vaStream & inStream );
        virtual void                                    ReconnectDependencies( );

        // create mesh with normals, with provided vertices & indices
        static shared_ptr<vaRenderMesh>                 Create( const vaMatrix4x4 & transform, const std::vector<vaVector3> & vertices, const std::vector<vaVector3> & normals, const std::vector<vaVector4> & tangents, const std::vector<vaVector2> & texcoords0, const std::vector<vaVector2> & texcoords1, const std::vector<uint32> & indices, vaWindingOrder frontFaceWinding = vaWindingOrder::CounterClockwise );

        // these use vaStandardShapes::Create* functions and create shapes with center in (0, 0, 0) and each vertex magnitude of 1 (normalized), except where specified otherwise, and then transformed by the provided transform
        static shared_ptr<vaRenderMesh>                 CreatePlane( const vaMatrix4x4 & transform, float sizeX = 1.0f, float sizeY = 1.0f );
        static shared_ptr<vaRenderMesh>                 CreateTetrahedron( const vaMatrix4x4 & transform, bool shareVertices );
        static shared_ptr<vaRenderMesh>                 CreateCube( const vaMatrix4x4 & transform, bool shareVertices, float edgeHalfLength = 0.7071067811865475f );
        static shared_ptr<vaRenderMesh>                 CreateOctahedron( const vaMatrix4x4 & transform, bool shareVertices );
        static shared_ptr<vaRenderMesh>                 CreateIcosahedron( const vaMatrix4x4 & transform, bool shareVertices );
        static shared_ptr<vaRenderMesh>                 CreateDodecahedron( const vaMatrix4x4 & transform, bool shareVertices );
        static shared_ptr<vaRenderMesh>                 CreateSphere( const vaMatrix4x4 & transform, int tessellationLevel, bool shareVertices );
        static shared_ptr<vaRenderMesh>                 CreateCylinder( const vaMatrix4x4 & transform, float height, float radiusBottom, float radiusTop, int tessellation, bool openTopBottom, bool shareVertices );
        static shared_ptr<vaRenderMesh>                 CreateTeapot( const vaMatrix4x4 & transform );
    };

    class vaRenderMeshDrawList
    {
    public:
        struct Entry
        {
            std::shared_ptr<vaRenderMesh>               Mesh;
            vaVector3                                   Translation;
            vaVector3                                   Scale;
            vaQuaternion                                Rotation;
            uint32                                      SubPartMask;

            // per-instance data; could be a color but it could be an index into more elaborate data storage somewhere, etc.
            vaVector4                                   Color;

            Entry( const std::shared_ptr<vaRenderMesh> & mesh, const vaVector3 & translation, const vaQuaternion & rotation, const vaVector4 & color = vaVector4( 1.0f, 1.0f, 1.0f, 1.0f ), uint32 subPartMask = 0xFFFFFFFF ) : Mesh( mesh ), Translation( translation ), Rotation( rotation ), Color( color ), SubPartMask( subPartMask ), Scale( 1.0f, 1.0f, 1.0f ) { }
            Entry( const std::shared_ptr<vaRenderMesh> & mesh, const vaVector3 & scale, const vaVector3 & translation, const vaQuaternion & rotation, const vaVector4 & color = vaVector4( 1.0f, 1.0f, 1.0f, 1.0f ), uint32 subPartMask = 0xFFFFFFFF ) : Mesh( mesh ), Scale( scale ), Translation( translation ), Rotation( rotation ), Color( color ), SubPartMask( subPartMask ) { }
        };

    private:
        vector< Entry >                                 m_drawList;

    protected:

    public:
        void                                            Reset( )                            { m_drawList.clear(); }
        int                                             Count( ) const                      { return (int)m_drawList.size(); }
        void                                            Insert( const std::shared_ptr<vaRenderMesh> & mesh, const vaVector3 & scale, const vaVector3 & translation, const vaQuaternion & rotation, const vaVector4 & color = vaVector4( 1.0f, 1.0f, 1.0f, 1.0f ), uint32 subPartMask = 0xFFFFFFFF );
        void                                            Insert( const std::shared_ptr<vaRenderMesh> & mesh, const vaVector3 & translation, const vaQuaternion & rotation, const vaVector4 & color = vaVector4( 1.0f, 1.0f, 1.0f, 1.0f ), uint32 subPartMask = 0xFFFFFFFF );
        void                                            Insert( const std::shared_ptr<vaRenderMesh> & mesh, const vaMatrix4x4 & transform, const vaVector4 & color = vaVector4( 1.0f, 1.0f, 1.0f, 1.0f ), uint32 subPartMask = 0xFFFFFFFF );

        const Entry &                                   operator[] ( int index ) const      { return m_drawList[index]; }
    };

    class vaRenderMeshManager : public vaRenderingModule, public vaImguiHierarchyObject, public vaSingletonBase< vaRenderMeshManager >
    {
        VA_RENDERING_MODULE_MAKE_FRIENDS( );
    protected:

    public:

    protected:
        vaTT_Tracker< vaRenderMesh * >                  m_renderMeshes;
        //vector< vector< MeshDrawListEntry > >           m_renderMeshesDrawLists;     // at the moment we only have one per vaRenderMeshManager, but they could be separate -> need to split them into vaRenderMeshList entities
        //int                                             m_totalDrawListEntriesQueued;


        // map<wstring, shared_ptr<vaRenderMesh>>          m_renderMeshesMap;

        bool                                            m_isDestructing;

    public:
        friend class vaRenderingCore;
        vaRenderMeshManager( );
        virtual ~vaRenderMeshManager( );

    private:
        friend class MVMSimpleMeshManagerDX11;

    private:
        void                                            RenderMeshesTrackeeAddedCallback( int newTrackeeIndex );
        void                                            RenderMeshesTrackeeBeforeRemovedCallback( int removedTrackeeIndex, int replacedByTrackeeIndex );

    public:
        virtual void                                    Draw( vaDrawContext & drawContext, const vaRenderMeshDrawList & list ) = 0;

        vaTT_Tracker< vaRenderMesh * > *                GetRenderMeshTracker( )                                                     { return &m_renderMeshes; }

        shared_ptr<vaRenderMesh>                        CreateRenderMesh( const vaGUID & uid = vaCore::GUIDCreate() );

    protected:
        virtual string                                  IHO_GetInstanceInfo( ) const                                                { return vaStringTools::Format("vaRenderMeshManager (%d meshes)", m_renderMeshes.size() ); }
        virtual void                                    IHO_Draw( );
    };

    inline void vaRenderMeshDrawList::Insert( const std::shared_ptr<vaRenderMesh> & mesh, const vaVector3 & scale, const vaVector3 & translation, const vaQuaternion & rotation, const vaVector4 & color, uint32 subPartMask )
    {
        m_drawList.push_back( Entry( mesh, scale, translation, rotation, color, subPartMask ) );
    }

    inline void vaRenderMeshDrawList::Insert( const std::shared_ptr<vaRenderMesh> & mesh, const vaVector3 & translation, const vaQuaternion & rotation, const vaVector4 & color, uint32 subPartMask )
    {
        m_drawList.push_back( Entry( mesh, translation, rotation, color, subPartMask ) );
    }

    inline void vaRenderMeshDrawList::Insert( const std::shared_ptr<vaRenderMesh> & mesh, const vaMatrix4x4 & transform, const vaVector4 & color, uint32 subPartMask ) 
    { 
        vaVector3 scale, translation; vaQuaternion rotation; 
        transform.Decompose( scale, rotation, translation ); 
        Insert( mesh, scale, translation, rotation, color, subPartMask ); 
    }

    inline vaTriangleMesh<vaRenderMesh::StandardVertex>::vaTriangleMesh( )
    {
        assert( vaRenderingCore::IsInitialized( ) );
    }

}