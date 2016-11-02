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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project info
//
// VertexAsylum codebase was originally created by filip.strugar@intel.com for personal research & development use.
//
// It was intended to be an MIT-licensed platform for experimentation with DirectX, with rudimentary asset loading 
// through AssImp, simple rendering pipeline and support for at-runtime shader recompilation, basic post-processing, 
// basic GPU profiling and UI using Imgui, basic non-optimized vector math and other helpers.
//
// While the original codebase was designed in a platform independent way, the current state is that it only supports 
// DirectX 11 on Windows Desktop and HLSL shaders. So, it's not platform or API independent. I've abstracted a lot of 
// core stuff, and the graphics API parts are abstracted through the VA_RENDERING_MODULE_CREATE system, which once 
// handled Dx9 and OpenGL modules but, for now, just DX11 support exists.
//
// It is very much work in progress with future development based on short term needs, so feel free to use it any way 
// you like (and feel free to contribute back to it), but also use it at your own peril!
// 
// Filip Strugar, 14 October 2016
// 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// CRT's memory leak detection
#if defined(DEBUG) || defined(_DEBUG)

   #define _CRTDBG_MAP_ALLOC
   #define _CRTDBG_MAP_ALLOC_NEW
   #include <stdlib.h>
   #include <crtdbg.h>

#endif

#include "..\vaConfig.h"

#include "vaPlatformBase.h"

#include "vaCoreTypes.h"
#include "vaSTL.h"

// Frequently used includes
#include <assert.h>

// not platform independent yet - maybe use http://graemehill.ca/minimalist-cross-platform-uuid-guid-generation-in-c++/
#define __INLINE_ISEQUAL_GUID
#include <initguid.h>
#include <cguid.h>


namespace VertexAsylum
{
   class vaSystemManagerBase;

   //enum InitializationPass
   //{
   //   IP_Early = -10,
   //   IP_Regular = 0,
   //   IP_Late = 10,
   //};

   class vaThreadPool;
   
   // maybe make a real type that initializes to vaCore::GUIDNull()
   typedef GUID vaGUID;
   
   ////////////////////////////////////////////////////////////////////////////////////////////////
   // vaCore 
   class vaCore
   {
   private:
      typedef std::vector<vaSystemManagerBase*> SubsystemManagerContainerType;
      static SubsystemManagerContainerType       s_subsystemManagersTickList;
      static SubsystemManagerContainerType       s_subsystemManagersInitDeinitList;

      static bool                   s_initialized;
      static bool                   s_managersInitialized;

      static bool                   s_currentlyInitializing;
      static bool                   s_currentlyDeinitializing;

   public:

      // Initialize the system - must be called before any other calls
      static void                   Initialize( );
      
      // This must only be called from the same thread that called Initialize
      static void                   Deinitialize();

      // // This must only be called from the same thread that called Initialize
      // static void                Tick( float deltaTime );

      static void                   Error( const wchar_t * messageFormat, ... );
      static void                   Warning( const wchar_t * messageFormat, ... );
      static void                   DebugOutput( const wstring & message );

      static bool                   MessageBoxYesNo( const wchar_t * title, const wchar_t * messageFormat, ... );

      //static void                AddSubsystemManager( vaSystemManagerBase * manager );

      //static bool                IsInitializingManagers( )             { return s_currentlyInitializing; }
      //static bool                IsDeinitializingManagers( )           { return s_currentlyDeinitializing; }
      //static InitializationPass  GetCurrentManagersInitDeinitPass( )   { return s_currentInitDeinitPass; }

      static wstring                GetWorkingDirectory( );
      static wstring                GetExecutableDirectory( );

      static vaGUID                 GUIDCreate( );
      static vaGUID                 GUIDNull( );
      static wstring                GUIDToString( const vaGUID & id );
      static vaGUID                 GUIDFromString( const wstring & str );
//      static int32                  GUIDGetHashCode( const vaGUID & id );

      static string                 GetCPUIDName( );

   private:
      // // Initialize (& deinitialize) all added vaSystemManagerBase-derived classes
      // static void                InitializeSubsystemManagers( );
      // static void                DeinitializeSubsystemManagers( );
      // 
      // static void                TickSubsytemManagers( float deltaTime );
   };
   typedef vaCore    vaLevel0;
   ////////////////////////////////////////////////////////////////////////////////////////////////

   ////////////////////////////////////////////////////////////////////////////////////////////////
   // vaCoreInitDeinit - RAII for vaCore
   class vaCoreInitDeinit
   {
   public:
      vaCoreInitDeinit( ) { vaCore::Initialize( );    }
      ~vaCoreInitDeinit( ) { vaCore::Deinitialize( );  }
   };
   ////////////////////////////////////////////////////////////////////////////////////////////////


   struct vaGUIDComparer
   {
       bool operator()( const vaGUID & Left, const vaGUID & Right ) const
       {
           // comparison logic goes here
           return memcmp( &Left, &Right, sizeof( Right ) ) < 0;
       }
   };

   // Various defines

   #define VA__T(x)        L ## x
   #define VA_T(x)         VA__T(x)
   #define VA_NT(x)        L#x


   #ifdef _DEBUG
      #define VA_ASSERT_ALWAYS( format, ... )                           { vaCore::Warning( L"%s:%d\n" format , VA_T(__FILE__), __LINE__, __VA_ARGS__ ); assert( false ); } 
      #define VA_ASSERT( condition, format, ... )  if( !(condition) )   { VA_ASSERT_ALWAYS( format, __VA_ARGS__ ); } //{ vaCore::Warning( L"%s:%d\n" format , VA_T(__FILE__), __LINE__, __VA_ARGS__ ); assert( false ); } 
   #else
      #define VA_ASSERT_ALWAYS( format, ... )  { }
      #define VA_ASSERT( condition, format, ... )  { }
   #endif

   // recoverable error or a warning
    #define VA_WARN( format, ... )  { vaCore::Warning( L"%s:%d : " format , VA_T(__FILE__), __LINE__, __VA_ARGS__); } 

   // irrecoverable error, save the log and die gracefully
   #define VA_ERROR( format, ... )  { vaCore::Error( L"%s:%d : " format , VA_T(__FILE__), __LINE__, __VA_ARGS__); } 


   // Other stuff

   // warning C4201: nonstandard extension used : nameless struct/union
   #pragma warning( disable : 4201 )

   // warning C4239: nonstandard extension used : 'default argument' 
   #pragma warning( disable : 4239 )

#ifndef IsNullGUID
   inline bool IsNullGUID(REFGUID rguid)     { return IsEqualGUID( GUID_NULL, rguid ) != 0; }
#endif

   template< typename OutTypeName, typename InTypeName >
   inline OutTypeName vaSaferStaticCast( InTypeName ptr )
   {
#ifdef _DEBUG
       OutTypeName ret = dynamic_cast< OutTypeName >( ptr );
       assert( ret != NULL );
       return ret;
#else
       return static_cast < OutTypeName >( ptr );
#endif
   }

#if defined(DEBUG) || defined(_DEBUG)
#ifndef V
#define V(x)           { hr = (x); if( FAILED(hr) ) { assert(false); } } //DXUTTrace( __FILE__, (DWORD)__LINE__, hr, L#x, true ); } }
#endif
#ifndef V_RETURN
#define V_RETURN(x)    { hr = (x); if( FAILED(hr) ) { assert(false); } } //return DXUTTrace( __FILE__, (DWORD)__LINE__, hr, L#x, true ); } }
#endif
#else
#ifndef V
#define V(x)           { hr = (x); }
#endif
#ifndef V_RETURN
#define V_RETURN(x)    { hr = (x); if( FAILED(hr) ) { return hr; } }
#endif
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)          { if (p) { delete (p);     (p)=NULL; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p)    { if (p) { delete[] (p);   (p)=NULL; } }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)         { if (p) { (p)->Release(); (p)=NULL; } }
#endif
#ifndef SAFE_RELEASE_ARRAY
#define SAFE_RELEASE_ARRAY(p)   { for( int i = 0; i < _countof(p); i++ ) if (p[i]) { (p[i])->Release(); (p[i])=NULL; } }
#endif

#ifndef VERIFY_TRUE_RETURN_ON_FALSE
#define VERIFY_TRUE_RETURN_ON_FALSE( x )        \
if( !(x) )                                      \
{                                               \
    assert( false );                            \
    return false;                               \
}                                                                                          
#endif

}