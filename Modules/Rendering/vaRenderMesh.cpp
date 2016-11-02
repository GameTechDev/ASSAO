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

#include "vaRenderMesh.h"

#include "Rendering/vaStandardShapes.h"

using namespace VertexAsylum;

//vaRenderMeshManager & renderMeshManager, const vaGUID & uid

const int c_renderMeshFileVersion = 2;


vaRenderMesh::vaRenderMesh( vaRenderMeshManager & renderMeshManager, const vaGUID & uid ) : vaAssetResource(uid), m_trackee(renderMeshManager.GetRenderMeshTracker(), this), m_renderMeshManager( renderMeshManager )
{
    m_frontFaceWinding          = vaWindingOrder::CounterClockwise;
    m_boundingBox               = vaBoundingBox::Degenerate;
    m_tangentBitangentValid     = true;
}

void vaRenderMesh::SetTriangleMesh( const shared_ptr<StandardTriangleMesh> & mesh )
{
    m_triangleMesh = mesh;
}

void vaRenderMesh::CreateTriangleMesh( const vector<StandardVertex> & vertices, const vector<uint32> & indices )
{
    shared_ptr<StandardTriangleMesh> mesh = VA_RENDERING_MODULE_CREATE_SHARED( vaRenderMesh::StandardTriangleMesh );

    mesh->Vertices = vertices;
    mesh->Indices = indices;

    SetTriangleMesh( mesh );

    UpdateAABB( );
}

void vaRenderMesh::UpdateAABB( ) 
{ 
    if( m_triangleMesh != nullptr ) 
        m_boundingBox = vaTriangleMeshTools::CalculateBounds( m_triangleMesh->Vertices ); 
    else 
        m_boundingBox = vaBoundingBox::Degenerate; 
}

vaRenderMeshManager::vaRenderMeshManager( )
{
    m_isDestructing = false;
    m_renderMeshes.SetAddedCallback( std::bind( &vaRenderMeshManager::RenderMeshesTrackeeAddedCallback, this, std::placeholders::_1 ) );
    m_renderMeshes.SetBeforeRemovedCallback( std::bind( &vaRenderMeshManager::RenderMeshesTrackeeBeforeRemovedCallback, this, std::placeholders::_1, std::placeholders::_2 ) );

    //m_totalDrawListEntriesQueued = 0;
}

vaRenderMeshManager::~vaRenderMeshManager( )
{
    m_isDestructing = true;
    //m_renderMeshesMap.clear();

    // this must absolutely be true as they contain direct reference to this object
    assert( m_renderMeshes.size( ) == 0 );
}

shared_ptr<vaRenderMesh> vaRenderMeshManager::CreateRenderMesh( const vaGUID & uid ) 
{ 
    shared_ptr<vaRenderMesh> ret = shared_ptr<vaRenderMesh>( new vaRenderMesh( *this, uid ) ); 

    if( !ret->UIDObject_IsCorrectlyTracked() )
    {
        VA_LOG_ERROR_STACKINFO( "Error creating mesh; uid already used" );
        return nullptr;
    }

    return ret;
}

void vaRenderMeshManager::RenderMeshesTrackeeAddedCallback( int newTrackeeIndex )
{
//    m_renderMeshesDrawLists.push_back( std::vector< MeshDrawListEntry >() );
//    assert( (int)m_renderMeshesDrawLists.size()-1 == newTrackeeIndex );
}

void vaRenderMeshManager::RenderMeshesTrackeeBeforeRemovedCallback( int toBeRemovedTrackeeIndex, int toBeReplacedByTrackeeIndex )
{
//    assert( m_renderMeshesDrawLists.size( ) == m_renderMeshes.size( ) );
//    if( toBeReplacedByTrackeeIndex != -1 )
//        m_renderMeshesDrawLists[ toBeRemovedTrackeeIndex ] = m_renderMeshesDrawLists[ toBeReplacedByTrackeeIndex ];
//    m_renderMeshesDrawLists.pop_back();

    // // if ever need be, to make this faster track whether the mesh is in the library and if it is only then do the search below 
    // if( !m_isDestructing )
    // {
    //     const wstring & name = m_renderMeshes[toBeRemovedTrackeeIndex]->GetName( );
    //     const auto iterator = m_renderMeshesMap.find( name );
    //     if( iterator != m_renderMeshesMap.end( ) )
    //     {
    //         m_renderMeshesMap.erase( iterator );
    //     }
    // }
}

// void vaRenderMeshManager::ResetDrawEntries( )
// {
//     for( size_t i = 0; i < m_renderMeshesDrawLists.size(); i++ )
//         m_renderMeshesDrawLists[i].clear( );
//     m_totalDrawListEntriesQueued = 0;
// }

void vaRenderMeshManager::IHO_Draw( )
{
#ifdef VA_IMGUI_INTEGRATION_ENABLED
    static int selected = 0;
    ImGui::BeginChild( "left pane", ImVec2( 150, 0 ), true );
    for( int i = 0; i < 7; i++ )
    {
        char label[128];
        sprintf_s( label, _countof( label ), "MyObject %d", i );
        if( ImGui::Selectable( label, selected == i ) )
            selected = i;
    }
    ImGui::EndChild( );
    ImGui::SameLine( );

    // right
    ImGui::BeginGroup( );
    ImGui::BeginChild( "item view", ImVec2( 0, -ImGui::GetItemsLineHeightWithSpacing( ) ) ); // Leave room for 1 line below us
    ImGui::Text( "MyObject: %d", selected );
    ImGui::Separator( );
    ImGui::TextWrapped( "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. " );
    ImGui::EndChild( );
    ImGui::BeginChild( "buttons" );
    if( ImGui::Button( "Revert" ) ) { }
    ImGui::SameLine( );
    if( ImGui::Button( "Save" ) ) { }
    ImGui::EndChild( );
    ImGui::EndGroup( );
#endif
}

bool vaRenderMesh::Save( vaStream & outStream )
{
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<int32>( c_renderMeshFileVersion ) );

    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<int32>( (int32)m_frontFaceWinding ) );
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<bool>( m_tangentBitangentValid ) );

    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValueVector<uint32>( m_triangleMesh->Indices ) );
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValueVector<StandardVertex>( m_triangleMesh->Vertices ) );

    assert( m_parts.size( ) < INT_MAX );
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<int32>( (int32)m_parts.size( ) ) );

    for( int i = 0; i < m_parts.size( ); i++ )
    {
        VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<int32>( m_parts[i].IndexStart ) );
        VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<int32>( m_parts[i].IndexCount ) );

        auto material = m_parts[i].Material.lock();
        VERIFY_TRUE_RETURN_ON_FALSE( SaveUIDObjectUID( outStream, material ) );
    }

    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<vaBoundingBox>( m_boundingBox ) );

    return true;
}

bool vaRenderMesh::Load( vaStream & inStream )
{
    int32 fileVersion = 0;
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<int32>( fileVersion ) );
    if( !( (fileVersion >= 1 ) && (fileVersion <= c_renderMeshFileVersion) ) )
    {
        VA_LOG( L"vaRenderMesh::Load(): unsupported file version" );
        return false;
    }

    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<int32>( (int32&)m_frontFaceWinding ) );
    if( fileVersion > 1 )
        VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<bool>( m_tangentBitangentValid ) );

    shared_ptr<StandardTriangleMesh> triMesh = VA_RENDERING_MODULE_CREATE_SHARED( vaRenderMesh::StandardTriangleMesh );

    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValueVector<uint32>( triMesh->Indices ) );
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValueVector<StandardVertex>( triMesh->Vertices ) );

    SetTriangleMesh( triMesh );


    int32 partCount = 0;
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<int32>( (int32&)partCount ) );
    m_parts.resize( partCount );

    for( int i = 0; i < m_parts.size( ); i++ )
    {
        VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<int32>( m_parts[i].IndexStart ) );
        VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<int32>( m_parts[i].IndexCount ) );
        VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<vaGUID>( m_parts[i].MaterialID ) );
    }

    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<vaBoundingBox>( m_boundingBox ) );

    return true;
}

void vaRenderMesh::ReconnectDependencies( )
{
    for( size_t i = 0; i < m_parts.size(); i++ )
    {
        std::shared_ptr<vaRenderMaterial> materialSharedPtr;
        vaUIDObjectRegistrar::GetInstance( ).ReconnectDependency<vaRenderMaterial>( materialSharedPtr, m_parts[i].MaterialID );
        m_parts[i].Material = materialSharedPtr;
    }
}

shared_ptr<vaRenderMesh> vaRenderMesh::Create( const vaMatrix4x4 & transform, const std::vector<vaVector3> & vertices, const std::vector<vaVector3> & normals, const std::vector<vaVector4> & tangents, const std::vector<vaVector2> & texcoords0, const std::vector<vaVector2> & texcoords1, const std::vector<uint32> & indices, vaWindingOrder frontFaceWinding )
{
    assert( ( vertices.size( ) == normals.size( ) ) && ( vertices.size( ) == tangents.size( ) ) && ( vertices.size( ) == texcoords0.size( ) ) && ( vertices.size( ) == texcoords1.size( ) ) );

    vector<vaRenderMesh::StandardVertex> newVertices( vertices.size( ) );

    for( int i = 0; i < (int)vertices.size( ); i++ )
    {
        vaRenderMesh::StandardVertex & svert = newVertices[i];
        svert.Position = vaVector3::TransformCoord( vertices[i], transform );
        svert.Color = 0xFFFFFFFF;
        svert.Normal = vaVector4( vaVector3::TransformNormal( normals[i], transform ), 0.0f );
        svert.Tangent = vaVector4( vaVector3::TransformNormal( tangents[i].AsVec3( ), transform ), tangents[i].w );
        svert.TexCoord0 = texcoords0[i];
        svert.TexCoord1 = texcoords1[i];
    }

    shared_ptr<vaRenderMesh> mesh = vaRenderMeshManager::GetInstance( ).CreateRenderMesh( );
    if( mesh == nullptr )
    {
        assert( false );
    }
    mesh->CreateTriangleMesh( newVertices, indices );
    mesh->SetParts( vector<vaRenderMesh::SubPart>( 1, vaRenderMesh::SubPart( 0, (int)indices.size( ), weak_ptr<vaRenderMaterial>( ) ) ) );
    mesh->SetFrontFaceWindingOrder( frontFaceWinding );

    return mesh;
}

#define RMC_DEFINE_DATA                     \
    vector<vaVector3>  vertices;            \
    vector<vaVector3>  normals;             \
    vector<vaVector4>  tangents;            \
    vector<vaVector2>  texcoords0;          \
    vector<vaVector2>  texcoords1;          \
    vector<uint32>     indices; 

#define RMC_RESIZE_NTTT                     \
    normals.resize( vertices.size() );      \
    tangents.resize( vertices.size( ) );    \
    texcoords0.resize( vertices.size( ) );  \
    texcoords1.resize( vertices.size( ) );  \

shared_ptr<vaRenderMesh> vaRenderMesh::CreatePlane( const vaMatrix4x4 & transform, float sizeX, float sizeY )
{
    RMC_DEFINE_DATA;

    vaStandardShapes::CreatePlane( vertices, indices, sizeX, sizeY );
    vaWindingOrder windingOrder = vaWindingOrder::CounterClockwise;

    RMC_RESIZE_NTTT;

    vaTriangleMeshTools::GenerateNormals( normals, vertices, indices, windingOrder == vaWindingOrder::CounterClockwise );

    for( size_t i = 0; i < vertices.size( ); i++ )
    {
        tangents[i] = vaVector4( 1.0f, 0.0f, 0.0f, 1.0f );
        texcoords0[i] = vaVector2( vertices[i].x / sizeX + 0.5f, vertices[i].y / sizeY + 0.5f );
        texcoords1[i] = texcoords0[i];
    }

    return Create( transform, vertices, normals, tangents, texcoords0, texcoords1, indices, vaWindingOrder::CounterClockwise );
}

// dummy tangents, for better, http://www.terathon.com/code/tangent.html or http://developer.nvidia.com/object/NVMeshMender.html 
void FillDummyTTT( const vector<vaVector3> & vertices, const vector<vaVector3> & normals, vector<vaVector4> & tangents, vector<vaVector2> & texcoords0, vector<vaVector2> & texcoords1 )
{
    for( size_t i = 0; i < vertices.size( ); i++ )
    {
        vaVector3 bitangent = ( vertices[i] + vaVector3( 0.0f, 0.0f, -5.0f ) ).Normalize( );
        if( vaVector3::Dot( bitangent, normals[i] ) > 0.9f )
            bitangent = ( vertices[i] + vaVector3( -5.0f, 0.0f, 0.0f ) ).Normalize( );
        tangents[i] = vaVector4( vaVector3::Cross( bitangent, normals[i] ).Normalize( ), 1.0 );
        texcoords0[i] = vaVector2( vertices[i].x / 2.0f + 0.5f, vertices[i].y / 2.0f + 0.5f );
        texcoords1[i] = texcoords0[i];
    }
}

shared_ptr<vaRenderMesh> vaRenderMesh::CreateTetrahedron( const vaMatrix4x4 & transform, bool shareVertices )
{
    RMC_DEFINE_DATA;

    vaStandardShapes::CreateTetrahedron( vertices, indices, shareVertices );
    vaWindingOrder windingOrder = vaWindingOrder::Clockwise;

    RMC_RESIZE_NTTT;

    vaTriangleMeshTools::GenerateNormals( normals, vertices, indices, windingOrder == vaWindingOrder::CounterClockwise );

    FillDummyTTT( vertices, normals, tangents, texcoords0, texcoords1 );

    return Create( transform, vertices, normals, tangents, texcoords0, texcoords1, indices, windingOrder );
}

shared_ptr<vaRenderMesh> vaRenderMesh::CreateCube( const vaMatrix4x4 & transform, bool shareVertices, float edgeHalfLength )
{
    RMC_DEFINE_DATA;

    vaStandardShapes::CreateCube( vertices, indices, shareVertices, edgeHalfLength );
    vaWindingOrder windingOrder = vaWindingOrder::Clockwise;

    RMC_RESIZE_NTTT;

    vaTriangleMeshTools::GenerateNormals( normals, vertices, indices, windingOrder == vaWindingOrder::CounterClockwise );

    FillDummyTTT( vertices, normals, tangents, texcoords0, texcoords1 );

    return Create( transform, vertices, normals, tangents, texcoords0, texcoords1, indices, windingOrder );
}

shared_ptr<vaRenderMesh> vaRenderMesh::CreateOctahedron( const vaMatrix4x4 & transform, bool shareVertices )
{
    RMC_DEFINE_DATA;

    vaStandardShapes::CreateOctahedron( vertices, indices, shareVertices );
    vaWindingOrder windingOrder = vaWindingOrder::Clockwise;

    RMC_RESIZE_NTTT;

    vaTriangleMeshTools::GenerateNormals( normals, vertices, indices, windingOrder == vaWindingOrder::CounterClockwise );

    FillDummyTTT( vertices, normals, tangents, texcoords0, texcoords1 );

    return Create( transform, vertices, normals, tangents, texcoords0, texcoords1, indices, windingOrder );
}

shared_ptr<vaRenderMesh> vaRenderMesh::CreateIcosahedron( const vaMatrix4x4 & transform, bool shareVertices )
{
    RMC_DEFINE_DATA;

    vaStandardShapes::CreateIcosahedron( vertices, indices, shareVertices );
    vaWindingOrder windingOrder = vaWindingOrder::Clockwise;

    RMC_RESIZE_NTTT;

    vaTriangleMeshTools::GenerateNormals( normals, vertices, indices, windingOrder == vaWindingOrder::CounterClockwise );

    FillDummyTTT( vertices, normals, tangents, texcoords0, texcoords1 );

    return Create( transform, vertices, normals, tangents, texcoords0, texcoords1, indices, windingOrder );
}

shared_ptr<vaRenderMesh> vaRenderMesh::CreateDodecahedron( const vaMatrix4x4 & transform, bool shareVertices )
{
    RMC_DEFINE_DATA;

    vaStandardShapes::CreateDodecahedron( vertices, indices, shareVertices );
    vaWindingOrder windingOrder = vaWindingOrder::Clockwise;

    RMC_RESIZE_NTTT;

    vaTriangleMeshTools::GenerateNormals( normals, vertices, indices, windingOrder == vaWindingOrder::CounterClockwise );

    FillDummyTTT( vertices, normals, tangents, texcoords0, texcoords1 );

    return Create( transform, vertices, normals, tangents, texcoords0, texcoords1, indices, windingOrder );
}

shared_ptr<vaRenderMesh> vaRenderMesh::CreateSphere( const vaMatrix4x4 & transform, int tessellationLevel, bool shareVertices )
{
    RMC_DEFINE_DATA;

    vaStandardShapes::CreateSphere( vertices, indices, tessellationLevel, shareVertices );
    vaWindingOrder windingOrder = vaWindingOrder::Clockwise;

    RMC_RESIZE_NTTT;

    vaTriangleMeshTools::GenerateNormals( normals, vertices, indices, windingOrder == vaWindingOrder::CounterClockwise );

    FillDummyTTT( vertices, normals, tangents, texcoords0, texcoords1 );

    return Create( transform, vertices, normals, tangents, texcoords0, texcoords1, indices, windingOrder );
}

shared_ptr<vaRenderMesh> vaRenderMesh::CreateCylinder( const vaMatrix4x4 & transform, float height, float radiusBottom, float radiusTop, int tessellation, bool openTopBottom, bool shareVertices )
{
    RMC_DEFINE_DATA;

    vaStandardShapes::CreateCylinder( vertices, indices, height, radiusBottom, radiusTop, tessellation, openTopBottom, shareVertices );
    vaWindingOrder windingOrder = vaWindingOrder::Clockwise;

    RMC_RESIZE_NTTT;

    vaTriangleMeshTools::GenerateNormals( normals, vertices, indices, windingOrder == vaWindingOrder::CounterClockwise );

    FillDummyTTT( vertices, normals, tangents, texcoords0, texcoords1 );

    return Create( transform, vertices, normals, tangents, texcoords0, texcoords1, indices, windingOrder );
}

shared_ptr<vaRenderMesh> vaRenderMesh::CreateTeapot( const vaMatrix4x4 & transform )
{
    RMC_DEFINE_DATA;

    vaStandardShapes::CreateTeapot( vertices, indices );
    vaWindingOrder windingOrder = vaWindingOrder::Clockwise;

    RMC_RESIZE_NTTT;

    vaTriangleMeshTools::GenerateNormals( normals, vertices, indices, windingOrder == vaWindingOrder::CounterClockwise );

    FillDummyTTT( vertices, normals, tangents, texcoords0, texcoords1 );

    return Create( transform, vertices, normals, tangents, texcoords0, texcoords1, indices, windingOrder );
}

