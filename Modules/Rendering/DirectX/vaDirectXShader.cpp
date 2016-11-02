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

#include "vaDirectXShader.h"

#include "vaDirectXTools.h"

#include <assert.h>

#include "sys/stat.h"

#include <D3DCompiler.h>

#include <sstream>

#include <inttypes.h>

#define           USE_SHADER_CACHE_SYSTEM

#ifdef NDEBUG
#define           STILL_LOAD_FROM_CACHE_IF_ORIGINAL_FILE_MISSING
#endif

using namespace std;

namespace VertexAsylum
{
    // this is used to make the error reporting report to correct file (or embedded storage)
    string CorrectErrorIfNotFullPath( const char * errorText )
    {
        string ret;

        string codeStr( errorText );

        stringstream ss( codeStr );
        string line;
        int lineNum = 1;
        while( std::getline( ss, line ) )
        {
            size_t fileSeparator = line.find( ':' );
            if( fileSeparator != string::npos )
            {
                string filePlusLinePart = line.substr( 0, fileSeparator );
                string errorPart = line.substr( fileSeparator );

                size_t lineInfoSeparator = filePlusLinePart.find( '(' );
                if( lineInfoSeparator != string::npos )
                {
                    string filePart = filePlusLinePart.substr( 0, lineInfoSeparator );
                    string lineInfoPart = filePlusLinePart.substr( lineInfoSeparator );

                    wstring fileName = vaStringTools::SimpleWiden( filePart );

                    // First try the file system...
                    wstring fullFileName = vaDirectXShaderManager::GetInstance( ).FindShaderFile( fileName.c_str( ) );
                    if( fullFileName != L"" )
                    {
                        filePart = vaStringTools::SimpleNarrow( fullFileName );
                    }
                    else
                    {
                        // ...then try embedded storage
                        vaFileTools::EmbeddedFileData embeddedData = vaFileTools::EmbeddedFilesFind( wstring( L"shaders:\\" ) + fileName );
                        if( embeddedData.HasContents() )
                            filePart = vaStringTools::SimpleNarrow( wstring( L"shaders:\\" ) + fileName );
                    }

                    line = filePart + lineInfoPart + errorPart;
                }
            }
            ret += line + "\n";
        }
        return ret;
    }

    class vaShaderIncludeHelper : public ID3DInclude
    {
        std::vector<vaShaderCacheEntry::FileDependencyInfo> &     m_dependenciesCollector;

        vaShaderIncludeHelper( const vaShaderIncludeHelper & c ) : m_dependenciesCollector( c.m_dependenciesCollector )
        {
            assert( false );
        }    // to prevent warnings (and also this object doesn't support copying/assignment)
        void operator = ( const vaShaderIncludeHelper & )
        {
            assert( false );
        }    // to prevent warnings (and also this object doesn't support copying/assignment)

    public:
        vaShaderIncludeHelper( std::vector<vaShaderCacheEntry::FileDependencyInfo> & dependenciesCollector ) : m_dependenciesCollector( dependenciesCollector ) { }

        STDMETHOD( Open )( D3D10_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes )
        {
            vaShaderCacheEntry::FileDependencyInfo fileDependencyInfo;
            std::shared_ptr<vaMemoryStream>        memBuffer;

            wstring fileName = vaStringTools::SimpleWiden( string( pFileName ) );

            // First try the file system...
            wstring fullFileName = vaDirectXShaderManager::GetInstance( ).FindShaderFile( fileName.c_str( ) );
            if( fullFileName != L"" )
            {
                fileDependencyInfo = vaShaderCacheEntry::FileDependencyInfo( fileName.c_str( ) );
                memBuffer = vaFileTools::LoadFileToMemoryStream( fullFileName.c_str( ) );
            }
            else
            {
                // ...then try embedded storage
                vaFileTools::EmbeddedFileData embeddedData = vaFileTools::EmbeddedFilesFind( wstring( L"shaders:\\" ) + fileName );

                if( !embeddedData.HasContents( ) )
                {
                    VA_ERROR( L"Error trying to find shader file '%s'!", fileName.c_str( ) );
                    return E_FAIL;
                }
                fileDependencyInfo = vaShaderCacheEntry::FileDependencyInfo( fileName.c_str( ), embeddedData.TimeStamp );
                memBuffer = embeddedData.MemStream;
            }

            if( memBuffer != NULL )
            {
                m_dependenciesCollector.push_back( fileDependencyInfo );

                int length = (int)memBuffer->GetLength( );
                void * pData = new char[length];
                memcpy( pData, memBuffer->GetBuffer( ), length );
                *ppData = pData;
                *pBytes = length;
                return S_OK;
            }

            return E_FAIL;
        }
        STDMETHOD( Close )( LPCVOID pData )
        {
            delete[] pData;
            return S_OK;
        }
    };

    void vaDirectXShader::AddBuiltInMacros( )
    {
        m_macros.push_back( pair<string, string>( "VA_COMPILED_AS_SHADER_CODE", "" ) );
    }

    void vaDirectXShader::CreateShaderFromFile( const wchar_t * filePath, const char * shaderModel, const char * entryPoint, D3D_SHADER_MACRO * macros )
    {
        DestroyShader( );
        m_shaderCode = "";
        m_shaderFilePath = filePath;
        m_entryPoint = entryPoint;
        m_shaderModel = shaderModel;
        StoreMacros( macros );
        AddBuiltInMacros( );
        CreateShader( );
    }

    void vaDirectXShader::CreateShaderFromBuffer( const char * shaderCode, const char * shaderModel, const char * entryPoint, D3D_SHADER_MACRO * macros )
    {
        DestroyShader( );
        m_shaderCode = shaderCode;
        m_shaderFilePath = L"";
        m_entryPoint = entryPoint;
        m_shaderModel = shaderModel;
        StoreMacros( macros );
        AddBuiltInMacros( );
        CreateShader( );
    }
    //
    void vaDirectXShader::CreateShaderFromFile( const wchar_t * filePath, const char * shaderModel, const char * entryPoint, const vector<pair<string, string>> & macros )
    {
        DestroyShader( );
        m_shaderCode = "";
        m_shaderFilePath = filePath;
        m_entryPoint = entryPoint;
        m_shaderModel = shaderModel;
        m_macros = macros;
        AddBuiltInMacros( );
        CreateShader( );
    }

    void vaDirectXShader::CreateShaderFromBuffer( const char * shaderCode, const char * shaderModel, const char * entryPoint, const vector<pair<string, string>> & macros )
    {
        DestroyShader( );
        m_shaderCode = shaderCode;
        m_shaderFilePath = L"";
        m_entryPoint = entryPoint;
        m_shaderModel = shaderModel;
        m_macros = macros;
        AddBuiltInMacros( );
        CreateShader( );
    }
    //
    vaDirectXShader::vaDirectXShader( )
    {
        m_shader = NULL;

        m_entryPoint = "";
        m_shaderFilePath = L"";
        m_shaderCode = "";
        m_shaderModel = "";

        // prevent asserts
        m_helperMacroConstructorCalled = true;
        m_helperMacroDesctructorCalled = true;
        
        m_lastLoadedFromCache = false;
        GetAllShadersList( ).push_back( this );

#ifdef VA_HOLD_SHADER_DISASM
        m_disasmAutoDumpToFile = false;
#endif
    }
    //
    vaDirectXShader::~vaDirectXShader( )
    {
        DestroyShaderBase( );

        bool found = false;
        for( size_t i = 0; i < GetAllShadersList( ).size( ); i++ )
        {
            if( GetAllShadersList( )[i] == this )
            {
                GetAllShadersList( ).erase( GetAllShadersList( ).begin( ) + i );
                found = true;
                break;
            }
        }
        assert( found );
    }
    //
    void vaDirectXShader::Clear( )
    {
        DestroyShader();

        m_shader = NULL;

        m_entryPoint = "";
        m_shaderFilePath = L"";
        m_shaderCode = "";
        m_shaderModel = "";
#ifdef VA_HOLD_SHADER_DISASM
        m_disasm = "";
#endif
    }
    //
    std::vector<vaDirectXShader *> & vaDirectXShader::GetAllShadersList( )
    {
        return vaDirectXShaderManager::GetInstance( ).GetAllShaderList();
    }
    //
    void vaDirectXShader::StoreMacros( CONST D3D_SHADER_MACRO * macroDefines )
    {
        m_macros.clear( );

        if( macroDefines == NULL )
            return;

        CONST D3D_SHADER_MACRO* pNext = macroDefines;
        while( pNext->Name != NULL && pNext->Definition != NULL )
        {
            m_macros.push_back( pair<string,string>( pNext->Name, pNext->Definition ) );
            pNext++;

            if( m_macros.size( ) > c_maxMacros - 1 )
            {
                assert( false ); // no more than c_maxMacros-1 supported, increase c_maxMacros
                break;
            }
        }
    }
    //
    const D3D_SHADER_MACRO * vaDirectXShader::GetStoredMacros( D3D_SHADER_MACRO * buffer, int bufferElementCount )
    {
        if( m_macros.size( ) == 0 )
            return NULL;

        if( (size_t)bufferElementCount <= m_macros.size() )
        {
            // output buffer element too small!
            assert( false );
            return NULL;
        }

        size_t i;
        for( i = 0; i < m_macros.size( ); i++ )
        {
            buffer[i].Name = m_macros[i].first.c_str( );
            buffer[i].Definition = m_macros[i].second.c_str( );
        }
        buffer[i].Name = NULL;
        buffer[i].Definition = NULL;

        return buffer;
    }

    void vaDirectXShader::ReloadAllShaders( )
    {
        vaLog::GetInstance( ).Add( LOG_COLORS_SHADERS, L"Recompiling shaders..." );

        int totalLoaded = (int)GetAllShadersList().size();
        int totalLoadedFromCache = 0;
        for( size_t i = 0; i < GetAllShadersList( ).size( ); i++ )
        {
            GetAllShadersList( )[i]->Reload( );
            if( GetAllShadersList( )[i]->IsLoadedFromCache( ) ) 
                totalLoadedFromCache++;
        }
        
        vaLog::GetInstance( ).Add( LOG_COLORS_SHADERS, L"... %d shaders reloaded (%d from cache)", totalLoaded, totalLoadedFromCache );
    }

    void vaDirectXShader::Reload( D3D_SHADER_MACRO * newMacroDefines )
    {
        StoreMacros( newMacroDefines );
        Reload( );
    }
    //
    void vaDirectXShader::Reload( const std::vector<std::pair<std::string, std::string>> & macros )
    {
        m_macros = macros;
        Reload( );
    }
    //
    void vaDirectXShader::OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain )
    {
        DestroyShader( );
        CreateShader( );
    }
    //
    void vaDirectXShader::OnDeviceDestroyed( )
    {
        DestroyShader( );
    }
    //
    void vaDirectXShader::DestroyShaderBase( )
    {
        m_lastLoadedFromCache = false;
        SAFE_RELEASE( m_shader );
    }
    //
    void vaDirectXShader::CreateCacheKey( vaShaderCacheKey & outKey )
    {
        // supposed to be empty!
        assert( outKey.StringPart.size( ) == 0 );
        outKey.StringPart.clear( );

        outKey.StringPart += vaStringTools::Format( "%d", (int)m_macros.size( ) ) + " ";
        for( uint32 i = 0; i < m_macros.size( ); i++ )
        {
            outKey.StringPart += m_macros[i].first + " ";
            outKey.StringPart += m_macros[i].second + " ";
        }
        outKey.StringPart += m_shaderModel + " ";
        outKey.StringPart += m_entryPoint + " ";
        outKey.StringPart += vaStringTools::ToLower( vaStringTools::SimpleNarrow( m_shaderFilePath ) ) + " ";
    }
    //
    ID3D11PixelShader * vaDirectXPixelShader::GetShader( )
    {
        if( m_shader == NULL )
        {
            return NULL;
        }

        HRESULT hr;
        ID3D11PixelShader * ret;
        V( m_shader->QueryInterface( IID_ID3D11PixelShader, (void**)&ret ) );
        ret->Release( );

        assert( ret != NULL );

        return ret;
    }
    //
    ID3D11ComputeShader * vaDirectXComputeShader::GetShader( )
    {
        if( m_shader == NULL )
        {
            assert( false );
            return NULL;
        }

        HRESULT hr;
        ID3D11ComputeShader * ret;
        V( m_shader->QueryInterface( IID_ID3D11ComputeShader, (void**)&ret ) );
        ret->Release( );

        assert( ret != NULL );

        return ret;
    }
    //
    ID3D11VertexShader * vaDirectXVertexShader::GetShader( )
    {
        if( m_shader == NULL )
        {
            assert( false );
            return NULL;
        }

        HRESULT hr;
        ID3D11VertexShader * ret;
        V( m_shader->QueryInterface( IID_ID3D11VertexShader, (void**)&ret ) );
        ret->Release( );

        assert( ret != NULL );

        return ret;
    }
    //
    ID3D11HullShader * vaDirectXHullShader::GetShader( )
    {
        if( m_shader == NULL )
        {
            assert( false );
            return NULL;
        }

        HRESULT hr;
        ID3D11HullShader * ret;
        V( m_shader->QueryInterface( IID_ID3D11HullShader, (void**)&ret ) );
        ret->Release( );

        assert( ret != NULL );

        return ret;
    }
    //
    ID3D11DomainShader * vaDirectXDomainShader::GetShader( )
    {
        if( m_shader == NULL )
        {
            assert( false );
            return NULL;
        }

        HRESULT hr;
        ID3D11DomainShader * ret;
        V( m_shader->QueryInterface( IID_ID3D11DomainShader, (void**)&ret ) );
        ret->Release( );

        assert( ret != NULL );

        return ret;
    }
    //
    ID3D11GeometryShader * vaDirectXGeometryShader::GetShader( )
    {
        if( m_shader == NULL )
        {
            assert( false );
            return NULL;
        }

        HRESULT hr;
        ID3D11GeometryShader * ret;
        V( m_shader->QueryInterface( IID_ID3D11GeometryShader, (void**)&ret ) );
        ret->Release( );

        assert( ret != NULL );

        return ret;
    }
    //
    void vaDirectXVertexShader::CreateCacheKey( vaShaderCacheKey & outKey )
    {
        vaDirectXShader::CreateCacheKey( outKey );

        outKey.StringPart += vaStringTools::Format( "%d ", (int)m_inputLayoutElements.size( ) ) + " ";
        for( uint32 i = 0; i < m_inputLayoutElements.size( ); i++ )
        {
            outKey.StringPart += string( m_inputLayoutElements[i].SemanticName ) + " ";

            outKey.StringPart += vaStringTools::Format( "%x ", (uint32)m_inputLayoutElements[i].SemanticIndex );
            outKey.StringPart += vaStringTools::Format( "%x ", (uint32)m_inputLayoutElements[i].Format );
            outKey.StringPart += vaStringTools::Format( "%x ", (uint32)m_inputLayoutElements[i].InputSlot );
            outKey.StringPart += vaStringTools::Format( "%x ", (uint32)m_inputLayoutElements[i].AlignedByteOffset );
            outKey.StringPart += vaStringTools::Format( "%x ", (uint32)m_inputLayoutElements[i].InputSlotClass );
            outKey.StringPart += vaStringTools::Format( "%x ", (uint32)m_inputLayoutElements[i].InstanceDataStepRate );
        }
        // add input-layout stuff here
    }
    //
    HRESULT CompileShaderFromFile( const wchar_t* szFileName, const D3D10_SHADER_MACRO* pDefines, LPCSTR szEntryPoint,
        LPCSTR szShaderModel, ID3DBlob** ppBlobOut, vector<vaShaderCacheEntry::FileDependencyInfo> & outDependencies )
    {
        HRESULT hr = S_OK;


        DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
        // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
        // Setting this flag improves the shader debugging experience, but still allows 
        // the shaders to be optimized and to run exactly the way they will run in 
        // the release configuration of this program.
        dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
        dwShaderFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;

        if( vaDirectXShaderManager::GetInstance( ).Settings().WarningsAreErrors )
            dwShaderFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;

        // find the file
        wstring fullFileName = vaDirectXShaderManager::GetInstance( ).FindShaderFile( szFileName );

        if( fullFileName != L"" )
        {
            bool attemptReload;

            do
            {
                outDependencies.clear( );
                outDependencies.push_back( vaShaderCacheEntry::FileDependencyInfo( szFileName ) );
                vaShaderIncludeHelper includeHelper( outDependencies );

                attemptReload = false;
                ID3DBlob* pErrorBlob;
                hr = D3DCompileFromFile( fullFileName.c_str( ), pDefines, (ID3DInclude*)&includeHelper,
                    szEntryPoint, szShaderModel, dwShaderFlags, 0, ppBlobOut, &pErrorBlob );
                if( FAILED( hr ) )
                {
                    if( pErrorBlob != NULL )
                    {
                        wstring absFileName = vaFileTools::GetAbsolutePath( fullFileName );

                        OutputDebugStringA( vaStringTools::Format( "\nError while compiling '%s' shader, SM: '%s', EntryPoint: '%s' :\n", vaStringTools::SimpleNarrow(absFileName).c_str(), szShaderModel, szEntryPoint ).c_str() );
                        OutputDebugStringA( CorrectErrorIfNotFullPath( (char*)pErrorBlob->GetBufferPointer( ) ).c_str() );
#if 1 || defined( _DEBUG ) // display always for now
                        wstring errorMsg = vaStringTools::SimpleWiden( (char*)pErrorBlob->GetBufferPointer( ) );
                        wstring shmStr = vaStringTools::SimpleWiden( szShaderModel );
                        wstring entryStr = vaStringTools::SimpleWiden( szEntryPoint );
                        wstring fullMessage = vaStringTools::Format( L"Error while compiling '%s' shader, SM: '%s', EntryPoint: '%s'."
                            L"\n\n---\n%s\n---\nAttempt reload?",
                            fullFileName.c_str( ), shmStr.c_str( ), entryStr.c_str( ), errorMsg.c_str( ) );
                        if( MessageBox( NULL, fullMessage.c_str( ), L"Shader compile error", MB_YESNO | MB_SETFOREGROUND ) == IDYES )
                            attemptReload = true;
#endif
                    }
                    SAFE_RELEASE( pErrorBlob );
                    if( !attemptReload )
                        return hr;
                }
                SAFE_RELEASE( pErrorBlob );
            } while( attemptReload );
        }
        else
        {
            // ...then try embedded storage
            vaFileTools::EmbeddedFileData embeddedData = vaFileTools::EmbeddedFilesFind( wstring( L"shaders:\\" ) + wstring( szFileName ) );

            if( !embeddedData.HasContents( ) )
            {
                VA_ERROR( L"Error trying to find shader file '%s'!", szFileName );
                return E_FAIL;
            }

            vaShaderCacheEntry::FileDependencyInfo fileDependencyInfo = vaShaderCacheEntry::FileDependencyInfo( szFileName, embeddedData.TimeStamp );

            outDependencies.clear( );
            outDependencies.push_back( fileDependencyInfo );
            vaShaderIncludeHelper includeHelper( outDependencies );

            ID3DBlob* pErrorBlob;
            string ansiName = vaStringTools::SimpleNarrow( embeddedData.Name );
            hr = D3DCompile( (LPCSTR)embeddedData.MemStream->GetBuffer( ), (SIZE_T)embeddedData.MemStream->GetLength( ), ansiName.c_str( ), pDefines, (ID3DInclude*)&includeHelper,
                szEntryPoint, szShaderModel, dwShaderFlags, 0, ppBlobOut, &pErrorBlob );
            if( FAILED( hr ) )
            {
                if( pErrorBlob != NULL )
                {
                    OutputDebugStringA( CorrectErrorIfNotFullPath( (char*)pErrorBlob->GetBufferPointer( ) ).c_str() );
#if 1 || defined( _DEBUG ) // display always for now
                    wstring errorMsg = vaStringTools::SimpleWiden( (char*)pErrorBlob->GetBufferPointer( ) );
                    wstring shmStr = vaStringTools::SimpleWiden( szShaderModel );
                    wstring entryStr = vaStringTools::SimpleWiden( szEntryPoint );
                    wstring fullMessage = vaStringTools::Format( L"Error while compiling '%s' shader, SM: '%s', EntryPoint: '%s'."
                        L"\n\n---\n%s\n---",
                        fullFileName.c_str( ), shmStr.c_str( ), entryStr.c_str( ), errorMsg.c_str( ) );
                    MessageBox( NULL, fullMessage.c_str( ), L"Shader compile error", MB_OK | MB_SETFOREGROUND );
#endif
                }
                SAFE_RELEASE( pErrorBlob );
                return hr;
            }
            SAFE_RELEASE( pErrorBlob );
        }

        return S_OK;
    }
    //
    HRESULT CompileShaderFromBuffer( const string & shaderCode, const D3D10_SHADER_MACRO* pDefines, LPCSTR szEntryPoint,
        LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
    {
        HRESULT hr = S_OK;

        DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
        // Set the D3D10_SHADER_DEBUG flag to embed debug information in the shaders.
        // Setting this flag improves the shader debugging experience, but still allows 
        // the shaders to be optimized and to run exactly the way they will run in 
        // the release configuration of this program.
        dwShaderFlags |= D3D10_SHADER_DEBUG;
#endif

        ID3DBlob* pErrorBlob;
        hr = D3DCompile( shaderCode.c_str( ), shaderCode.size( ) + 1, NULL, pDefines, NULL, szEntryPoint, szShaderModel,
            dwShaderFlags, 0, ppBlobOut, &pErrorBlob );
        if( FAILED( hr ) )
        {
            if( pErrorBlob != NULL )
            {
                OutputDebugStringA( CorrectErrorIfNotFullPath( (char*)pErrorBlob->GetBufferPointer( ) ).c_str() );
#if 1 || defined( _DEBUG ) // display always for now
                wstring errorMsg = vaStringTools::SimpleWiden( (char*)pErrorBlob->GetBufferPointer( ) );
                wstring fullMessage = vaStringTools::Format( L"Error while compiling '%s' shader, SM: '%s', EntryPoint: '%s'. \n\n---\n%s\n---",
                    L"<from_buffer>", szShaderModel, szEntryPoint, errorMsg.c_str( ) );
                MessageBox( NULL, fullMessage.c_str( ), L"Shader compile error", MB_OK | MB_SETFOREGROUND );
#endif
            }
            SAFE_RELEASE( pErrorBlob );
            return hr;
        }
        SAFE_RELEASE( pErrorBlob );
        return S_OK;
    }
    //
    ID3DBlob * vaDirectXShader::CreateShaderBase( bool & loadedFromCache )
    {
        ID3DBlob * shaderBlob = NULL;
        loadedFromCache = false;

        ID3D11Device * device = vaDirectXCore::GetDevice( );
        if( device == NULL )
        {
            return NULL;
        }
        if( ( m_shaderFilePath.size( ) == 0 ) && ( m_shaderCode.size( ) == 0 ) )
        {
            // still no path or code set provided
            return NULL;
        }

        if( m_shaderFilePath.size( ) != 0 )
        {
            vaShaderCacheKey cacheKey;
            CreateCacheKey( cacheKey );

#ifdef USE_SHADER_CACHE_SYSTEM
            bool foundButModified;
            shaderBlob = vaDirectXShaderManager::GetInstance( ).FindInCache( cacheKey, foundButModified );

            if( shaderBlob == nullptr )
            {
                wstring entryw = vaStringTools::SimpleWiden( m_entryPoint );
                wstring shadermodelw = vaStringTools::SimpleWiden( m_shaderModel );

                if( foundButModified )
                    vaLog::GetInstance( ).Add( LOG_COLORS_SHADERS, L" > file '%s' for '%s', entry '%s', found in cache but modified; recompiling...", m_shaderFilePath.c_str(), shadermodelw.c_str(), entryw.c_str() );
                else
                    vaLog::GetInstance( ).Add( LOG_COLORS_SHADERS, L" > file '%s' for '%s', entry '%s', not found in cache; recompiling...", m_shaderFilePath.c_str(), shadermodelw.c_str(), entryw.c_str() );
            }
#endif

            loadedFromCache = shaderBlob != nullptr;

            if( shaderBlob == NULL )
            {
                vector<vaShaderCacheEntry::FileDependencyInfo> dependencies;

                D3D_SHADER_MACRO macrosBuffer[ c_maxMacros ];

                CompileShaderFromFile( m_shaderFilePath.c_str( ), GetStoredMacros( macrosBuffer, c_maxMacros ), m_entryPoint.c_str( ), m_shaderModel.c_str( ), &shaderBlob, dependencies );

                if( shaderBlob != NULL )
                {
                    vaDirectXShaderManager::GetInstance( ).AddToCache( cacheKey, shaderBlob, dependencies );
                }
            }
        }
        else if( m_shaderCode.size( ) != 0 )
        {
            D3D_SHADER_MACRO macrosBuffer[c_maxMacros];
            CompileShaderFromBuffer( m_shaderCode, GetStoredMacros( macrosBuffer, c_maxMacros ), m_entryPoint.c_str( ), m_shaderModel.c_str( ), &shaderBlob );
        }
        else
        {
            assert( false );
        }


#ifdef VA_HOLD_SHADER_DISASM
        {
            ID3DBlob * disasmBlob = NULL;
            
            HRESULT hr = D3DDisassemble( shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), 0, nullptr, &disasmBlob );
            assert( SUCCEEDED( hr ) );

            m_disasm = "";
            if( (disasmBlob != nullptr) && (disasmBlob->GetBufferPointer() != nullptr) )
            {
                m_disasm = (const char *)disasmBlob->GetBufferPointer();

                if( m_disasmAutoDumpToFile )
                {
                    wstring fileName = vaCore::GetWorkingDirectory( );

                    vaShaderCacheKey cacheKey;
                    CreateCacheKey( cacheKey );
                    vaCRC64 crc; 
                    crc.AddString( cacheKey.StringPart );

                    fileName += L"shaderdump_" + vaStringTools::SimpleWiden( m_entryPoint ) + L"_" + vaStringTools::SimpleWiden( m_shaderModel ) /*+ L"_" + vaStringTools::SimpleWiden( vaStringTools::Format( "0x%" PRIx64, crc.GetCurrent() ) )*/ + L".shaderasm";

                    vaStringTools::WriteTextFile( fileName, m_disasm );
                }

            }

            SAFE_RELEASE( disasmBlob );
        }
#endif

        return shaderBlob;
    }
    //
    void vaDirectXPixelShader::CreateShader( )
    {
        HRESULT hr;
        bool loadedFromCache;
        ID3DBlob * shaderBlob = CreateShaderBase( loadedFromCache );

        if( shaderBlob == NULL )
        {
            return;
        }
        m_lastLoadedFromCache = loadedFromCache;

        ID3D11PixelShader * shader = NULL;
        hr = vaDirectXCore::GetDevice( )->CreatePixelShader( shaderBlob->GetBufferPointer( ), shaderBlob->GetBufferSize( ), NULL, &shader );
        assert( SUCCEEDED( hr ) );
        if( SUCCEEDED( hr ) )
        {
            V( shader->QueryInterface( IID_IUnknown, (void**)&m_shader ) );
            shader->Release( );
            vaDirectXCore::NameObject( shader, "vaDirectXPixelShader shader object" );
        }

        SAFE_RELEASE( shaderBlob );
    }
    //
    void vaDirectXComputeShader::CreateShader( )
    {
        HRESULT hr;
        bool loadedFromCache;
        ID3DBlob * shaderBlob = CreateShaderBase( loadedFromCache );

        if( shaderBlob == NULL )
        {
            return;
        }
        m_lastLoadedFromCache = loadedFromCache;

        ID3D11ComputeShader * shader = NULL;
        hr = vaDirectXCore::GetDevice( )->CreateComputeShader( shaderBlob->GetBufferPointer( ), shaderBlob->GetBufferSize( ), NULL, &shader );
        assert( SUCCEEDED( hr ) );
        if( SUCCEEDED( hr ) )
        {
            V( shader->QueryInterface( IID_IUnknown, (void**)&m_shader ) );
            shader->Release( );
            vaDirectXCore::NameObject( shader, "vaDirectXComputeShader shader object" );
        }

        SAFE_RELEASE( shaderBlob );

    }
    //
    vaDirectXVertexShader::vaDirectXVertexShader( )
    {
        m_inputLayout = NULL;
    }
    //
    vaDirectXVertexShader::~vaDirectXVertexShader( )
    {
        SAFE_RELEASE( m_inputLayout );
        m_inputLayoutElements.clear( );
    }
    //
    void vaDirectXVertexShader::SetInputLayout( D3D11_INPUT_ELEMENT_DESC * elements, uint32 elementCount )
    {
        m_inputLayoutElements.clear( );
        for( uint32 i = 0; i < elementCount; i++ )
            m_inputLayoutElements.push_back( elements[i] );
    }
    //
    void vaDirectXVertexShader::CreateShaderAndILFromFile( const wchar_t * filePath, const char * shaderModel, const char * entryPoint, D3D_SHADER_MACRO * macros, D3D11_INPUT_ELEMENT_DESC * elements, uint32 elementCount )
    {
        SetInputLayout( elements, elementCount );
        vaDirectXShader::CreateShaderFromFile( filePath, shaderModel, entryPoint, macros );
    }
    //
    void vaDirectXVertexShader::CreateShaderAndILFromBuffer( const char * shaderCode, const char * shaderModel, const char * entryPoint, D3D_SHADER_MACRO * macros, D3D11_INPUT_ELEMENT_DESC * elements, uint32 elementCount )
    {
        SetInputLayout( elements, elementCount );
        vaDirectXShader::CreateShaderFromBuffer( shaderCode, shaderModel, entryPoint, macros );
    }
    //
    void vaDirectXVertexShader::CreateShaderAndILFromFile( const wchar_t * filePath, const char * shaderModel, const char * entryPoint, const MacroContaner & macros, D3D11_INPUT_ELEMENT_DESC * elements, uint32 elementCount )
    {
        SetInputLayout( elements, elementCount );
        vaDirectXShader::CreateShaderFromFile( filePath, shaderModel, entryPoint, macros );
    }
    //
    void vaDirectXVertexShader::CreateShaderAndILFromBuffer( const char * shaderCode, const char * shaderModel, const char * entryPoint, const MacroContaner & macros, D3D11_INPUT_ELEMENT_DESC * elements, uint32 elementCount )
    {
        SetInputLayout( elements, elementCount );
        vaDirectXShader::CreateShaderFromBuffer( shaderCode, shaderModel, entryPoint, macros );
    }
    //
    void vaDirectXVertexShader::CreateShader( )
    {
        HRESULT hr;
        ID3DBlob * shaderBlob = NULL;

        if( m_shader == NULL )
        {
            bool loadedFromCache;
            shaderBlob = CreateShaderBase( loadedFromCache );

            if( shaderBlob == NULL )
            {
                return;
            }
            m_lastLoadedFromCache = loadedFromCache;

            ID3D11VertexShader * shader = NULL;
            hr = vaDirectXCore::GetDevice( )->CreateVertexShader( shaderBlob->GetBufferPointer( ), shaderBlob->GetBufferSize( ), NULL, &shader );
            assert( SUCCEEDED( hr ) );
            if( SUCCEEDED( hr ) )
            {
                V( shader->QueryInterface( IID_IUnknown, (void**)&m_shader ) );
                shader->Release( );
                vaDirectXCore::NameObject( shader, "vaDirectXVertexShader shader object" );
            }
        }

        if( ( m_inputLayout == NULL ) && ( m_inputLayoutElements.size( ) > 0 ) && ( shaderBlob != NULL ) )
        {
            D3D11_INPUT_ELEMENT_DESC elements[64];
            if( m_inputLayoutElements.size( ) > _countof( elements ) )
            {
                assert( false );
                SAFE_RELEASE( shaderBlob );
                return;
            }
            for( uint32 i = 0; i < m_inputLayoutElements.size( ); i++ )
                elements[i] = m_inputLayoutElements[i];

            V( vaDirectXCore::GetDevice( )->CreateInputLayout( elements, (UINT)m_inputLayoutElements.size( ),
                shaderBlob->GetBufferPointer( ), shaderBlob->GetBufferSize( ), &m_inputLayout ) );

            vaDirectXCore::NameObject( m_inputLayout, "vaDirectXVertexShader input layout object" );
        }
        SAFE_RELEASE( shaderBlob );
    }
    //
    void vaDirectXHullShader::CreateShader( )
    {
        HRESULT hr;
        bool loadedFromCache;
        ID3DBlob * shaderBlob = CreateShaderBase( loadedFromCache );

        if( shaderBlob == NULL )
        {
            return;
        }
        m_lastLoadedFromCache = loadedFromCache;

        ID3D11HullShader * shader = NULL;
        hr = vaDirectXCore::GetDevice( )->CreateHullShader( shaderBlob->GetBufferPointer( ), shaderBlob->GetBufferSize( ), NULL, &shader );
        assert( SUCCEEDED( hr ) );
        if( SUCCEEDED( hr ) )
        {
            V( shader->QueryInterface( IID_IUnknown, (void**)&m_shader ) );
            shader->Release( );
            vaDirectXCore::NameObject( shader, "vaDirectXHullShader shader object" );
        }

        SAFE_RELEASE( shaderBlob );
    }
    //
    void vaDirectXDomainShader::CreateShader( )
    {
        HRESULT hr;
        bool loadedFromCache;
        ID3DBlob * shaderBlob = CreateShaderBase( loadedFromCache );

        if( shaderBlob == NULL )
        {
            return;
        }
        m_lastLoadedFromCache = loadedFromCache;

        ID3D11DomainShader * shader = NULL;
        hr = vaDirectXCore::GetDevice( )->CreateDomainShader( shaderBlob->GetBufferPointer( ), shaderBlob->GetBufferSize( ), NULL, &shader );
        assert( SUCCEEDED( hr ) );
        if( SUCCEEDED( hr ) )
        {
            V( shader->QueryInterface( IID_IUnknown, (void**)&m_shader ) );
            shader->Release( );
            vaDirectXCore::NameObject( shader, "vaDirectXDomainShader shader object" );
        }

        SAFE_RELEASE( shaderBlob );
    }
    //
    void vaDirectXGeometryShader::CreateShader( )
    {
        HRESULT hr;
        bool loadedFromCache;
        ID3DBlob * shaderBlob = CreateShaderBase( loadedFromCache );

        if( shaderBlob == NULL )
        {
            return;
        }
        m_lastLoadedFromCache = loadedFromCache;

        ID3D11GeometryShader * shader = NULL;
        hr = vaDirectXCore::GetDevice( )->CreateGeometryShader( shaderBlob->GetBufferPointer( ), shaderBlob->GetBufferSize( ), NULL, &shader );
        assert( SUCCEEDED( hr ) );
        if( SUCCEEDED( hr ) )
        {
            V( shader->QueryInterface( IID_IUnknown, (void**)&m_shader ) );
            shader->Release( );
            vaDirectXCore::NameObject( shader, "vaDirectXGeometryShader shader object" );
        }

        SAFE_RELEASE( shaderBlob );
    }
    //
    void vaDirectXVertexShader::DestroyShader( )
    {
        vaDirectXShader::DestroyShader( );

        SAFE_RELEASE( m_inputLayout );
    }

    vaDirectXShaderManager::vaDirectXShaderManager( )
    {
        m_cacheLoaded = false;

        // this should be set externally, but good enough for now
        m_cacheFileDir = vaCore::GetExecutableDirectory( ) + L"cache\\";
    }

    vaDirectXShaderManager::~vaDirectXShaderManager( )
    {
#ifdef USE_SHADER_CACHE_SYSTEM
        SaveCache( GetCacheFileName().c_str() );
#endif
        ClearCache( );

        // Ensure no shaders remain
        assert( m_allShaderList.size( ) == 0 );
    }

    wstring vaDirectXShaderManager::GetCacheFileName( ) const
    {
#if defined( DEBUG ) || defined( _DEBUG )
        return L"shaders_debug_" + vaDirectXCore::GetInstance().GetAdapterNameID();
#else
        return L"shaders_release_" + vaDirectXCore::GetInstance( ).GetAdapterNameID( );
#endif
    }

    void vaDirectXShaderManager::RegisterShaderSearchPath( const std::wstring & path, bool pushBack )
    {
        wstring cleanedSearchPath = vaFileTools::CleanupPath( path + L"\\", false );
        if( pushBack )
            m_searchPaths.push_back( cleanedSearchPath );
        else
            m_searchPaths.push_front( cleanedSearchPath );
    }

    wstring vaDirectXShaderManager::FindShaderFile( const wchar_t * fileName )
    {
        for( unsigned int i = 0; i < m_searchPaths.size( ); i++ )
        {
            std::wstring filePath = m_searchPaths[i] + L"\\" + fileName;
            if( vaFileTools::FileExists( filePath.c_str( ) ) )
            {
                return vaFileTools::CleanupPath( filePath, false );
            }
            if( vaFileTools::FileExists( ( vaCore::GetWorkingDirectory( ) + filePath ).c_str( ) ) )
            {
                return vaFileTools::CleanupPath( vaCore::GetWorkingDirectory( ) + filePath, false );
            }
        }

        if( vaFileTools::FileExists( fileName ) )
            return vaFileTools::CleanupPath( fileName, false );

        if( vaFileTools::FileExists( ( vaCore::GetWorkingDirectory( ) + fileName ).c_str( ) ) )
            return vaFileTools::CleanupPath( vaCore::GetWorkingDirectory( ) + fileName, false );

        return L"";
    }

    void vaDirectXShaderManager::LoadCache( const wchar_t * filePath )
    {
        wstring cacheDir = m_cacheFileDir;

        m_cacheLoaded = true;

        wstring fullFileName = cacheDir + filePath;

        if( fullFileName == L"" )
        {
            return;
        }

        if( !vaFileTools::FileExists( fullFileName ) )
            return;

        ClearCache( );

        vaFileStream inFile;
        if( inFile.Open( fullFileName.c_str( ), FileCreationMode::Open, FileAccessMode::Read ) )
        {
            int version = -1;
            inFile.ReadValue<int32>( version );

            assert( version == 0 );

            int32 entryCount = 0;
            inFile.ReadValue<int32>( entryCount );

            for( int i = 0; i < entryCount; i++ )
            {
                vaShaderCacheKey key;
                key.Load( inFile );
                vaShaderCacheEntry * entry = new vaShaderCacheEntry( inFile );

                m_cache.insert( std::pair<vaShaderCacheKey, vaShaderCacheEntry *>( key, entry ) );
            }

            int32 terminator;
            inFile.ReadValue<int32>( terminator );
            assert( terminator == 0xFF );
        }
    }

    void vaDirectXShaderManager::SaveCache( const wchar_t * filePath ) const
    {
        wstring cacheDir = m_cacheFileDir;

        wstring fullFileName = cacheDir + filePath;

        vaFileTools::EnsureDirectoryExists( cacheDir.c_str( ) );

        vaFileStream outFile;
        outFile.Open( fullFileName.c_str( ), FileCreationMode::Create );

        outFile.WriteValue<int32>( 0 );                 // version;

        outFile.WriteValue<int32>( (int32)m_cache.size( ) );    // number of entries

        for( std::map<vaShaderCacheKey, vaShaderCacheEntry *>::const_iterator it = m_cache.cbegin( ); it != m_cache.cend( ); it++ )
        {
            // Save key
            ( *it ).first.Save( outFile );

            // Save data
            ( *it ).second->Save( outFile );
        }

        outFile.WriteValue<int32>( 0xFF );  // EOF;
    }

    void vaDirectXShaderManager::ClearCache( )
    {
        for( std::map<vaShaderCacheKey, vaShaderCacheEntry *>::iterator it = m_cache.begin( ); it != m_cache.end( ); it++ )
        {
            delete ( *it ).second;;
        }
        m_cache.clear( );
    }

    ID3DBlob * vaDirectXShaderManager::FindInCache( vaShaderCacheKey & key, bool & foundButModified )
    {
        foundButModified = false;

        if( !m_cacheLoaded )
        {
#ifdef USE_SHADER_CACHE_SYSTEM
            LoadCache( GetCacheFileName().c_str() );
#endif
        }

        std::map<vaShaderCacheKey, vaShaderCacheEntry *>::iterator it = m_cache.find( key );

        if( it != m_cache.end( ) )
        {
            if( ( *it ).second->IsModified( ) )
            {
                foundButModified = true;

                // Have to recompile...
                delete ( *it ).second;
                m_cache.erase( it );

                return NULL;
            }

            return ( *it ).second->GetCompiledShader( );
        }
        return NULL;
    }

    void vaDirectXShaderManager::AddToCache( vaShaderCacheKey & key, ID3DBlob * shaderBlob, std::vector<vaShaderCacheEntry::FileDependencyInfo> & dependencies )
    {
        std::map<vaShaderCacheKey, vaShaderCacheEntry *>::iterator it = m_cache.find( key );

        if( it != m_cache.end( ) )
        {
            // Already in?
            assert( false );
            delete ( *it ).second;
            m_cache.erase( it );
        }

        m_cache.insert( std::pair<vaShaderCacheKey, vaShaderCacheEntry *>( key, new vaShaderCacheEntry( shaderBlob, dependencies ) ) );
    }

    vaShaderCacheEntry::FileDependencyInfo::FileDependencyInfo( const wchar_t * filePath )
    {
        wstring fullFileName = vaDirectXShaderManager::GetInstance( ).FindShaderFile( filePath );

        if( fullFileName == L"" )
        {
            VA_ERROR( L"Error trying to find shader file '%s'!", filePath );

            assert( false );
            this->FilePath = L"";
            this->ModifiedTimeDate = 0;
        }
        else
        {
            this->FilePath = filePath;

            // struct _stat64 fileInfo;
            // _wstat64( fullFileName.c_str( ), &fileInfo ); // check error code?
            //this->ModifiedTimeDate = fileInfo.st_mtime;

            // maybe add some CRC64 here too? that would require reading contents of every file and every dependency which would be costly! 
            WIN32_FILE_ATTRIBUTE_DATA attrInfo;
            ::GetFileAttributesEx( fullFileName.c_str( ), GetFileExInfoStandard, &attrInfo );
            this->ModifiedTimeDate = (((int64)attrInfo.ftLastWriteTime.dwHighDateTime) << 32) | ((int64)attrInfo.ftLastWriteTime.dwLowDateTime);
        }
    }

    vaShaderCacheEntry::FileDependencyInfo::FileDependencyInfo( const wchar_t * filePath, int64 modifiedTimeDate )
    {
        this->FilePath = filePath;
        this->ModifiedTimeDate = modifiedTimeDate;
    }

    bool vaShaderCacheEntry::FileDependencyInfo::IsModified( )
    {
        wstring fullFileName = vaDirectXShaderManager::GetInstance( ).FindShaderFile( this->FilePath.c_str( ) );

        //vaLog::GetInstance( ).Add( LOG_COLORS_SHADERS, (L"vaShaderCacheEntry::FileDependencyInfo::IsModified, file name %s", fullFileName.c_str() ) );

        if( fullFileName == L"" )  // Can't find the file?
        {
            vaFileTools::EmbeddedFileData embeddedData = vaFileTools::EmbeddedFilesFind( wstring(L"shaders:\\") + this->FilePath );

            if( !embeddedData.HasContents( ) )
            {
                //vaLog::GetInstance( ).Add( LOG_COLORS_SHADERS, L"        > embedded data has NO contents" );
#ifdef STILL_LOAD_FROM_CACHE_IF_ORIGINAL_FILE_MISSING
                return false;
#else
                VA_ERROR( L"Error trying to find shader file '%s'!", this->FilePath.c_str( ) );
                return true;
#endif
            }
            else
            {
                //vaLog::GetInstance( ).Add( LOG_COLORS_SHADERS,  L"        > embedded data has contents" );
                return this->ModifiedTimeDate != embeddedData.TimeStamp;
            }
        }

        // struct _stat64 fileInfo;
        // _wstat64( fullFileName.c_str( ), &fileInfo ); // check error code?
        // return this->ModifiedTimeDate != fileInfo.st_mtime;

        // maybe add some CRC64 here too? that would require reading contents of every file and every dependency which would be costly! 
        WIN32_FILE_ATTRIBUTE_DATA attrInfo;
        ::GetFileAttributesEx( fullFileName.c_str( ), GetFileExInfoStandard, &attrInfo );
        bool ret = this->ModifiedTimeDate != (( ( (int64)attrInfo.ftLastWriteTime.dwHighDateTime ) << 32 ) | ( (int64)attrInfo.ftLastWriteTime.dwLowDateTime ));

        // if( ret )
        // {
        //     vaLog::GetInstance( ).Add( LOG_COLORS_SHADERS,  (const wchar_t*)((ret)?(L"   > shader file '%s' modified "):(L"   > shader file '%s' NOT modified ")), fullFileName.c_str() );
        // }

        return ret;
    }

    vaShaderCacheEntry::vaShaderCacheEntry( ID3DBlob * compiledShader, std::vector<vaShaderCacheEntry::FileDependencyInfo> & dependencies )
    {
        m_compiledShader = compiledShader;
        m_compiledShader->AddRef( );

        m_dependencies = dependencies;
    }
    vaShaderCacheEntry::~vaShaderCacheEntry( )
    {
        m_compiledShader->Release( );
    }

    bool vaShaderCacheEntry::IsModified( )
    {
        for( std::vector<vaShaderCacheEntry::FileDependencyInfo>::iterator it = m_dependencies.begin( ); it != m_dependencies.end( ); it++ )
        {
            if( ( *it ).IsModified( ) )
                return true;
        }
        return false;
    }

    void vaShaderCacheKey::Save( vaStream & outStream ) const
    {
        outStream.WriteString( this->StringPart.c_str( ) );
    }

    void vaShaderCacheEntry::FileDependencyInfo::Save( vaStream & outStream ) const
    {
        outStream.WriteString( this->FilePath.c_str( ) );
        outStream.WriteValue<int64>( this->ModifiedTimeDate );
    }

    void vaShaderCacheEntry::Save( vaStream & outStream ) const
    {
        outStream.WriteValue<int32>( ( int32 )this->m_dependencies.size( ) );       // number of dependencies

        for( std::vector<vaShaderCacheEntry::FileDependencyInfo>::const_iterator it = m_dependencies.cbegin( ); it != m_dependencies.cend( ); it++ )
        {
            ( *it ).Save( outStream );
        }

        int bufferSize = (int)m_compiledShader->GetBufferSize( );

        outStream.WriteValue<int32>( bufferSize );
        outStream.Write( m_compiledShader->GetBufferPointer( ), bufferSize );
    }

    void vaShaderCacheKey::Load( vaStream & inStream )
    {
        inStream.ReadString( this->StringPart );
    }

    void vaShaderCacheEntry::FileDependencyInfo::Load( vaStream & inStream )
    {
        inStream.ReadString( this->FilePath );
        inStream.ReadValue<int64>( this->ModifiedTimeDate );
    }

    void vaShaderCacheEntry::Load( vaStream & inStream )
    {
        int dependencyCount;
        inStream.ReadValue<int32>( dependencyCount );

        for( int i = 0; i < dependencyCount; i++ )
        {
            m_dependencies.push_back( vaShaderCacheEntry::FileDependencyInfo( inStream ) );
        }

        int bufferSize;
        inStream.ReadValue<int32>( bufferSize );

        D3DCreateBlob( bufferSize, &m_compiledShader );
        inStream.Read( m_compiledShader->GetBufferPointer( ), bufferSize );
    }

    void vaDirectXShaderManager::RecompileFileLoadedShaders( )
    {
        vaDirectXShader::ReloadAllShaders( );
    }

}