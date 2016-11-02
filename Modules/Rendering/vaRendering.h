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

#include "Scene/vaCameraBase.h"

namespace VertexAsylum
{
    class vaRenderingModule;
    class vaRenderingModuleAPIImplementation;
    class vaRenderDeviceContext;
    class vaRenderingGlobals;
    class vaLighting;

    enum class vaRenderPassType : int32
    {
        Unknown                     = 0,
        DepthPrePass,
        GenerateShadowmap,
        GenerateVolumeShadowmap,
        Deferred,
        ForwardOpaque,
        ForwardTransparent,
        ForwardDebugWireframe,
    };

    struct vaDrawContext
    {
        vaDrawContext( vaCameraBase & camera, vaRenderDeviceContext & deviceContext, vaRenderingGlobals & globals, vaLighting * lighting = nullptr, class vaSimpleShadowMap * simpleShadowMap = nullptr, void * userContext = nullptr ) 
            : Camera( camera ), APIContext( deviceContext ), Globals( globals ), Lighting( lighting ), SimpleShadowMap( simpleShadowMap ), UserContext( userContext ), renderingGlobalsUpdated( false )
        {
            PassType        = vaRenderPassType::Unknown;
        }

        vaRenderDeviceContext &         APIContext;         // vaRenderDeviceContext is used to get/set current render targets and access rendering API stuff like contexts, etc.
        vaCameraBase &                  Camera;             // Currently selected camera
        vaRenderingGlobals &            Globals;            // Used to set global shader constants, track current frame index, provide some debugging tools, etc.
        vaLighting * const              Lighting;
        void * const                    UserContext;
    
        // can be changed at runtime
        vaRenderPassType                PassType;
        class vaSimpleShadowMap *       SimpleShadowMap;

    private:
        friend class vaRenderingGlobals;
        bool                            renderingGlobalsUpdated;
    
    public:
        bool                            GetRenderingGlobalsUpdated( ) const             { return renderingGlobalsUpdated; }
    };

    // base type for forwarding vaRenderingModule constructor parameters
    struct vaConstructorParamsBase
    {
        virtual ~vaConstructorParamsBase( ) { }
    };

    class vaRenderingModuleRegistrar : private vaSingletonBase < vaRenderingModuleRegistrar >
    {
    private:
        vaRenderingModuleRegistrar( )   { }
        ~vaRenderingModuleRegistrar( )  { }

        struct ModuleInfo
        {
            std::function< vaRenderingModule * ( const vaConstructorParamsBase * )> 
                                                ModuleCreateFunction;

            ModuleInfo( const std::function< vaRenderingModule * ( const vaConstructorParamsBase * )> & moduleCreateFunction )
                : ModuleCreateFunction( moduleCreateFunction ) { }
        };

        std::map< std::string, ModuleInfo >     m_modules;

    public:
        static void                             RegisterModule( const std::string & name, std::function< vaRenderingModule * ( const vaConstructorParamsBase * )> moduleCreateFunction );
        static vaRenderingModule *              CreateModule( const std::string & name, const vaConstructorParamsBase * params );
        //void                                  CreateModuleArray( int inCount, vaRenderingModule * outArray[] );

        template< typename ModuleType >
        static ModuleType *                     CreateModuleTyped( const std::string & name, const vaConstructorParamsBase * params );


    private:
        friend class vaRenderingCore;
        static void                             CreateSingletonIfNotCreated( );
        static void                             DeleteSingleton( );
    };

    class vaRenderingCore : public vaSingletonBase<vaRenderingCore>
    {
        std::deque<wstring>                     m_assetSearchPaths;

    private:
                                                vaRenderingCore( );
                                                ~vaRenderingCore( );

    public:
        // at the moment vaRenderingCore handles asset loading - if there's need in the future, this can be split into a separate class or even a separate module
        
        // pushBack (searched last) or pushFront (searched first)
        void                                    RegisterAssetSearchPath( const wstring & searchPath, bool pushBack = true );
        wstring                                 FindAssetFilePath( const wstring & assetFileName );


    public:
        // Initialize the system - must be called before any other calls to the module
        static void                             Initialize( );
        static void                             Deinitialize( );

        static void                             OnAPIInitialized( );
        static void                             OnAPIAboutToBeDeinitialized( );

        static bool                             IsInitialized( )                                            { return GetInstancePtr() != NULL; }

    private:
        static void                             InitializePlatform( );
    };

    // just a helper class for scope-based init/deinit
    class vaRenderingCoreInitDeinit
    {
    public:
        vaRenderingCoreInitDeinit( ) { vaRenderingCore::Initialize( ); }
        ~vaRenderingCoreInitDeinit( ) { vaRenderingCore::Deinitialize( ); }
    };

    class vaRenderingModule
    {
        string                              m_renderingModuleTypeName;

    protected:
        friend class vaRenderingModuleRegistrar;
        friend class vaRenderingModuleAPIImplementation;


    protected:
        vaRenderingModule( )                { assert( vaRenderingCore::IsInitialized( ) ); }
//                                            vaRenderingModule( const char * renderingCounterpartID );
    public:
        virtual                             ~vaRenderingModule( )                                                       { }

    private:
        // called only by vaRenderingModuleRegistrar::CreateModule
        void                                InternalRenderingModuleSetTypeName( const string & name )                   { m_renderingModuleTypeName = name; }

    public:
        const char *                        GetRenderingModuleTypeName( )                                               { return m_renderingModuleTypeName.c_str(); }

    public:
        template< typename CastToType >
        CastToType                          SafeCast( )                                                                 { return vaSaferStaticCast< CastToType  >( this ); }

    };

    class vaDrawableRenderingModule : public vaRenderingModule
    {
    public:
        virtual void                        Draw( vaDrawContext & drawContext )                                     = 0;
    };

    template< typename ModuleType, typename ModuleAPIImplementationType >
    class vaRenderingModuleAutoRegister
    {
    public:
        vaRenderingModuleAutoRegister( const char * name )
        {
            vaRenderingModuleRegistrar::RegisterModule( name, &CreateNew );
        }
        ~vaRenderingModuleAutoRegister( ) { }

    private:
        static inline vaRenderingModule *                   CreateNew( const vaConstructorParamsBase * params )
        { 
            return static_cast<vaRenderingModule*>( new ModuleAPIImplementationType( params ) ); 
        }
    };

    // to be placed at the end of the API counterpart module C++ file
    // example: 
    //      VA_RENDERING_MODULE_REGISTER( Tree, TreeDX11 );
#define VA_RENDERING_MODULE_REGISTER( ModuleTypeName, ModuleAPIImplementationTypeName )                                                    \
        vaRenderingModuleAutoRegister< ModuleTypeName, ModuleAPIImplementationTypeName > autoReg##ModuleTypeName_##ModuleAPIImplementationTypeName( #ModuleTypeName );     \

// A helper for implementing vaRenderingModule and vaRenderingModuleAPIImplementation 
#define     VA_RENDERING_MODULE_MAKE_FRIENDS( )                                                                               \
    template< typename ModuleType, typename ModuleAPIImplementationType > friend class vaRenderingModuleAutoRegister;       \
    friend class vaRenderingModuleRegistrar;                                                                                            

    // A helper used to create rendering module and its counterpart at runtime
    // use example: 
    //      Tree * newTree = VA_RENDERING_MODULE_CREATE( Tree );
#define VA_RENDERING_MODULE_CREATE( ModuleTypeName )        vaRenderingModuleRegistrar::CreateModuleTyped<ModuleTypeName>( #ModuleTypeName, nullptr )
#define VA_RENDERING_MODULE_CREATE_SHARED( ModuleTypeName ) std::shared_ptr< ModuleTypeName >( vaRenderingModuleRegistrar::CreateModuleTyped<ModuleTypeName>( #ModuleTypeName, nullptr ) )
#define VA_RENDERING_MODULE_CREATE_UNIQUE( ModuleTypeName ) std::unique_ptr< ModuleTypeName >( vaRenderingModuleRegistrar::CreateModuleTyped<ModuleTypeName>( #ModuleTypeName, nullptr ) )

#define VA_RENDERING_MODULE_CREATE_PARAMS( ModuleTypeName, Params )        vaRenderingModuleRegistrar::CreateModuleTyped<ModuleTypeName>( #ModuleTypeName, &Params )
#define VA_RENDERING_MODULE_CREATE_PARAMS_SHARED( ModuleTypeName, Params ) std::shared_ptr< ModuleTypeName >( vaRenderingModuleRegistrar::CreateModuleTyped<ModuleTypeName>( #ModuleTypeName, &Params ) )
#define VA_RENDERING_MODULE_CREATE_PARAMS_UNIQUE( ModuleTypeName, Params ) std::unique_ptr< ModuleTypeName >( vaRenderingModuleRegistrar::CreateModuleTyped<ModuleTypeName>( #ModuleTypeName, &Params ) )


    template< typename ModuleType >
    inline static ModuleType * vaRenderingModuleRegistrar::CreateModuleTyped( const std::string & name, const vaConstructorParamsBase * params )
    {
        ModuleType * ret = NULL;
        vaRenderingModule * createdModule = CreateModule( name, params );
#ifdef _DEBUG
        ret = dynamic_cast<ModuleType*>( createdModule );
#else
        ret = static_cast<ModuleType*>( createdModule );
#endif
        if( ret == NULL )
        {
            wstring wname = vaStringTools::SimpleWiden( name );
            VA_ERROR( L"vaRenderingModuleRegistrar::CreateModuleTyped failed for '%s'; have you done VA_RENDERING_MODULE_REGISTER( vaSomeClass, vaSomeClassDX11 )?; is vaSomeClass inheriting vaRenderingModule with 'public'? ", wname.c_str() );
        }

        return ret;
    }

    // this should go into a separate header (vaAsset.h?)
    struct vaAsset;
    class vaAssetResource : public std::enable_shared_from_this<vaAssetResource>, public vaUIDObject
    {
    private:
        const vaAsset *     m_parentAsset;
    
    protected:
        vaAssetResource( const vaGUID & uid ) : vaUIDObject( uid )          { m_parentAsset = nullptr; }
        virtual ~vaAssetResource( )                                         { assert( m_parentAsset == nullptr ); }
    
    public:
        const vaAsset *     GetParentAsset( )                               { return m_parentAsset; }

    private:
        friend struct vaAsset;
        void                SetParentAsset( const vaAsset * asset )         
        { 
            // there can be only one asset resource linked to one asset; this is one (of the) way to verify it:
            if( asset == nullptr )
            {
                assert( m_parentAsset != nullptr );
            }
            else
            {
                assert( m_parentAsset == nullptr );
            }
            m_parentAsset = asset; 
        }

    protected:
        virtual void        ReconnectDependencies( )                        { }
    };

}


#ifdef VA_SCOPE_PROFILER_ENABLED
    #define VA_SCOPE_CPUGPU_TIMER( name, apiContext )                                       vaScopeTimer scope_##name( #name, (apiContext).GetAPIImmediateContextPtr() )
    #define VA_SCOPE_CPUGPU_TIMER_DEFAULTSELECTED( name, apiContext )                       vaScopeTimer scope_##name( #name, (apiContext).GetAPIImmediateContextPtr(), true )
    #define VA_SCOPE_CPUGPU_TIMER_NAMED( nameVar, nameScope, apiContext )                   vaScopeTimer scope_##name( nameScope, (apiContext).GetAPIImmediateContextPtr() )
    #define VA_SCOPE_CPUGPU_TIMER_NAMED_DEFAULTSELECTED( nameVar, nameScope, apiContext )   vaScopeTimer scope_##name( nameScope, (apiContext).GetAPIImmediateContextPtr(), true )
#else
    #define VA_SCOPE_CPUGPU_TIMER( name, apiContext )                       
    #define VA_SCOPE_CPUGPU_TIMER_DEFAULTSELECTED( name, apiContext )                       
    #define VA_SCOPE_CPUGPU_TIMER_NAMED( nameVar, nameScope, apiContext )   
    #define VA_SCOPE_CPUGPU_TIMER_NAMED_DEFAULTSELECTED( nameVar, nameScope, apiContext )   
#endif