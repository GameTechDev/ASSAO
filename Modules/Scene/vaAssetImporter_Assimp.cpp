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

#include "vaAssetImporter.h"
#include "Rendering/vaStandardShapes.h"

#include "IntegratedExternals/vaAssimpIntegration.h"

// #include <d3d11_1.h>
// #include <DirectXMath.h>
// #pragma warning(disable : 4324 4481)
//
// #include <exception>

//#include <mutex>

using namespace VertexAsylum;

#ifdef VA_ASSIMP_INTEGRATION_ENABLED

//static vector<shared_ptr<vaAssetRenderMaterial>> SDKMESH_LoadMaterials( const wstring & textureSearchPath, const DXUT::SDKMESH_MATERIAL* pMaterials, UINT numMaterials, vaAssetImporter::LoadingParameters & parameters, vaAssetImporter::LoadedContent * outContent )
//{
//    vector<shared_ptr<vaAssetRenderMaterial>>   loadedMaterials;
//
//    vector<shared_ptr<vaAssetTexture>>          loadedTextures;
//    vector<string>                              loadedTextureNames;
//
//    for( UINT m = 0; m < numMaterials; m++ )
//    {
//
//        shared_ptr<vaRenderMaterial> material = vaRenderMaterialManager::GetInstance().CreateRenderMaterial();
//
//        // load textures
//        if( pMaterials[m].DiffuseTexture[0] != 0 )
//        {
//            shared_ptr<vaAssetTexture> loadedTextureAsset = FindOrLoadTexture( textureSearchPath, pMaterials[m].DiffuseTexture, loadedTextures, loadedTextureNames, parameters, outContent, true, false );
//            if( loadedTextureAsset != nullptr )
//                material->SetTextureAlbedo( loadedTextureAsset->Resource );
//        }
//        if( pMaterials[m].NormalTexture[0] != 0 )
//        {
//            shared_ptr<vaAssetTexture> loadedTextureAsset = FindOrLoadTexture( textureSearchPath, pMaterials[m].NormalTexture, loadedTextures, loadedTextureNames, parameters, outContent, false, false );
//            if( loadedTextureAsset != nullptr )
//                material->SetTextureNormalmap( loadedTextureAsset->Resource );
//        }
//        if( pMaterials[m].SpecularTexture[0] != 0 )
//        {
//            shared_ptr<vaAssetTexture> loadedTextureAsset = FindOrLoadTexture( textureSearchPath, pMaterials[m].SpecularTexture, loadedTextures, loadedTextureNames, parameters, outContent, false, false );
//            if( loadedTextureAsset != nullptr )
//                material->SetTextureSpecular( loadedTextureAsset->Resource );
//        }
//
//        vaRenderMaterial::MaterialSettings settings;
//
//        // material->Settings().ColorMultAlbedo    = vaVector4( pMaterials[m].Diffuse.x, pMaterials[m].Diffuse.y, pMaterials[m].Diffuse.z, pMaterials[m].Diffuse.w );
//        // material->Settings().ColorMultSpecular  = vaVector4( pMaterials[m].Specular.x, pMaterials[m].Specular.y, pMaterials[m].Specular.z, pMaterials[m].Specular.w );
//        settings.AlphaTest          = false;
//        settings.FaceCull           = vaFaceCull::Back;
//
//        auto materialAsset = parameters.AssetPack.Add( material, parameters.AssetPack.FindSuitableAssetName( string(pMaterials[m].Name ) ) );
//
//        loadedMaterials.push_back( materialAsset );
//
//        if( outContent != nullptr )
//            outContent->LoadedAssets.push_back( materialAsset );
//    }
//
//    return loadedMaterials;
//}

namespace 
{ 
    class myLogInfoStream : public Assimp::LogStream
    {
    public:
            myLogInfoStream()   { }
            ~myLogInfoStream()  { }

            void write(const char* message)
            {
                VA_LOG( "Assimp info    : %s", message );
            }
    };
    class myLogWarningStream : public Assimp::LogStream
    {
    public:
            myLogWarningStream()   { }
            ~myLogWarningStream()  { }

            void write(const char* message)
            {
                VA_LOG( "Assimp warning : %s", message );
            }
    };
    class myLogErrorStream : public Assimp::LogStream
    {
    public:
            myLogErrorStream()   { }
            ~myLogErrorStream()  { }

            void write(const char* message)
            {
                VA_LOG_ERROR( "Assimp error   : %s", message );
            }
    };

    struct LoadingTempStorage
    {
        struct LoadedTexture
        {
            const aiTexture *                   AssimpTexture;
            shared_ptr<vaAssetTexture>          Texture;
            string                              OriginalPath;
            bool                                AssumedSRGB;
            bool                                NoAutogeneratedMIPs;

            LoadedTexture( const aiTexture * assimpTexture, const shared_ptr<vaAssetTexture> & texture, const string & originalPath, bool assumedSRGB, bool noAutogeneratedMIPs )
                : AssimpTexture( assimpTexture ), Texture(texture), OriginalPath(originalPath), AssumedSRGB(assumedSRGB), NoAutogeneratedMIPs(noAutogeneratedMIPs)
            { }
        };

        struct LoadedMaterial
        {
            const aiMaterial *                  AssimpMaterial;
            shared_ptr<vaAssetRenderMaterial>   Material;

            LoadedMaterial( const aiMaterial * assimpMaterial, const shared_ptr<vaAssetRenderMaterial> & material )
                : AssimpMaterial( assimpMaterial ), Material( material ) 
            { }
        };

        struct LoadedMesh
        {
            const aiMesh *                      AssimpMesh;
            shared_ptr<vaAssetRenderMesh>       Mesh;

            LoadedMesh( const aiMesh * assimpMesh, const shared_ptr<vaAssetRenderMesh> & mesh )
                : AssimpMesh( assimpMesh ), Mesh( mesh ) 
            { }
        };

        wstring                                     ImportDirectory;
        wstring                                     ImportFileName;
        wstring                                     ImportExt;

        vector<LoadedTexture>                       LoadedTextures;
        vector<LoadedMaterial>                      LoadedMaterials;
        vector<LoadedMesh>                          LoadedMeshes;

        shared_ptr<vaAssetRenderMaterial>           FindMaterial( const aiMaterial * assimpMaterial )
        {
            for( int i = 0; i < LoadedMaterials.size(); i++ )
            {
                if( LoadedMaterials[i].AssimpMaterial == assimpMaterial )
                    return LoadedMaterials[i].Material;
            }
            return nullptr;
        }

        shared_ptr<vaAssetRenderMesh>               FindMesh( const aiMesh * assimpMesh )
        {
            for( int i = 0; i < LoadedMeshes.size(); i++ )
            {
                if( LoadedMeshes[i].AssimpMesh == assimpMesh )
                    return LoadedMeshes[i].Mesh;
            }
            return nullptr;
        }
    };
}


static shared_ptr<vaAssetTexture> FindOrLoadTexture( aiTexture * assimpTexture, const string & _path, LoadingTempStorage & tempStorage, vaAssetImporter::LoadingParameters & parameters, vaAssetImporter::LoadedContent * outContent, bool assumeSourceIsInSRGB, bool dontAutogenerateMIPs )
{
    string originalPath = vaStringTools::ToLower(_path);
    wstring filePath = vaStringTools::SimpleWiden( originalPath );

    for( int i = 0; i < tempStorage.LoadedTextures.size(); i++ )
    {
        if( (originalPath == tempStorage.LoadedTextures[i].OriginalPath) && (assumeSourceIsInSRGB == tempStorage.LoadedTextures[i].AssumedSRGB) && (dontAutogenerateMIPs == tempStorage.LoadedTextures[i].NoAutogeneratedMIPs) )
        {
            assert( assimpTexture == tempStorage.LoadedTextures[i].AssimpTexture );
            return tempStorage.LoadedTextures[i].Texture;
        }
    }

    wstring outDir, outName, outExt;
    vaStringTools::SplitPath( filePath, &outDir, &outName, &outExt );

    bool foundDDS = outExt == L".dds";
    if( !foundDDS && (parameters.TextureOnlyLoadDDS || parameters.TextureTryLoadDDS) )
    {
        wstring filePathDDS = outDir + outName + L".dds";
        if( vaFileTools::FileExists( filePathDDS ) )
        {
            filePath = filePathDDS;
            foundDDS = true;
        }
        else
        {
            wstring filePathDDS = tempStorage.ImportDirectory + outDir + outName + L".dds";
            if( vaFileTools::FileExists( filePathDDS ) )
            {
                filePath = filePathDDS;
                foundDDS = true;
            }
        }
    }

    if( !foundDDS && parameters.TextureOnlyLoadDDS )
    {
        VA_LOG( L"VaAssetImporter_Assimp : TextureOnlyLoadDDS true but no .dds texture found when looking for '%s'", filePath.c_str() )
        return nullptr;
    }

    if( !vaFileTools::FileExists( filePath ) )
    {
        wstring filePath = tempStorage.ImportDirectory + outDir + outName + L"." + outExt;
        if( !vaFileTools::FileExists( filePath ) )
        {
            VA_LOG( "VaAssetImporter_Assimp - Unable to find texture '%s'", filePath.c_str() );
            return nullptr;
        }
    }

    shared_ptr<vaTexture> textureOut = shared_ptr<vaTexture>( vaTexture::Import( filePath, assumeSourceIsInSRGB, dontAutogenerateMIPs ) );

    if( textureOut == nullptr )
    {
        VA_LOG( "VaAssetImporter_Assimp - Error while loading '%s'", filePath.c_str( ) );
        return nullptr;
    }

    shared_ptr<vaAssetTexture> textureAssetOut = parameters.AssetPack.Add( textureOut, parameters.AssetPack.FindSuitableAssetName( vaStringTools::SimpleNarrow(outName) ) );

    tempStorage.LoadedTextures.push_back( LoadingTempStorage::LoadedTexture( assimpTexture, textureAssetOut, originalPath, assumeSourceIsInSRGB, dontAutogenerateMIPs ) );

    if( outContent != nullptr )
        outContent->LoadedAssets.push_back( textureAssetOut );

    VA_LOG_SUCCESS( L"Assimp texture '%s' loaded ok.", filePath.c_str() );

    return textureAssetOut;
}

static bool ProcessTextures( const aiScene* loadedScene, LoadingTempStorage & tempStorage, vaAssetImporter::LoadingParameters & parameters, vaAssetImporter::LoadedContent * outContent )
{
    if( loadedScene->HasTextures( ) )
    {
        VA_LOG_ERROR( "Assimp error: Support for meshes with embedded textures is not implemented" );
        return false;
        // for( int i = 0; i < loadedScene->mNumTextures; i++ )
        // {
        //     aiTexture * texture = loadedScene->mTextures[i];
        // }
    }

/*
	for( unsigned int m = 0; m < loadedScene->mNumMaterials; m++ )
	{
		int texIndex = 0;
		aiReturn texFound = AI_SUCCESS;

		aiString path;	// filename

		while (texFound == AI_SUCCESS)
		{
			texFound = loadedScene->mMaterials[m]->GetTexture( aiTextureType_DIFFUSE, texIndex, &path );
			//textureIdMap[path.data] = NULL; //fill map with textures, pointers still NULL yet

            FindOrLoadTexture( path.data, tempStorage, parameters, outContent, true, false );

			texIndex++;
		}
	}
    */


    return true;
}

static bool ProcessMaterials( const aiScene* loadedScene, LoadingTempStorage & tempStorage, vaAssetImporter::LoadingParameters & parameters, vaAssetImporter::LoadedContent * outContent )
{
	for( unsigned int mi = 0; mi < loadedScene->mNumMaterials; mi++ )
	{
        aiMaterial * assimpMaterial = loadedScene->mMaterials[mi];

        aiString pathTmp;

        aiString        matName("unnamed");
        aiColor3D       matColorDiffuse         = aiColor3D( 1.0f, 1.0f, 1.0f );
        aiColor3D       matColorSpecular        = aiColor3D( 1.0f, 1.0f, 1.0f );
        aiColor3D       matColorAmbient         = aiColor3D( 1.0f, 1.0f, 1.0f );
        aiColor3D       matColorEmissive        = aiColor3D( 0.0f, 0.0f, 0.0f );
        aiColor3D       matColorTransparent     = aiColor3D( 0.0f, 0.0f, 0.0f );    // for transparent materials / not sure if it is intended for subsurface scattering?
        aiColor3D       matColorReflective      = aiColor3D( 0.0f, 0.0f, 0.0f );    // not sure what this is
        int             matWireframe            = 0;                                // Specifies whether wireframe rendering must be turned on for the material. 0 for false, !0 for true.	
        int             matTwosided             = 0;                                // Specifies whether meshes using this material must be rendered without backface culling. 0 for false, !0 for true.	
        aiShadingMode   matShadingModel         = aiShadingMode_Flat;               // 
        aiBlendMode     matBlendMode            = aiBlendMode_Default;              // Blend mode - not sure if applicable at all to us.
        float           matOpacity              = 1.0f;                             // Blend alpha multiplier.
        float           matSpecularPow          = 1.0f;                             // SHININESS - Defines the shininess of a phong-shaded material. This is actually the exponent of the phong specular equation; SHININESS=0 is equivalent to SHADING_MODEL=aiShadingMode_Gouraud.
        float           matSpecularMul          = 1.0f;                             // SHININESS_STRENGTH - Scales the specular color of the material.This value is kept separate from the specular color by most modelers, and so do we.
        float           matRefractI             = 1.0f;                             // Defines the Index Of Refraction for the material. That's not supported by most file formats.	Might be of interest for raytracing.
        float           matBumpScaling          = 1.0f;
        float           matReflectivity         = 1.0f;

        assimpMaterial->Get( AI_MATKEY_NAME, matName );

        assimpMaterial->Get( AI_MATKEY_TWOSIDED, matTwosided );
        assimpMaterial->Get( AI_MATKEY_SHADING_MODEL, *(unsigned int*)&matShadingModel );
        assimpMaterial->Get( AI_MATKEY_ENABLE_WIREFRAME, *(unsigned int*)&matWireframe );
        assimpMaterial->Get( AI_MATKEY_BLEND_FUNC, *(unsigned int*)&matBlendMode );
        assimpMaterial->Get( AI_MATKEY_OPACITY, matOpacity );
        assimpMaterial->Get( AI_MATKEY_BUMPSCALING, matBumpScaling );
        assimpMaterial->Get( AI_MATKEY_SHININESS, matSpecularPow );
        assimpMaterial->Get( AI_MATKEY_SHININESS_STRENGTH, matSpecularMul );
        assimpMaterial->Get( AI_MATKEY_REFLECTIVITY, matReflectivity );
        assimpMaterial->Get( AI_MATKEY_REFRACTI, matRefractI );
        assimpMaterial->Get( AI_MATKEY_COLOR_DIFFUSE, matColorDiffuse );
        assimpMaterial->Get( AI_MATKEY_COLOR_AMBIENT, matColorSpecular );
        assimpMaterial->Get( AI_MATKEY_COLOR_SPECULAR, matColorAmbient );
        assimpMaterial->Get( AI_MATKEY_COLOR_EMISSIVE, matColorEmissive );
        assimpMaterial->Get( AI_MATKEY_COLOR_TRANSPARENT, matColorTransparent );
        assimpMaterial->Get( AI_MATKEY_COLOR_REFLECTIVE, matColorReflective );
        //assimpMaterial->Get( AI_MATKEY_GLOBAL_BACKGROUND_IMAGE "?bg.global",0,0

        VA_LOG( "Assimp processing material '%s'", matName.data );

        vaRenderMaterial::MaterialSettings matSettings;
        {
            matSettings.Transparent         = false;
            matSettings.AlphaTest           = false;
            matSettings.ColorMultAlbedo     = vaVector4( matColorDiffuse.r, matColorDiffuse.g, matColorDiffuse.b, matOpacity );
            matSettings.ColorMultEmissive   = vaVector4( matColorEmissive.r, matColorEmissive.g, matColorEmissive.b, 0.0f );
            matSettings.ColorMultSpecular   = vaVector4( matColorSpecular.r, matColorSpecular.g, matColorSpecular.b, 0.0f );
            matSettings.FaceCull            = (matTwosided==0)?(vaFaceCull::Back):(vaFaceCull::None);
            matSettings.ReceiveShadows      = true;
            matSettings.Wireframe           = matWireframe != 0;
            matSettings.SpecMul             = matSpecularMul;
            matSettings.SpecPow             = matSpecularPow;
        }

        shared_ptr<vaAssetTexture> textureAlbedo   = nullptr;
        shared_ptr<vaAssetTexture> textureNormal   = nullptr;
        shared_ptr<vaAssetTexture> textureSpecular = nullptr;
        shared_ptr<vaAssetTexture> textureEmissive = nullptr;

        //////////////////////////////////////////////////////////////////////////
        // textures
        aiTextureType texTypesSupported[] = { aiTextureType_NONE, aiTextureType_DIFFUSE, aiTextureType_SPECULAR, aiTextureType_NORMALS, aiTextureType_EMISSIVE, aiTextureType_UNKNOWN };
        for( int texTypeIndex = 0; texTypeIndex < _countof( texTypesSupported ); texTypeIndex++ )
        {
            aiTextureType texType = texTypesSupported[texTypeIndex];
            for( int texIndex = 0; texIndex < (int)assimpMaterial->GetTextureCount( texType ); texIndex++ )
            {
                aiTextureMapping    texMapping      = aiTextureMapping_UV;
                uint32              texUVIndex      = 0;
                float               texBlendFactor  = 0.0f;
                aiTextureOp         texOp           = aiTextureOp_Add;
                aiTextureMapMode    texMapModes[2]  = {aiTextureMapMode_Wrap, aiTextureMapMode_Wrap};
                aiTextureFlags      texFlags        = (aiTextureFlags)0;

                //aiReturn texFound = assimpMaterial->GetTexture( texType, texIndex, &pathTmp, &texMapping, &texUVIndex, &texBlendFactor, &texOp, &texMapMode );
                aiReturn texFound = aiGetMaterialTexture( assimpMaterial, texType, texIndex, &pathTmp, &texMapping, &texUVIndex, &texBlendFactor, &texOp, texMapModes, (unsigned int *)&texFlags );
                if( texFound == aiReturn_SUCCESS )
                {
                    if( texMapping != aiTextureMapping_UV )
                    {
                        VA_LOG( "Assimp warning: Texture 's' mapping mode not supported (only aiTextureMapping_UV supported), skipping", pathTmp.data );
                        continue;
                    }
                    if( texUVIndex > 0 )
                    {
                        VA_LOG( "Assimp warning: Texture 's' UV index out of supported range, skipping", pathTmp.data );
                        continue;
                    }

                    shared_ptr<vaAssetTexture> newTextureAsset = FindOrLoadTexture( nullptr, pathTmp.data, tempStorage, parameters, outContent, texType == aiTextureType_DIFFUSE, false );
                     
                    if( (newTextureAsset == nullptr) || (newTextureAsset->GetTexture() == nullptr) )
                    {
                        VA_LOG( "Assimp warning: Texture 's' could not be imported, skipping", pathTmp.data );
                        continue;
                    }
                    
                    if( texType == aiTextureType_DIFFUSE )
                    {
                        textureAlbedo = newTextureAsset;
                    }
                    if( texType == aiTextureType_SPECULAR )
                        textureSpecular = newTextureAsset;
                    if( texType == aiTextureType_NORMALS )
                        textureNormal = newTextureAsset;
                    if( texType == aiTextureType_EMISSIVE )
                        textureEmissive = newTextureAsset;

                    if( (texType == aiTextureType_NONE) || (texType == aiTextureType_UNKNOWN) || (texType == aiTextureType_UNKNOWN) )
                    {
                        VA_LOG_WARNING( "Assimp warning: texture '%s' is of unrecognizeable type; using as albedo if not already set.", newTextureAsset->Name().c_str() );
                        if( textureAlbedo == nullptr )
                            textureAlbedo = newTextureAsset;
                    }
                }
            }
        }

        shared_ptr<vaRenderMaterial> importedMaterial = vaRenderMaterialManager::GetInstance().CreateRenderMaterial();

        if( textureAlbedo != nullptr ) 
            importedMaterial->SetTextureAlbedo( textureAlbedo->Resource );
        if( textureNormal != nullptr  )
            importedMaterial->SetTextureNormalmap( textureNormal->Resource );
        if( textureSpecular != nullptr )
            importedMaterial->SetTextureSpecular( textureSpecular->Resource );
        if( textureEmissive != nullptr )
            importedMaterial->SetTextureEmissive( textureEmissive->Resource );

        importedMaterial->SetSettings( matSettings );

        string newMaterialName = matName.data;
        newMaterialName = parameters.AssetPack.FindSuitableAssetName( newMaterialName );

        auto materialAsset = parameters.AssetPack.Add( importedMaterial, newMaterialName );

        VA_LOG_SUCCESS( "    material '%s' added", newMaterialName.c_str() );


        tempStorage.LoadedMaterials.push_back( LoadingTempStorage::LoadedMaterial( assimpMaterial, materialAsset ) );

        if( outContent != nullptr )
            outContent->LoadedAssets.push_back( materialAsset );
	}

    return true;
}

static bool ProcessMeshes( const aiScene* loadedScene, LoadingTempStorage & tempStorage, vaAssetImporter::LoadingParameters & parameters, vaAssetImporter::LoadedContent * outContent )
{
	for( unsigned int mi = 0; mi < loadedScene->mNumMeshes; mi++ )
	{
        aiMesh * assimpMesh = loadedScene->mMeshes[mi];

        bool hasTangentBitangents = assimpMesh->HasTangentsAndBitangents();

        VA_LOG( "Assimp processing mesh '%s'", assimpMesh->mName.data );

        if( !assimpMesh->HasFaces() )
        { 
            assert( false );
            VA_LOG_ERROR( "Assimp error: mesh '%s' has no faces, skipping.", assimpMesh->mName.data );
            continue;
        }

        if( assimpMesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE )
        { 
            VA_LOG_WARNING( "Assimp warning: mesh '%s' reports non-triangle primitive types - those will be skipped during import.", assimpMesh->mName.data );
        }

        if( !assimpMesh->HasPositions() )
        { 
            assert( false );
            VA_LOG_ERROR( "Assimp error: mesh '%s' does not have positions, skipping.", assimpMesh->mName.data );
            continue;
        }
        if( !assimpMesh->HasNormals() )
        { 
            //assert( false );
            VA_LOG_ERROR( "Assimp error: mesh '%s' does not have normals, skipping.", assimpMesh->mName.data );
            continue;
        }

        vector<vaVector3>   vertices;
        vector<uint32>      colors;
        vector<vaVector3>   normals;
        vector<vaVector4>   tangents;  // .w holds handedness
        vector<vaVector2>   texcoords0;
        vector<vaVector2>   texcoords1;

        vertices.resize( assimpMesh->mNumVertices );
        colors.resize( vertices.size( ) );
        normals.resize( vertices.size( ) );
        tangents.resize( vertices.size( ) );
        texcoords0.resize( vertices.size( ) );
        texcoords1.resize( vertices.size( ) );


        for( int i = 0; i < (int)vertices.size(); i++ )
            vertices[i] = vaVector3( assimpMesh->mVertices[i].x, assimpMesh->mVertices[i].y, assimpMesh->mVertices[i].z );

        if( assimpMesh->HasVertexColors( 0 ) )
        {
            for( int i = 0; i < (int)colors.size(); i++ )
                colors[i] = vaVector4::ToRGBA( vaVector4( assimpMesh->mColors[0][i].r, assimpMesh->mColors[0][i].g, assimpMesh->mColors[0][i].b, assimpMesh->mColors[0][i].a ) );
        }
        else
        {
            for( int i = 0; i < (int)tangents.size(); i++ )
                colors[i] = vaVector4::ToRGBA( vaVector4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        }

        for( int i = 0; i < (int)vertices.size(); i++ )
            normals[i] = vaVector3( assimpMesh->mNormals[i].x, assimpMesh->mNormals[i].y, assimpMesh->mNormals[i].z );

        if( hasTangentBitangents )
        {
            for( int i = 0; i < (int)tangents.size(); i++ )
            {
                vaVector3 tangent   = vaVector3( assimpMesh->mTangents[i].x, assimpMesh->mTangents[i].y, assimpMesh->mTangents[i].z );
                vaVector3 bitangent = vaVector3( assimpMesh->mBitangents[i].x, assimpMesh->mBitangents[i].y, assimpMesh->mBitangents[i].z );
                float handedness = vaVector3::Dot( bitangent, vaVector3::Cross( normals[i], tangent ).Normalize() );
                tangents[i] = vaVector4( tangent, handedness );
            }
        }
        else
        {
            // no tex coords? hmm ok just create some dummy tangents... 
            for( size_t i = 0; i < tangents.size( ); i++ )
            {
                vaVector3 bitangent = ( vertices[i] + vaVector3( 0.0f, 0.0f, -5.0f ) ).Normalize( );
                if( vaMath::Abs( vaVector3::Dot( bitangent, normals[i] ) ) > 0.9f )
                    bitangent = ( vertices[i] + vaVector3( -5.0f, 0.0f, 0.0f ) ).Normalize( );
                tangents[i] = vaVector4( vaVector3::Cross( bitangent, normals[i] ).Normalize( ), 1.0f );
            }
        }

        vector<vaVector2> * uvsOut[] = { &texcoords0, &texcoords1 };

        for( int uvi = 0; uvi < 2; uvi++ )
        {
            vector<vaVector2> & texcoords = *uvsOut[uvi];

            if( assimpMesh->HasTextureCoords( uvi ) )
            {
                for( int i = 0; i < (int)texcoords0.size(); i++ )
                    texcoords[i] = vaVector2( assimpMesh->mTextureCoords[uvi][i].x, assimpMesh->mTextureCoords[uvi][i].y );
            }
            else
            {
                for( int i = 0; i < (int)texcoords0.size(); i++ )
                    texcoords[i] = vaVector2( 0.0f, 0.0f );
            }
        }

        bool indicesOk = true;
        vector<uint32>      indices;
        indices.reserve( assimpMesh->mNumFaces * 3 );
        for( int i = 0; i < (int)assimpMesh->mNumFaces; i ++ )
        {
            if( assimpMesh->mFaces[i].mNumIndices != 3 )
            {
                continue;
                //assert( false );
                //VA_LOG_ERROR( "Assimp error: mesh '%s' face has incorrect number of indices (3), skipping.", assimpMesh->mName.data );
                //indicesOk = false;
                //break;
            }
            indices.push_back( assimpMesh->mFaces[i].mIndices[0] );
            indices.push_back( assimpMesh->mFaces[i].mIndices[1] );
            indices.push_back( assimpMesh->mFaces[i].mIndices[2] );
        }
        if( !indicesOk )
            continue;

        auto materialAsset = tempStorage.FindMaterial( loadedScene->mMaterials[assimpMesh->mMaterialIndex] );
        auto material = *materialAsset->GetMaterial();

        vector<vaRenderMesh::SubPart> parts;
        parts.resize( 1 );
        vaRenderMesh::SubPart & part = parts[0];
        part.Material   = material;
        part.MaterialID = material->UIDObject_GetUID();
        part.IndexStart = 0;
        part.IndexCount = (int)indices.size();
        
        shared_ptr<vaRenderMesh> newMesh = vaRenderMesh::Create( parameters.BaseTransform, vertices, normals, tangents, texcoords0, texcoords1, indices, vaWindingOrder::Clockwise );
        newMesh->SetParts( parts );
        newMesh->SetTangentBitangentValid( hasTangentBitangents );

        string newMeshName = assimpMesh->mName.data;
        if( newMeshName == "" )
            newMeshName = "_" + materialAsset->Name();
        newMeshName = parameters.AssetPack.FindSuitableAssetName( newMeshName );

        shared_ptr<vaAssetRenderMesh> newAsset = parameters.AssetPack.Add( newMesh, newMeshName );

        VA_LOG_SUCCESS( "    mesh '%s' added", newMeshName.c_str() );

        tempStorage.LoadedMeshes.push_back( LoadingTempStorage::LoadedMesh( assimpMesh, newAsset ) );

        if( outContent != nullptr )
            outContent->LoadedAssets.push_back( newAsset );
    }

    return true;
}

static bool ProcessScene( /*const wstring & path, */ const aiScene* loadedScene, LoadingTempStorage & tempStorage, vaAssetImporter::LoadingParameters & parameters, vaAssetImporter::LoadedContent * outContent )
{
    if( !ProcessTextures( loadedScene, tempStorage, parameters, outContent ) )
        return false;

    if( !ProcessMaterials( loadedScene, tempStorage, parameters, outContent ) )
        return false;

    if( !ProcessMeshes( loadedScene, tempStorage, parameters, outContent ) )
        return false;


    return true;
}

bool LoadFileContents_Assimp( const wstring & path, vaAssetImporter::LoadingParameters & parameters, vaAssetImporter::LoadedContent * outContent )
{
    // construct global importer and exporter instances
	Assimp::Importer importer;
	
	
	// aiString s;
    // imp.GetExtensionList(s);
    // vaLog::GetInstance().Add( "Assimp extension list: %s", s.data );

    const aiScene* loadedScene = nullptr;

    // // set loggers
    // Assimp::DefaultLogger::get()->attachStream( new myLogInfoStream(), Assimp::Logger::Info | Assimp::Logger::Debugging );
    // Assimp::DefaultLogger::get()->attachStream( new myLogWarningStream(), Assimp::Logger::Warn );
    // Assimp::DefaultLogger::get()->attachStream( new myLogErrorStream(), Assimp::Logger::Err );

    LoadingTempStorage tempStorage;
    vaStringTools::SplitPath( vaStringTools::ToLower( path ), &tempStorage.ImportDirectory, &tempStorage.ImportFileName, &tempStorage.ImportExt );

    {
        vaSimpleScopeTimerLog timerLog( vaStringTools::Format( L"Assimp parsing '%s'", path.c_str( ) ) );

        string apath = vaStringTools::SimpleNarrow( path );

        unsigned int flags = 0;
        flags |= aiProcess_JoinIdenticalVertices;
        flags |= aiProcess_MakeLeftHanded;
        flags |= aiProcess_Triangulate;

        flags |= aiProcess_RemoveComponent;

        int removeComponentFlags = aiComponent_CAMERAS | aiComponent_LIGHTS;

        flags |= aiProcess_PreTransformVertices;
        flags |= aiProcess_ValidateDataStructure;
        flags |= aiProcess_ImproveCacheLocality;
        flags |= aiProcess_RemoveRedundantMaterials;
        flags |= aiProcess_FixInfacingNormals;
        flags |= aiProcess_FindDegenerates;
        flags |= aiProcess_FindInvalidData;
        flags |= aiProcess_GenUVCoords;
        flags |= aiProcess_TransformUVCoords;
        //flags |= aiProcess_FindInstances;
        //flags |= aiProcess_OptimizeMeshes;
        //flags |= aiProcess_OptimizeGraph;
        //flags |= aiProcess_FlipUVs;
        //flags |= aiProcess_FlipWindingOrder; default is CCW - use aiProcess_FlipWindingOrder flag to switch to CW
        flags |= aiProcess_ConvertToLeftHanded;
        
        //if( parameters.RegenerateTangents )
        flags |= aiProcess_CalcTangentSpace;

        if( parameters.ForceGenerateNormals )
            parameters.GenerateNormalsIfNeeded = true;

        if( parameters.GenerateNormalsIfNeeded )
        {
            if( parameters.GenerateSmoothNormalsIfGenerating )
            {
                flags |= aiProcess_GenSmoothNormals; 
                importer.SetPropertyFloat( AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, parameters.GenerateSmoothNormalsSmoothingAngle );
            }
            else
            {
                flags |= aiProcess_GenNormals; 
            }
            if( parameters.ForceGenerateNormals )
                removeComponentFlags |= aiComponent_NORMALS;
       }

        importer.SetPropertyInteger( AI_CONFIG_PP_RVC_FLAGS, removeComponentFlags );
        importer.SetPropertyBool( "GLOB_MEASURE_TIME", true );

        loadedScene = importer.ReadFile( apath.c_str(), flags );
        if( loadedScene == nullptr )
        {
            VA_LOG_ERROR( importer.GetErrorString() );
            return false; 
        }
    }

    {
        vaSimpleScopeTimerLog timerLog( vaStringTools::Format( L"Importing Assimp scene...", path.c_str( ) ) );

        return ProcessScene( loadedScene, tempStorage, parameters, outContent );
    }

    
//    importer.

    return false;
}

#else

bool LoadFileContents_Assimp( const wstring & path, vaAssetImporter::LoadingParameters & parameters, vaAssetImporter::LoadedContent * outContent )
{
    return false;
}

#endif