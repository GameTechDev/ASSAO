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

#include "vaRendering.h"

#include "vaRenderingIncludes.h"

#include "vaTexture.h"

#include "EmbeddedRenderingMedia.cpp"

#include "vaAssetPack.h"

#include "vaRenderMesh.h"

using namespace VertexAsylum;


vaRenderingCore::vaRenderingCore( ) 
{
    vaRenderingModuleRegistrar::CreateSingletonIfNotCreated( );

    for( int i = 0; i < BINARY_EMBEDDER_ITEM_COUNT; i++ )
    {
        wchar_t * name = BINARY_EMBEDDER_NAMES[i];
        unsigned char * data = BINARY_EMBEDDER_DATAS[i];
        int64 dataSize = BINARY_EMBEDDER_SIZES[i];
        int64 timeStamp = BINARY_EMBEDDER_TIMES[i];

        vaFileTools::EmbeddedFilesRegister( name, data, dataSize, timeStamp );
    }
}

vaRenderingCore::~vaRenderingCore( ) 
{
    vaRenderingModuleRegistrar::DeleteSingleton( );
}

void    vaRenderingCore::RegisterAssetSearchPath( const wstring & searchPath, bool pushBack )
{
    wstring cleanedSearchPath = vaFileTools::CleanupPath( searchPath + L"\\", false );
    if( pushBack )
        m_assetSearchPaths.push_back( cleanedSearchPath );
    else
        m_assetSearchPaths.push_front( cleanedSearchPath );
}

wstring vaRenderingCore::FindAssetFilePath( const wstring & fileName )
{
    for( unsigned int i = 0; i < m_assetSearchPaths.size( ); i++ )
    {
        std::wstring filePath = m_assetSearchPaths[i] + L"\\" + fileName;
        if( vaFileTools::FileExists( filePath.c_str( ) ) )
        {
            return vaFileTools::CleanupPath( filePath, false );
        }
        if( vaFileTools::FileExists( ( vaCore::GetWorkingDirectory( ) + filePath ).c_str( ) ) )
        {
            return vaFileTools::CleanupPath( vaCore::GetWorkingDirectory( ) + filePath, false );
        }
    }

    if( vaFileTools::FileExists( ( vaCore::GetWorkingDirectory( ) + fileName ).c_str( ) ) )
        return vaFileTools::CleanupPath( vaCore::GetWorkingDirectory( ) + fileName, false );
    if( vaFileTools::FileExists( ( vaCore::GetExecutableDirectory( ) + fileName ).c_str( ) ) )
        return vaFileTools::CleanupPath( vaCore::GetExecutableDirectory( ) + fileName, false );
    if( vaFileTools::FileExists( fileName.c_str( ) ) )
        return vaFileTools::CleanupPath( fileName, false );

    return L"";

}

void vaRenderingCore::OnAPIInitialized( )
{
    VA_RENDERING_MODULE_CREATE( vaRenderMaterialManager );
    VA_RENDERING_MODULE_CREATE( vaRenderMeshManager );
    new vaAssetPackManager( );
}

void vaRenderingCore::OnAPIAboutToBeDeinitialized( )
{

}

void vaRenderingCore::Initialize( )
{
    assert( !IsInitialized() );
    new vaRenderingCore();

    InitializePlatform( );
}

void vaRenderingCore::Deinitialize( )
{
    assert( IsInitialized( ) );
    delete GetInstancePtr();
}

void vaRenderingModuleRegistrar::RegisterModule( const std::string & name, std::function< vaRenderingModule * ( const vaConstructorParamsBase * )> moduleCreateFunction )
{
    // make sure the singleton is alive
    if( GetInstancePtr() == NULL )
        new vaRenderingModuleRegistrar( );

    assert( name != "" );

    auto it = GetInstance().m_modules.find( name );
    if( it != GetInstance().m_modules.end( ) )
    {
        VA_ERROR( L"vaRenderingCore::RegisterModule - name '%s' already registered!", name.c_str() );
        return;
    }
    GetInstance().m_modules.insert( std::pair< std::string, ModuleInfo >( name, ModuleInfo( moduleCreateFunction ) ) );
}

vaRenderingModule * vaRenderingModuleRegistrar::CreateModule( const std::string & name, const vaConstructorParamsBase * params )
{
    auto it = GetInstance().m_modules.find( name );
    if( it == GetInstance().m_modules.end( ) )
    {
        wstring wname = vaStringTools::SimpleWiden( name );
        VA_ERROR( L"vaRenderingCore::CreateModule - name '%s' not registered.", wname.c_str( ) );
        return NULL;
    }

    vaRenderingModule * ret = ( *it ).second.ModuleCreateFunction( params );

    ret->InternalRenderingModuleSetTypeName( name );

    return ret;
}

void vaRenderingModuleRegistrar::CreateSingletonIfNotCreated( )
{
    if( vaRenderingModuleRegistrar::GetInstancePtr( ) == NULL )
        new vaRenderingModuleRegistrar( );
}

void vaRenderingModuleRegistrar::DeleteSingleton( )
{
    if( vaRenderingModuleRegistrar::GetInstancePtr() != NULL )
        delete vaRenderingModuleRegistrar::GetInstancePtr();
}


