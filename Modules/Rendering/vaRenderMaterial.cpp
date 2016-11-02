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

const int c_renderMeshMaterialFileVersion = 2;


vaRenderMaterial::vaRenderMaterial( const vaConstructorParamsBase * params ) : 
    vaAssetResource( vaSaferStaticCast< const vaRenderMaterialConstructorParams *, const vaConstructorParamsBase *>( params )->UID), 
    m_trackee( vaSaferStaticCast< const vaRenderMaterialConstructorParams *, const vaConstructorParamsBase *>( params )->RenderMaterialManager.GetRenderMaterialTracker( ), this ), 
    m_renderMaterialManager( vaSaferStaticCast< const vaRenderMaterialConstructorParams *, const vaConstructorParamsBase *>( params )->RenderMaterialManager )
{
    m_shaderFileName            = L"vaRenderMesh.hlsl";
    m_shaderEntryVS_PosOnly     = "VS_PosOnly";
    m_shaderEntryPS_DepthOnly   = "PS_DepthOnly";
    m_shaderEntryVS_Standard    = "VS_Standard";
    m_shaderEntryPS_Forward     = "PS_Forward";
    m_shaderEntryPS_Deferred    = "PS_Deferred";

    m_shaderMacros.reserve( 16 );
    m_shaderMacrosDirty = true;
    m_shadersDirty = true;

    m_textureAlbedoUID          = vaCore::GUIDNull();
    m_textureNormalmapUID       = vaCore::GUIDNull();
    m_textureSpecularUID        = vaCore::GUIDNull();
    m_textureEmissiveUID        = vaCore::GUIDNull();
}

void vaRenderMaterial::UpdateShaderMacros( )
{
    if( !m_shaderMacrosDirty )
        return;

    vector< pair< string, string > > prevShaderMacros = m_shaderMacros;

    assert( m_shaderMacrosDirty );
    m_shaderMacros.clear();

    m_shaderMacros.push_back( std::pair<std::string, std::string>( "VA_RMM_TRANSPARENT",        ( ( m_settings.Transparent       ) ? ( "1" ) : ( "0" ) ) ) );
    m_shaderMacros.push_back( std::pair<std::string, std::string>( "VA_RMM_ALPHATEST",          ( ( m_settings.AlphaTest         ) ? ( "1" ) : ( "0" ) ) ) );
    m_shaderMacros.push_back( std::pair<std::string, std::string>( "VA_RMM_ACCEPTSHADOWS",      ( ( m_settings.ReceiveShadows    ) ? ( "1" ) : ( "0" ) ) ) );
    m_shaderMacros.push_back( std::pair<std::string, std::string>( "VA_RMM_WIREFRAME",          ( ( m_settings.Wireframe         ) ? ( "1" ) : ( "0" ) ) ) );

    bool texturingEnabled = !m_renderMaterialManager.GetTexturingDisabled();

    m_shaderMacros.push_back( std::pair<std::string, std::string>( "VA_RMM_HASALBEDOTEXTURE",   ( ( texturingEnabled & (m_textureAlbedo != nullptr    ) ) ? ( "1" ) : ( "0" ) ) ) );
    m_shaderMacros.push_back( std::pair<std::string, std::string>( "VA_RMM_HASNORMALMAPTEXTURE",( ( texturingEnabled & (m_textureNormalmap != nullptr ) ) ? ( "1" ) : ( "0" ) ) ) );
    m_shaderMacros.push_back( std::pair<std::string, std::string>( "VA_RMM_HASSPECULARTEXTURE", ( ( texturingEnabled & (m_textureSpecular != nullptr  ) ) ? ( "1" ) : ( "0" ) ) ) );
    m_shaderMacros.push_back( std::pair<std::string, std::string>( "VA_RMM_HASEMISSIVETEXTURE", ( ( texturingEnabled & (m_textureEmissive != nullptr  ) ) ? ( "1" ) : ( "0" ) ) ) );

    m_shaderMacrosDirty = false;
    m_shadersDirty = prevShaderMacros != m_shaderMacros;
}


bool vaRenderMaterial::Save( vaStream & outStream )
{
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<int32>( c_renderMeshMaterialFileVersion ) );

    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<MaterialSettings>( m_settings ) );
    
    VERIFY_TRUE_RETURN_ON_FALSE( SaveUIDObjectUID( outStream, m_textureAlbedo ) );
    VERIFY_TRUE_RETURN_ON_FALSE( SaveUIDObjectUID( outStream, m_textureNormalmap ) );
    VERIFY_TRUE_RETURN_ON_FALSE( SaveUIDObjectUID( outStream, m_textureSpecular ) );
    VERIFY_TRUE_RETURN_ON_FALSE( SaveUIDObjectUID( outStream, m_textureEmissive ) );

    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteString( m_shaderFileName )          );
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteString( m_shaderEntryVS_PosOnly )   );
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteString( m_shaderEntryPS_DepthOnly ) );
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteString( m_shaderEntryVS_Standard )  );
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteString( m_shaderEntryPS_Forward )   );
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteString( m_shaderEntryPS_Deferred )  );

    return true;
}

bool vaRenderMaterial::Load( vaStream & inStream )
{
    int32 fileVersion = 0;
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<int32>( fileVersion ) );
    if( !((fileVersion >= 1) && (fileVersion <= c_renderMeshMaterialFileVersion)) )
    {
        VA_LOG( L"vaRenderMaterial::Load(): unsupported file version" );
        return false;
    }

    if( fileVersion == 1 )
    {
        MaterialSettingsV1 settingsTemp;
        VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<MaterialSettingsV1>( settingsTemp ) );
        m_settings = MaterialSettings();
        m_settings.FaceCull          = settingsTemp.FaceCull;
        m_settings.ColorMultAlbedo   = settingsTemp.ColorMultAlbedo;
        m_settings.ColorMultSpecular = settingsTemp.ColorMultSpecular;
        m_settings.AlphaTest         = settingsTemp.AlphaTest;
        m_settings.ReceiveShadows    = settingsTemp.ReceiveShadows;
    }
    else
    {
        VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<MaterialSettings>(m_settings ) );
    }

    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<vaGUID>( m_textureAlbedoUID     ) );
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<vaGUID>( m_textureNormalmapUID  ) );
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<vaGUID>( m_textureSpecularUID   ) );
    if( fileVersion > 1 )
        VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<vaGUID>( m_textureEmissiveUID   ) );

    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadString( m_shaderFileName ) );
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadString( m_shaderEntryVS_PosOnly ) );
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadString( m_shaderEntryPS_DepthOnly ) );
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadString( m_shaderEntryVS_Standard ) );
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadString( m_shaderEntryPS_Forward ) );
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadString( m_shaderEntryPS_Deferred ) );

    m_shaderMacrosDirty = true;
    m_shadersDirty = true;

    return true;
}

void vaRenderMaterial::ReconnectDependencies( )
{
    m_textureAlbedo     = nullptr;
    m_textureNormalmap  = nullptr;
    m_textureSpecular   = nullptr;

    vaUIDObjectRegistrar::GetInstance( ).ReconnectDependency<vaTexture>( m_textureAlbedo, m_textureAlbedoUID );
    vaUIDObjectRegistrar::GetInstance( ).ReconnectDependency<vaTexture>( m_textureNormalmap, m_textureNormalmapUID );
    vaUIDObjectRegistrar::GetInstance( ).ReconnectDependency<vaTexture>( m_textureSpecular, m_textureSpecularUID );
    vaUIDObjectRegistrar::GetInstance( ).ReconnectDependency<vaTexture>( m_textureEmissive, m_textureEmissiveUID );
}

vaRenderMaterialManager::vaRenderMaterialManager( )
{
    m_isDestructing = false;
    
    m_renderMaterials.SetAddedCallback( std::bind( &vaRenderMaterialManager::RenderMaterialsTrackeeAddedCallback, this, std::placeholders::_1 ) );
    m_renderMaterials.SetBeforeRemovedCallback( std::bind( &vaRenderMaterialManager::RenderMaterialsTrackeeBeforeRemovedCallback, this, std::placeholders::_1, std::placeholders::_2 ) );

    m_defaultMaterial = VA_RENDERING_MODULE_CREATE_PARAMS_SHARED( vaRenderMaterial, vaRenderMaterialConstructorParams( *this, vaCore::GUIDFromString( L"11523d65-09ea-4342-9bad-8dab7a4dc1e0" ) ) );

    m_texturingDisabled = false;
}
vaRenderMaterialManager::~vaRenderMaterialManager( )
{
    m_isDestructing = true;
    //m_renderMeshesMap.clear();

    m_defaultMaterial = nullptr;
    // this must absolutely be true as they contain direct reference to this object
    assert( m_renderMaterials.size( ) == 0 );
}

void vaRenderMaterialManager::SetTexturingDisabled( bool texturingDisabled )
{
    if( m_texturingDisabled == texturingDisabled )
        return;

    m_texturingDisabled = texturingDisabled;

    for( int i = 0; i < (int)m_renderMaterials.size(); i++ )
    {
        m_renderMaterials[i]->SetSettingsDirty();
    }
}

void vaRenderMaterialManager::RenderMaterialsTrackeeAddedCallback( int newTrackeeIndex )
{
}

void vaRenderMaterialManager::RenderMaterialsTrackeeBeforeRemovedCallback( int removedTrackeeIndex, int replacedByTrackeeIndex )
{
    //    assert( m_renderMaterialsMap.size() == 0 ); // removal not implemented!
}

shared_ptr<vaRenderMaterial> vaRenderMaterialManager::CreateRenderMaterial( const vaGUID & uid )
{
    auto ret = VA_RENDERING_MODULE_CREATE_PARAMS_SHARED( vaRenderMaterial, vaRenderMaterialConstructorParams( *this, uid ) );

    if( !ret->UIDObject_IsCorrectlyTracked() )
    {
        VA_LOG_ERROR_STACKINFO( "Error creating render material; uid already used" );
        return nullptr;
    }

    return ret;
}

void vaRenderMaterialManager::IHO_Draw( )
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
