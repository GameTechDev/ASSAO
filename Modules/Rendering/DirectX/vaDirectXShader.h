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

#include "vaDirectXCore.h"

#include <string>
#include <vector>
#include <map>

#define LOG_COLORS_SHADERS  (vaVector4( 0.4f, 0.9f, 1.0f, 1.0f ) )

#ifdef _DEBUG
#define VA_HOLD_SHADER_DISASM
#endif

namespace VertexAsylum
{
    struct vaShaderCacheKey;
    class vaShaderCacheEntry;

    class vaDirectXShader : protected vaDirectXNotifyTarget
    {
        // struct MacroPair
        // {
        //     std::string      Name;
        //     std::string      Definition;
        //     MacroPair( ) {}
        //     MacroPair( const char * name, const char * definition ) : Name( name ), Definition( definition ) { }
        // };

    protected:
        IUnknown *                      m_shader;

        std::string                     m_entryPoint;
        std::string                     m_shaderModel;

        std::wstring                    m_shaderFilePath;
        std::string                     m_shaderCode;

#ifdef VA_HOLD_SHADER_DISASM
        std::string                     m_disasm;
        bool                            m_disasmAutoDumpToFile;
#endif

        typedef std::vector<std::pair<std::string, std::string>> MacroContaner;

        static const int                c_maxMacros = 128;
        MacroContaner                   m_macros;

        bool                            m_lastLoadedFromCache;

    public:
        vaDirectXShader( );
        virtual ~vaDirectXShader( );
        //
        void                            CreateShaderFromFile( const wchar_t * filePath, const char * shaderModel, const char * entryPoint, D3D_SHADER_MACRO * macros = NULL );
        void                            CreateShaderFromBuffer( const char * shaderCode, const char * shaderModel, const char * entryPoint, D3D_SHADER_MACRO * macros = NULL );
        //
        void                            CreateShaderFromFile( const wchar_t * filePath, const char * shaderModel, const char * entryPoint, const MacroContaner & macros );
        void                            CreateShaderFromBuffer( const char * shaderCode, const char * shaderModel, const char * entryPoint, const MacroContaner & macros );
        //
        void                            Clear( );
        //
        virtual void                    Reload( )                                       { DestroyShader( ); CreateShader( ); }
        virtual void                    Reload( D3D_SHADER_MACRO * newMacroDefines );
        virtual void                    Reload( const MacroContaner & macros );
        //
        virtual void                    SetToD3DContext( ID3D11DeviceContext * context ) = 0;
        //
        bool                            IsLoadedFromCache( ) const                      { return m_lastLoadedFromCache; }
        //
#ifdef VA_HOLD_SHADER_DISASM
        void                            SetShaderDisasmAutoDumpToFile( bool enable )    { m_disasmAutoDumpToFile = enable; }
        const string &                  GetShaderDisAsm( ) const                        { return m_disasm; }
#endif
        //
    protected:
        static std::vector<vaDirectXShader *> & GetAllShadersList( );
        //
        virtual void                    CreateCacheKey( vaShaderCacheKey & outKey );
        //
        void                            StoreMacros( const D3D_SHADER_MACRO * macroDefines );
        // returns NULL if macro count == 0
        const D3D_SHADER_MACRO *        GetStoredMacros( D3D_SHADER_MACRO * buffer, int bufferElementCount );
        //
    public:
        //
        static void                     ReloadAllShaders( );
        //
    protected:
        virtual void                    OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain );
        virtual void                    OnDeviceDestroyed( );
        //
        virtual void                    CreateShader( ) = 0;
        virtual void                    DestroyShader( )            { DestroyShaderBase( ); }
        void                            DestroyShaderBase( );
        //
        ID3DBlob *                      CreateShaderBase( bool & loadedFromCache );
        //
        virtual void                    AddBuiltInMacros( );
    };

    class vaDirectXPixelShader : public vaDirectXShader
    {
    public:
        vaDirectXPixelShader( )            {}
        virtual ~vaDirectXPixelShader( )   {}

    public:
        ID3D11PixelShader *        GetShader( );

        operator ID3D11PixelShader * ( ) { return GetShader( ); }

        virtual void               SetToD3DContext( ID3D11DeviceContext * context )
        {
            context->PSSetShader( GetShader( ), NULL, 0 );
        }

    protected:
        virtual void               CreateShader( );

    };

    class vaDirectXComputeShader : public vaDirectXShader
    {
    public:
        vaDirectXComputeShader( )            {}
        virtual ~vaDirectXComputeShader( )   {}

    public:
        ID3D11ComputeShader *      GetShader( );

        operator ID3D11ComputeShader * ( ) { return GetShader( ); }

        virtual void               SetToD3DContext( ID3D11DeviceContext * context )
        {
            context->CSSetShader( GetShader( ), NULL, 0 );
        }

    protected:
        virtual void               CreateShader( );

    };

    class vaDirectXVertexShader : public vaDirectXShader
    {
    protected:
        ID3D11InputLayout *        m_inputLayout;
        std::vector < D3D11_INPUT_ELEMENT_DESC >
            m_inputLayoutElements;

        void                       SetInputLayout( D3D11_INPUT_ELEMENT_DESC * elements, uint32 elementCount );

    public:
        vaDirectXVertexShader( );
        virtual ~vaDirectXVertexShader( );

    public:
        ID3D11VertexShader *       GetShader( );
        ID3D11InputLayout *        GetInputLayout( ) { return m_inputLayout; }

        operator ID3D11VertexShader * ( ) { return GetShader( ); }

        void                       CreateShaderAndILFromFile( const wchar_t * filePath, const char * shaderModel, const char * entryPoint, D3D_SHADER_MACRO * macros, D3D11_INPUT_ELEMENT_DESC * elements, uint32 elementCount );
        void                       CreateShaderAndILFromBuffer( const char * shaderCode, const char * shaderModel, const char * entryPoint, D3D_SHADER_MACRO * macros, D3D11_INPUT_ELEMENT_DESC * elements, uint32 elementCount );

        void                       CreateShaderAndILFromFile( const wchar_t * filePath, const char * shaderModel, const char * entryPoint, const MacroContaner & macros, D3D11_INPUT_ELEMENT_DESC * elements, uint32 elementCount );
        void                       CreateShaderAndILFromBuffer( const char * shaderCode, const char * shaderModel, const char * entryPoint, const MacroContaner & macros, D3D11_INPUT_ELEMENT_DESC * elements, uint32 elementCount );

    public:

        virtual void               SetToD3DContext( ID3D11DeviceContext * context )
        {
            if( m_inputLayout != NULL )
                context->IASetInputLayout( GetInputLayout( ) );
            context->VSSetShader( GetShader( ), NULL, 0 );
        }

    protected:
        virtual void               CreateShader( );
        virtual void               DestroyShader( );

        virtual void               CreateCacheKey( vaShaderCacheKey & outKey );
    };

    class vaDirectXHullShader : public vaDirectXShader
    {
    public:
        vaDirectXHullShader( )            {}
        virtual ~vaDirectXHullShader( )   {}

    public:
        ID3D11HullShader *         GetShader( );

        operator ID3D11HullShader * ( ) { return GetShader( ); }

        virtual void               SetToD3DContext( ID3D11DeviceContext * context )
        {
            context->HSSetShader( GetShader( ), NULL, 0 );
        }

    protected:
        virtual void               CreateShader( );

    };

    class vaDirectXDomainShader : public vaDirectXShader
    {
    public:
        vaDirectXDomainShader( )            {}
        virtual ~vaDirectXDomainShader( )   {}

    public:
        ID3D11DomainShader *       GetShader( );

        operator ID3D11DomainShader * ( ) { return GetShader( ); }

        virtual void               SetToD3DContext( ID3D11DeviceContext * context )
        {
            context->DSSetShader( GetShader( ), NULL, 0 );
        }

    protected:
        virtual void               CreateShader( );

    };

    class vaDirectXGeometryShader : public vaDirectXShader
    {
    public:
        vaDirectXGeometryShader( )            {}
        virtual ~vaDirectXGeometryShader( )   {}

    public:
        ID3D11GeometryShader *       GetShader( );

        operator ID3D11GeometryShader * ( ) { return GetShader( ); }

        virtual void               SetToD3DContext( ID3D11DeviceContext * context )
        {
            context->GSSetShader( GetShader( ), NULL, 0 );
        }

    protected:
        virtual void               CreateShader( );

    };



    struct vaShaderCacheKey
    {
        std::string                StringPart;

        bool                       operator == ( const vaShaderCacheKey & cmp ) const    { return this->StringPart == cmp.StringPart; }
        bool                       operator < ( const vaShaderCacheKey & cmp ) const     { return this->StringPart < cmp.StringPart; }
        bool                       operator >( const vaShaderCacheKey & cmp ) const     { return this->StringPart > cmp.StringPart; }

        void                       Save( vaStream & outStream ) const;
        void                       Load( vaStream & inStream );
    };

    class vaShaderCacheEntry
    {
    public:
        struct FileDependencyInfo
        {
            std::wstring            FilePath;
            int64                   ModifiedTimeDate;

            FileDependencyInfo( ) : FilePath( L"" ), ModifiedTimeDate( 0 )    { }
            FileDependencyInfo( const wchar_t * filePath );
            FileDependencyInfo( const wchar_t * filePath, int64 modifiedTimeDate );
            FileDependencyInfo( vaStream & inStream )                      { Load( inStream ); }

            bool                    IsModified( );

            void                    Save( vaStream & outStream ) const;
            void                    Load( vaStream & inStream );
        };

    private:

        ID3DBlob *                                               m_compiledShader;
        std::vector<vaShaderCacheEntry::FileDependencyInfo>      m_dependencies;

    private:
        vaShaderCacheEntry( ) {}
        vaShaderCacheEntry( const vaShaderCacheEntry & copy ) {}
    public:
        vaShaderCacheEntry( ID3DBlob * compiledShader, std::vector<vaShaderCacheEntry::FileDependencyInfo> & dependencies );
        vaShaderCacheEntry( vaStream & inFile )   { m_compiledShader = NULL; Load( inFile ); }
        ~vaShaderCacheEntry( );

        bool                       IsModified( );
        ID3DBlob *                 GetCompiledShader( )                       { m_compiledShader->AddRef( ); return m_compiledShader; }

        void                       Save( vaStream & outStream ) const;
        void                       Load( vaStream & inStream );
    };

    // Singleton utility class for handling shaders
    class vaDirectXShaderManager : public vaSingletonBase < vaDirectXShaderManager >
    {
    private:
        friend class vaShaderSharedManager;
        friend class vaMaterialManager;
        friend class vaDirectXShader;

    public:
        struct Settings
        {
            bool    WarningsAreErrors;

            Settings( )
            {
                WarningsAreErrors   = true;
            }
        };

    private:
        std::map<vaShaderCacheKey, vaShaderCacheEntry *>    m_cache;
        bool                                                m_cacheLoaded;

        std::deque<wstring>                                 m_searchPaths;

        wstring                                             m_cacheFileDir;

        std::vector<vaDirectXShader *>                      m_allShaderList;

        Settings                                            m_settings;

    public:
        vaDirectXShaderManager( );
        ~vaDirectXShaderManager( );

    public:
        ID3DBlob *          FindInCache( vaShaderCacheKey & key, bool & foundButModified );
        void                AddToCache( vaShaderCacheKey & key, ID3DBlob * shaderBlob, std::vector<vaShaderCacheEntry::FileDependencyInfo> & dependencies );
        void                ClearCache( );

        // pushBack (searched last) or pushFront (searched first)
        void                RegisterShaderSearchPath( const std::wstring & path, bool pushBack = true );

        wstring             FindShaderFile( const wchar_t * fileName );

        void                RecompileFileLoadedShaders( );

        Settings &          Settings( )                                             { return m_settings; }
        //const Settings &    Settings( ) const                                       { return m_settings; }

    private:
        wstring             GetCacheFileName( ) const;
        void                LoadCache( const wchar_t * filePath );
        void                SaveCache( const wchar_t * filePath ) const;

        std::vector<vaDirectXShader *> & GetAllShaderList( )        { return m_allShaderList; }
    };

}