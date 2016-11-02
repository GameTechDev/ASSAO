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

#include "vaAssetPack.h"

#include "Rendering/vaRenderMesh.h"

#include "Scene/vaAssetImporter.h"

using namespace VertexAsylum;

vaAssetTexture::vaAssetTexture( vaAssetPack & pack, const shared_ptr<vaTexture> & texture, const string & name ) 
    : vaAsset( pack, vaAssetType::Texture, name, texture ), Resource( texture ) 
{ }

vaAssetRenderMesh::vaAssetRenderMesh( vaAssetPack & pack, const shared_ptr<vaRenderMesh> & mesh, const string & name ) 
    : vaAsset( pack, vaAssetType::RenderMesh, name, mesh ), Resource( mesh ) 
{ }

vaAssetRenderMaterial::vaAssetRenderMaterial( vaAssetPack & pack, const shared_ptr<vaRenderMaterial> & material, const string & name ) 
    : vaAsset( pack, vaAssetType::RenderMaterial, name, material ), Resource( material ) 
{ }

vaAssetPack::vaAssetPack( const string & name ) : m_trackee( &vaAssetPackManager::GetInstance().m_assetPackTracker, this ), m_name( vaStringTools::ToLower( name ) )
{

}

vaAssetPack::~vaAssetPack( )
{
    RemoveAll();
}

void vaAssetPack::InsertAndTrackMe( shared_ptr<vaAsset> newAsset )
{
    m_assetMap.insert( std::pair< string, shared_ptr<vaAsset> >( vaStringTools::ToLower( newAsset->Name() ), newAsset ) );
    //    if( storagePath != L"" )
    //        m_assetMapByStoragePath.insert( std::pair< wstring, shared_ptr<vaAsset> >( vaStringTools::ToLower(newAsset->storagePath), newAsset ) );

    assert( newAsset->m_parentPackStorageIndex == -1 );
    m_assetList.push_back( newAsset );
    newAsset->m_parentPackStorageIndex = ((int)m_assetList.size())-1;
}

string vaAssetPack::FindSuitableAssetName( const string & _nameSuggestion )
{
    const string & nameSuggestion = vaStringTools::ToLower( _nameSuggestion );

    if( Find( nameSuggestion ) == nullptr )
        return nameSuggestion;

    int index = 0;
    do 
    {
        string newSuggestion = vaStringTools::Format( "%s_%d", nameSuggestion.c_str(), index );
        if( Find( newSuggestion ) == nullptr )
            return newSuggestion;

        index++;
    } while ( true );

}

shared_ptr<vaAssetTexture> vaAssetPack::Add( const std::shared_ptr<vaTexture> & texture, const string & _name )
{
    string name = vaStringTools::ToLower( _name );

    if( Find( name ) != nullptr )
    {
        assert( false );
        VA_LOG_ERROR( "Unable to add asset '%s' to the asset pack '%s' because the name already exists", name.c_str( ), m_name.c_str( ) );
        return nullptr;
    }
//    assert( ( storagePath == L"" ) || ( FindByStoragePath( storagePath ) == nullptr ) );    // assets in packs must have unique names
    
    shared_ptr<vaAssetTexture> newItem = shared_ptr<vaAssetTexture>( new vaAssetTexture( *this, texture, name ) );

    InsertAndTrackMe( newItem );

    return newItem;
}

shared_ptr<vaAssetRenderMesh> vaAssetPack::Add( const std::shared_ptr<vaRenderMesh> & mesh, const string & _name )
{
    string name = vaStringTools::ToLower( _name );

    if( Find( name ) != nullptr )
    {
        assert( false );
        VA_LOG_ERROR( "Unable to add asset '%s' to the asset pack '%s' because the name already exists", name.c_str( ), m_name.c_str( ) );
        return nullptr;
    }
//    assert( ( storagePath == L"" ) || ( FindByStoragePath( storagePath ) == nullptr ) );    // assets in packs must have unique names

    shared_ptr<vaAssetRenderMesh> newItem = shared_ptr<vaAssetRenderMesh>( new vaAssetRenderMesh( *this, mesh, name ) );

    InsertAndTrackMe( newItem );

    return newItem;
}

shared_ptr<vaAssetRenderMaterial> vaAssetPack::Add( const std::shared_ptr<vaRenderMaterial> & material, const string & _name )
{
    string name = vaStringTools::ToLower( _name );

    if( Find( name ) != nullptr )
    {
        assert( false );
        VA_LOG_ERROR( "Unable to add asset '%s' to the asset pack '%s' because the name already exists", name.c_str(), m_name.c_str() );
        return nullptr;
    }
//    assert( (storagePath == L"") || (FindByStoragePath( storagePath ) == nullptr) );    // assets in packs must have unique names

    shared_ptr<vaAssetRenderMaterial> newItem = shared_ptr<vaAssetRenderMaterial>( new vaAssetRenderMaterial( *this, material, name ) );

    InsertAndTrackMe( newItem );

    return newItem;
}

bool vaAsset::Rename( const string & newName )    
{ 
    return m_parentPack.Rename( *this, newName ); 
}

bool vaAssetPack::Rename( vaAsset & asset, const string & _newName )
{
    string newName = vaStringTools::ToLower( _newName );

    if( &asset.m_parentPack != this )
    {
        VA_LOG_ERROR( "Unable to change asset name from '%s' to '%s' in asset pack '%s' - not correct parent pack!", asset.Name().c_str(), newName.c_str(), this->m_name.c_str()  )
        return false;
    }
    if( newName == asset.Name() )
    {
        VA_LOG( "Changing asset name from '%s' to '%s' in asset pack '%s' - same name requested? Nothing changed.", asset.Name().c_str(), newName.c_str(), this->m_name.c_str()  )
        return true;
    }
    if( Find( newName ) != nullptr )
    {
        VA_LOG_ERROR( "Unable to change asset name from '%s' to '%s' in asset pack '%s' - name already used by another asset!", asset.Name().c_str(), newName.c_str(), this->m_name.c_str()  )
        return false;
    }

    {
        auto it = m_assetMap.find( vaStringTools::ToLower( asset.Name() ) );

        shared_ptr<vaAsset> assetSharedPtr;

        if( it != m_assetMap.end() ) 
        {
            assetSharedPtr = it->second;
            assert( assetSharedPtr.get() == &asset );
            m_assetMap.erase( it );
        }
        else
        {
            VA_LOG_ERROR( "Error changing asset name from '%s' to '%s' in asset pack '%s' - original asset not found!", asset.Name().c_str(), newName.c_str(), this->m_name.c_str()  )
            return false;
        }

        assetSharedPtr->m_name = newName;

        m_assetMap.insert( std::pair< string, shared_ptr<vaAsset> >( vaStringTools::ToLower( assetSharedPtr->Name() ), assetSharedPtr ) );
    }

    VA_LOG( "Changing asset name from '%s' to '%s' in asset pack '%s' - success!", asset.Name().c_str(), newName.c_str(), this->m_name.c_str() );
    return true;
}

void vaAssetPack::Remove( const shared_ptr<vaAsset> & asset )
{
    if( asset == nullptr )
        return;

    assert( m_assetList[asset->m_parentPackStorageIndex] == asset );
    if( (int)m_assetList.size( ) != ( asset->m_parentPackStorageIndex + 1 ) )
    {
        m_assetList[ asset->m_parentPackStorageIndex ] = m_assetList[ m_assetList.size( ) - 1 ];
        m_assetList[ asset->m_parentPackStorageIndex ]->m_parentPackStorageIndex = asset->m_parentPackStorageIndex;
    }
    asset->m_parentPackStorageIndex = -1;
    m_assetList.pop_back();

    {
        auto it = m_assetMap.find( vaStringTools::ToLower(asset->Name()) );

        // possible memory leak! does the asset belong to another asset pack?
        assert( it != m_assetMap.end() );
        if( it == m_assetMap.end() ) 
            return; 
    
        m_assetMap.erase( it );
    }
    
//    if( asset->StoragePath != L"" )
//    {
//        auto it = m_assetMapByStoragePath.find( vaStringTools::ToLower(asset->StoragePath) );
//
//        // can't not be in!
//        assert( it != m_assetMapByStoragePath.end( ) );
//        if( it == m_assetMapByStoragePath.end( ) )
//            return;
//
//        m_assetMapByStoragePath.erase( it );
//    }

}

void vaAssetPack::RemoveAll( )
{
    m_assetList.clear( );
//    m_assetMapByStoragePath.clear( );

    for( auto it = m_assetMap.begin( ); it != m_assetMap.end( ); it++ )
    {
        // if this fails it means someone is still holding a reference to assets from this pack - this shouldn't ever happen, they should have been released!
        assert( it->second.unique() );
    }
    m_assetMap.clear();
}

const int c_packFileVersion = 1;

bool vaAssetPack::Save( vaStream & outStream )
{
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.CanSeek( ) );

    int64 posOfSize = outStream.GetPosition( );
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<int64>( 0 ) );

    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<int32>( c_packFileVersion ) );

    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteString( m_name ) );

    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<int32>( (int)m_assetMap.size() ) );

    for( auto it = m_assetMap.begin( ); it != m_assetMap.end( ); it++ )
    {
        int64 posOfSubSize = outStream.GetPosition( );
        VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<int64>( 0 ) );

        // write type
        VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<int32>( (int32)it->second->Type ) );

        // write name
        VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteString( it->first ) );
        assert( it->first == it->second->Name() );

        // write asset
        VERIFY_TRUE_RETURN_ON_FALSE( it->second->Save( outStream ) );

        int64 calculatedSubSize = outStream.GetPosition() - posOfSubSize;
        auto aaaa = outStream.GetPosition();
        outStream.Seek( posOfSubSize );
        VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<int64>( calculatedSubSize ) );
        outStream.Seek( posOfSubSize + calculatedSubSize );
    }

    int64 calculatedSize = outStream.GetPosition( ) - posOfSize;
    outStream.Seek( posOfSize );
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<int64>( calculatedSize ) );
    outStream.Seek( posOfSize + calculatedSize );

    return true;
}

bool vaAssetPack::Load( vaStream & inStream )
{
//    RemoveAll( );

    int64 size = 0;
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<int64>( size ) );

    int32 fileVersion = 0;
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<int32>( fileVersion ) );
    if( fileVersion != c_packFileVersion )
    {
        VA_LOG( L"vaAssetPack::Load(): unsupported file version" );
        return false;
    }

    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadString( m_name ) );
    
    int32 numberOfAssets = 0;
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<int32>( numberOfAssets ) );

    vector< shared_ptr<vaAsset> > loadedAssets;

    for( int i = 0; i < numberOfAssets; i++  )
    {
        int64 subSize = 0;
        VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<int64>( subSize ) );

        // read type
        vaAssetType assetType;
        VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<int32>( (int32&)assetType ) );

        // read name
        string newAssetName;
        VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadString( newAssetName ) );

        if( Find( newAssetName ) != nullptr )
        {
            VA_LOG_ERROR( L"vaAssetPack::Load(): duplicated asset name, stopping loading." );
            assert( false );
            return false;
        }

        shared_ptr<vaAsset> newAsset = nullptr;

        switch( assetType )
        {
        case VertexAsylum::vaAssetType::Texture:
            newAsset = shared_ptr<vaAsset>( vaAssetTexture::CreateAndLoad( *this, newAssetName, inStream ) );
            break;
        case VertexAsylum::vaAssetType::RenderMesh:
            newAsset = shared_ptr<vaAsset>( vaAssetRenderMesh::CreateAndLoad( *this, newAssetName, inStream ) );
            break;
        case VertexAsylum::vaAssetType::RenderMaterial:
            newAsset = shared_ptr<vaAsset>( vaAssetRenderMaterial::CreateAndLoad( *this, newAssetName, inStream ) );
            break;
        default:
            break;
        }


        VERIFY_TRUE_RETURN_ON_FALSE( newAsset != nullptr );

        InsertAndTrackMe( newAsset );

        loadedAssets.push_back( newAsset );
    }

    for( size_t i = 0; i < loadedAssets.size(); i++ )
    {
        loadedAssets[i]->ReconnectDependencies( );
    }

    return true;
}

bool vaAssetTexture::Save( vaStream & outStream )
{
    VERIFY_TRUE_RETURN_ON_FALSE( Resource != nullptr );

    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<vaGUID>( Resource->UIDObject_GetUID( ) ) );

    return Resource->Save( outStream );
}

bool vaAssetRenderMesh::Save( vaStream & outStream ) 
{
    VERIFY_TRUE_RETURN_ON_FALSE( Resource != nullptr );

    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<vaGUID>( Resource->UIDObject_GetUID( ) ) );

    return Resource->Save( outStream ); 
}

bool vaAssetRenderMaterial::Save( vaStream & outStream )
{
    VERIFY_TRUE_RETURN_ON_FALSE( Resource != nullptr );

    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<vaGUID>( Resource->UIDObject_GetUID( ) ) );

    return Resource->Save( outStream );
}

vaAssetRenderMesh * vaAssetRenderMesh::CreateAndLoad( vaAssetPack & pack, const string & name, vaStream & inStream )
{
    vaGUID uid;
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<vaGUID>( uid ) );

    shared_ptr<vaRenderMesh> newResource = vaRenderMeshManager::GetInstance().CreateRenderMesh( uid );

    if( newResource == nullptr )
        return nullptr;

    if( newResource->Load( inStream ) )
    {
        return new vaAssetRenderMesh( pack, newResource, name );
    }
    else
    {
        return nullptr;
    }
}

vaAssetRenderMaterial * vaAssetRenderMaterial::CreateAndLoad( vaAssetPack & pack, const string & name, vaStream & inStream )
{
    vaGUID uid;
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<vaGUID>( uid ) );

    shared_ptr<vaRenderMaterial> newResource = vaRenderMaterialManager::GetInstance( ).CreateRenderMaterial( uid );

    if( newResource == nullptr )
        return nullptr;

    if( newResource->Load( inStream ) )
    {
        return new vaAssetRenderMaterial( pack, newResource, name );
    }
    else
    {
        return nullptr;
    }
}

vaAssetTexture * vaAssetTexture::CreateAndLoad( vaAssetPack & pack, const string & name, vaStream & inStream )
{
    vaGUID uid;
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<vaGUID>( uid ) );

    shared_ptr<vaTexture> newResource = VA_RENDERING_MODULE_CREATE_PARAMS_SHARED( vaTexture, vaTextureConstructorParams( uid ) );

    if( newResource == nullptr )
        return nullptr;

    if( !newResource->UIDObject_IsCorrectlyTracked() )
    {
        VA_LOG_ERROR_STACKINFO( "Error creating asset texture; uid already used" );
        return nullptr;
    }

    if( newResource->Load( inStream ) )
    {
        return new vaAssetTexture( pack, newResource, name );
    }
    else
    {
        return nullptr;
    }
}

void vaAsset::IHO_Draw( )                     
{
#ifdef VA_IMGUI_INTEGRATION_ENABLED
    bool renameJustOpened = false;
    const char * renameAssetPackPopup = "Rename asset pack";
    if( ImGui::Button( "Rename" ) )
    {
        ImGui::OpenPopup( renameAssetPackPopup );
        strcpy_s( m_uiContext.RenamingPopupNewNameBuff, sizeof(m_uiContext.RenamingPopupNewNameBuff), Name().c_str() );
        renameJustOpened = true;
    }

    if( ImGui::BeginPopupModal( renameAssetPackPopup ) )
    {
        string newName = "";
        if( renameJustOpened )
        {
            ImGui::SetKeyboardFocusHere();
        }

        //if( 
            ImGui::InputText( "New name", m_uiContext.RenamingPopupNewNameBuff, sizeof( m_uiContext.RenamingPopupNewNameBuff ), ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue );// )
        
        newName = vaStringTools::ToLower( string( m_uiContext.RenamingPopupNewNameBuff ) );
        if( ImGui::Button( "Accept" ) )
        {
            if( newName != "" )
            {
                if( m_parentPack.Rename( *this, newName ) )
                {
                    ImGui::CloseCurrentPopup();
                }
            }
        }
        ImGui::SameLine();
        if( ImGui::Button( "Cancel" ) )
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
#endif
}

#ifdef VA_IMGUI_INTEGRATION_ENABLED
static void IHO_DrawTextureInfo( string formatStr, const shared_ptr<vaTexture> & tex )
{
    if( tex != nullptr )
    {
        const vaAsset * parentAsset = tex->GetParentAsset( );
        if( parentAsset != nullptr )
        {
            ImGui::Text( formatStr.c_str(), parentAsset->Name().c_str() );
        }
        else
        {
            ImGui::Text( formatStr.c_str(), "no parent asset (procedural or etc.)" );
        }
    }
    else
    {
        ImGui::Text( formatStr.c_str(), "null" );
    }
}
#endif

void vaAssetRenderMaterial::IHO_Draw( )
{
#ifdef VA_IMGUI_INTEGRATION_ENABLED
    vaAsset::IHO_Draw();

    ImGui::Text("Textures:");
    ImGui::Indent();
    IHO_DrawTextureInfo( "Albedo:    %s", Resource->GetTextureAlbedo() );
    IHO_DrawTextureInfo( "Normalmap: %s", Resource->GetTextureNormalmap() );
    IHO_DrawTextureInfo( "Specular:  %s", Resource->GetTextureSpecular() );
    IHO_DrawTextureInfo( "Emissive:  %s", Resource->GetTextureEmissive() );
    ImGui::Unindent();

    ImGui::Text("Settings");
    ImGui::Indent();
    vaRenderMaterial::MaterialSettings settings = Resource->GetSettings();
    ImGui::ColorEdit4( "Color mult albedo", &settings.ColorMultAlbedo.x );
    ImGui::ColorEdit3( "Color mult specular", &settings.ColorMultSpecular.x );
    ImGui::ColorEdit3( "Color mult emissive", &settings.ColorMultEmissive.x );
    ImGui::Combo( "Culling mode", (int*)&settings.FaceCull, "None\0Front\0Back" );
    ImGui::Checkbox( "Transparent", &settings.Transparent );
    ImGui::Checkbox( "AlphaTest", &settings.AlphaTest );
    ImGui::Checkbox( "ReceiveShadows", &settings.ReceiveShadows );
    ImGui::Checkbox( "Wireframe", &settings.Wireframe );
    ImGui::InputFloat( "Specular Pow", &settings.SpecPow, 0.1f );
    ImGui::InputFloat( "Specular Mul", &settings.SpecMul, 0.1f );
    
    if( Resource->GetSettings() != settings )
        Resource->SetSettings( settings );
    
    ImGui::Unindent();
#endif
}

void vaAssetPack::IHO_Draw( )
{ 
#ifdef VA_IMGUI_INTEGRATION_ENABLED
    ImGui::PushItemWidth( 120.0f );

    // rename UI
    {
        bool renameJustOpened = false;
        const char * renameAssetPackPopup = "Rename asset pack";
        if( ImGui::Button( "Rename asset pack" ) )
        {
            ImGui::OpenPopup( renameAssetPackPopup );
            strcpy_s( m_uiContext.RenamingPopupNewNameBuff, sizeof(m_uiContext.RenamingPopupNewNameBuff), m_name.c_str() );
            renameJustOpened = true;
        }

        if( ImGui::BeginPopupModal( renameAssetPackPopup ) )
        {
            string newName = "";
            if( renameJustOpened )
            {
                ImGui::SetKeyboardFocusHere();
            }

            //if( 
                ImGui::InputText( "New name", m_uiContext.RenamingPopupNewNameBuff, sizeof( m_uiContext.RenamingPopupNewNameBuff ), ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue );// )
        
            newName = vaStringTools::ToLower( string( m_uiContext.RenamingPopupNewNameBuff ) );
            if( ImGui::Button( "Accept" ) )
            {
                if( newName != "" )
                {
                    m_name = newName;
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::SameLine();
            if( ImGui::Button( "Cancel" ) )
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    ImGui::Separator();

#ifdef VA_ASSIMP_INTEGRATION_ENABLED
    // importing assets UI
    {
        const char * importAssetsPopup = "Import assets";
        if( ImGui::Button( importAssetsPopup ) )
        {
            wstring fileName = vaFileTools::OpenFileDialog( L"", vaCore::GetExecutableDirectory() );
            if( vaFileTools::FileExists( fileName ) )
            {
                //strcpy_s( m_uiContext.ImportingModeNewFileNameBuff, sizeof(m_uiContext.ImportingModeNewFileNameBuff), "" );
                m_uiContext.ImportingPopupSelectedFile          = fileName;
                m_uiContext.ImportingPopupBaseTranslation       = vaVector3( 0.0f, 0.0f, 0.0f );
                m_uiContext.ImportingPopupBaseScaling           = vaVector3( 1.0f, 1.0f, 1.0f );
                m_uiContext.ImportingPopupRotateX90             = true;
                m_uiContext.ImportingForceGenerateNormals       = false;
                m_uiContext.ImportingGenerateNormalsIfNeeded    = true;
                m_uiContext.ImportingGenerateSmoothNormals      = true;
                //m_uiContext.ImportingRegenerateTangents     = false;
                ImGui::OpenPopup("Import assets");
            }
        }

        if( ImGui::BeginPopupModal( importAssetsPopup ) )
        {
            string fileNameA = vaStringTools::SimpleNarrow( m_uiContext.ImportingPopupSelectedFile );
            ImGui::Text( "Importing file:" );
            ImGui::Text( "(idea: store these import settings default next to the importing file in .va_lastimport)" );
            ImGui::Text( " '%s'", fileNameA.c_str() );
            ImGui::Separator();
            ImGui::Text( "Base transformation (applied to everything):" );
            ImGui::Checkbox( "Base rotate around X axis by 90º",        &m_uiContext.ImportingPopupRotateX90 );
            ImGui::InputFloat3( "Base scaling",                         &m_uiContext.ImportingPopupBaseScaling.x );
            ImGui::InputFloat3( "Base translation",                     &m_uiContext.ImportingPopupBaseTranslation.x );
            ImGui::Separator();
            ImGui::Text( "Import options:" );
            ImGui::Checkbox( "Force generate normals",                  &m_uiContext.ImportingForceGenerateNormals );
            ImGui::Checkbox( "Generate normals (if missing)",           &m_uiContext.ImportingGenerateNormalsIfNeeded );
            ImGui::Checkbox( "Generate smooth normals (if generating)", &m_uiContext.ImportingGenerateSmoothNormals );
            //ImGui::Checkbox( "Regenerate tangents/bitangents",      &m_uiContext.ImportingRegenerateTangents );
            ImGui::Separator();
            if( ImGui::Button( "          Start importing (can take a while)          " ) )
            {
                 vaAssetImporter::LoadingParameters loadParams( *this );
                 loadParams.BaseTransform                       = vaMatrix4x4::Scaling( m_uiContext.ImportingPopupBaseScaling ) * vaMatrix4x4::RotationX( m_uiContext.ImportingPopupRotateX90?(VA_PIf * 0.5f):(0.0f) ) * vaMatrix4x4::Translation( m_uiContext.ImportingPopupBaseTranslation ) ;
                 loadParams.ForceGenerateNormals                = m_uiContext.ImportingForceGenerateNormals;
                 loadParams.GenerateNormalsIfNeeded             = m_uiContext.ImportingGenerateNormalsIfNeeded;
                 loadParams.GenerateSmoothNormalsIfGenerating   = m_uiContext.ImportingGenerateSmoothNormals;
                 //loadParams.RegenerateTangents  = m_uiContext.ImportingRegenerateTangents;
                 vaAssetImporter::LoadFileContents( m_uiContext.ImportingPopupSelectedFile, loadParams );

                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if( ImGui::Button( "Cancel" ) )
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }
#endif

    // Loading assets
    if( ImGui::Button( "Load .apack" ) )
    {
        wstring fileName = vaFileTools::OpenFileDialog( L"", vaCore::GetExecutableDirectory(), L".apack files\0*.apack\0\0" );
        if( vaFileTools::FileExists( fileName ) )
        {
            vaFileStream fileIn;
            if( fileIn.Open( fileName, FileCreationMode::Open ) )
                Load( fileIn );
        }
    }

    // Saving assets
    if( ImGui::Button( "Save .apack" ) )
    {
        wstring fileName = vaFileTools::SaveFileDialog( L"", vaCore::GetExecutableDirectory(), L".apack files\0*.apack\0\0" );
        vaFileStream fileOut;
        if( fileOut.Open( fileName, FileCreationMode::Create ) )
            Save( fileOut );
    }

    ImGui::Separator();

        // remove all assets UI
    {
        const char * deleteAssetPackPopup = "Remove all assets";
        if( ImGui::Button( "Remove all assets" ) )
        {
            ImGui::OpenPopup( deleteAssetPackPopup );
        }

        if( ImGui::BeginPopupModal( deleteAssetPackPopup ) )
        {
            ImGui::Text( "Are you sure that you want to remove all assets?" ); 
            if( ImGui::Button( "Yes" ) )
            {
                RemoveAll();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if( ImGui::Button( "Cancel" ) )
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    ImGui::Separator();

    // Show contents
    {
        ImGui::Text( "Contained assets (%d):", m_assetList.size() );
        ImGui::Indent();
        for( size_t i = 0; i < m_assetList.size(); i++ )
        {
            vaImguiHierarchyObject::DrawCollapsable( *m_assetList[i] );
        }
        ImGui::Unindent();
    }

    ImGui::PopItemWidth( );
#endif
}

vaAssetPackManager::vaAssetPackManager( )
{
    m_defaultPack = unique_ptr<vaAssetPack>( new vaAssetPack( string("default") ) );
}

vaAssetPackManager::~vaAssetPackManager( )      
{ 
    m_defaultPack.reset();
}

void vaAssetPackManager::OnRenderingAPIAboutToShutdown( )
{
    // notify all packs of this
    for( size_t i = 0; i < m_assetPackTracker.size(); i++ )
        m_assetPackTracker[i]->OnRenderingAPIAboutToShutdown( );
}

void vaAssetPackManager::IHO_Draw( )
{ 
#ifdef VA_IMGUI_INTEGRATION_ENABLED
    // if( ImGui::Button( "Create new empty pack" ) )
    // {
    //     
    // }
    // 
    // ImGui::Button( "Load existing .apack file" );
    
    ImGui::Text( "Loaded asset packs: " );

    ImGui::Indent();
    for( size_t i = 0; i < m_assetPackTracker.size(); i++ )
    {
        vaImguiHierarchyObject::DrawCollapsable( *m_assetPackTracker[i] );
    }
    ImGui::Unindent();
#endif
}