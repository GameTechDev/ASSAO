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

#include "vaCore.h"

#include "vaMemory.h"
#include "vaPlatformBase.h"
#include "vaStringTools.h"
#include "vaMath.h"
#include "vaLog.h"

#include "vaUIDObject.h"

#include "System\vaFileTools.h"

#include "System\vaThreadPool.h"

#include "Misc\vaTelemetryServer.h"
#include "Misc\vaBenchmarkTool.h"
#include "Misc/vaProfiler.h"

#include <assert.h>
#include <stdarg.h>

//#pragma warning (disable : 4995)
//#include <vector>
#include <algorithm>

// for GUIDs
#include <Objbase.h>
#pragma comment( lib, "Ole32.lib" )
#pragma comment( lib, "Rpcrt4.lib" )

using namespace VertexAsylum;
//using namespace std;

#include <omp.h>

vaCore::SubsystemManagerContainerType vaCore::s_subsystemManagersTickList;
vaCore::SubsystemManagerContainerType vaCore::s_subsystemManagersInitDeinitList;

bool vaCore::s_initialized = false;
bool vaCore::s_managersInitialized = false;

bool                vaCore::s_currentlyInitializing   = false;
bool                vaCore::s_currentlyDeinitializing = false;

int omp_thread_count( ) 
{
    int n = 0;
#pragma omp parallel reduction(+:n)
    n += 1;
    return n;
}
void vaCore::Initialize( )
{
    // Initializing more than once?
    assert( !s_initialized );

   vaMemory::Initialize( );

   new vaUIDObjectRegistrar( );

   vaMath::Initialize( );

   vaPlatformBase::Initialize( );

   vaFileTools::Initialize( );

   new vaLog( );

   new vaProfiler( );

//   InitializeSubsystemManagers( );
   // hmm not needed at the moment
   // new vaTelemetryServer();

   int logicalCoresToUse = vaThread::GetCPULogicalCoreCount();
   logicalCoresToUse = vaMath::Max( 2, logicalCoresToUse-1 );

   new vaThreadPool( logicalCoresToUse, vaThread::TP_Normal, 256*1024 );

   new vaBenchmarkTool( );

   s_initialized = true;
}

void vaCore::Deinitialize()
{
   assert( s_initialized );

   delete vaBenchmarkTool::GetInstancePtr();

   delete vaThreadPool::GetInstancePtr();

//   DeinitializeSubsystemManagers( );

   delete vaProfiler::GetInstancePtr();

   delete vaLog::GetInstancePtr( );

   vaFileTools::Deinitialize( );

   vaPlatformBase::Deinitialize( );

   vaMath::Deinitialize( );

   delete vaUIDObjectRegistrar::GetInstancePtr();

   vaMemory::Deinitialize( );

   s_initialized = false;
}

//void vaCore::Tick( float deltaTime )
//{
//   assert( s_initialized );
//
//   TickSubsytemManagers( deltaTime );
//}
//
//
//void vaCore::InitializeSubsystemManagers( )
//{
//   assert( !s_managersInitialized );
//
//   std::sort( s_subsystemManagersInitDeinitList.begin(), s_subsystemManagersInitDeinitList.end(), vaSystemManagerBase::InitOrderCompare );
//
//   s_currentlyInitializing = true;
//   
//   s_currentInitDeinitPass = IP_Early;
//   for( SubsystemManagerContainerType::const_iterator it = s_subsystemManagersInitDeinitList.begin(); it != s_subsystemManagersInitDeinitList.end(); it++ )
//      (*it)->OnInitialize( s_currentInitDeinitPass );
//   s_currentInitDeinitPass = IP_Regular;
//   for( SubsystemManagerContainerType::const_iterator it = s_subsystemManagersInitDeinitList.begin(); it != s_subsystemManagersInitDeinitList.end(); it++ )
//      (*it)->OnInitialize( s_currentInitDeinitPass );
//   s_currentInitDeinitPass = IP_Late;
//   for( SubsystemManagerContainerType::const_iterator it = s_subsystemManagersInitDeinitList.begin(); it != s_subsystemManagersInitDeinitList.end(); it++ )
//      (*it)->OnInitialize( s_currentInitDeinitPass );
//
//   s_currentlyInitializing = false;
//
//   s_managersInitialized = true;
//}
//
//void vaCore::DeinitializeSubsystemManagers( )
//{
//   assert( s_managersInitialized );
//
//   s_currentlyDeinitializing = true;
//
//   s_currentInitDeinitPass = IP_Early;
//   for( SubsystemManagerContainerType::const_reverse_iterator it = s_subsystemManagersInitDeinitList.rbegin(); it != s_subsystemManagersInitDeinitList.rend(); it++ )
//      (*it)->OnDeinitialize( s_currentInitDeinitPass );
//   s_currentInitDeinitPass = IP_Regular;
//   for( SubsystemManagerContainerType::const_reverse_iterator it = s_subsystemManagersInitDeinitList.rbegin(); it != s_subsystemManagersInitDeinitList.rend(); it++ )
//      (*it)->OnDeinitialize( s_currentInitDeinitPass );
//   s_currentInitDeinitPass = IP_Late;
//   for( SubsystemManagerContainerType::const_reverse_iterator it = s_subsystemManagersInitDeinitList.rbegin(); it != s_subsystemManagersInitDeinitList.rend(); it++ )
//      (*it)->OnDeinitialize( s_currentInitDeinitPass );
//
//   s_currentlyDeinitializing = false;
//
//   for( SubsystemManagerContainerType::reverse_iterator it = s_subsystemManagersInitDeinitList.rbegin(); it != s_subsystemManagersInitDeinitList.rend(); it++ )
//   {
//      if( (*it)->m_autoDelete )
//      {
//         delete (*it);
//         (*it) = NULL;
//      }
//   }
//   s_subsystemManagersInitDeinitList.clear();
//
//   s_managersInitialized = false;
//}
//
//void vaCore::TickSubsytemManagers( float deltaTime )
//{
//   for( SubsystemManagerContainerType::const_iterator it = s_subsystemManagersTickList.begin(); it != s_subsystemManagersTickList.end(); it++ )
//   {
//      (*it)->OnTick( deltaTime );
//   }
//}
//
//
//void vaCore::AddSubsystemManager( class vaSystemManagerBase * manager )
//{
//   // currently has to be added before initialization. we could handle this case as well in the future!
//   assert( !s_managersInitialized );
//   
//   assert( !s_currentlyInitializing );
//   assert( !s_currentlyDeinitializing );
//
//   s_subsystemManagersTickList.push_back( manager );
//   s_subsystemManagersInitDeinitList.push_back( manager );
//
//    std::sort( s_subsystemManagersTickList.begin(), s_subsystemManagersTickList.end(), vaSystemManagerBase::TickOrderCompare );
//}

void vaCore::DebugOutput( const wstring & message )
{
    vaPlatformBase::DebugOutput( message.c_str() );
}

void vaCore::Error( const wchar_t * messageFormat, ... )
{
   va_list args;
   va_start(args, messageFormat);
   std::wstring ret = vaStringTools::Format( messageFormat, args );
   va_end(args);
   vaPlatformBase::Error( ret.c_str() );
}

void vaCore::Warning( const wchar_t * messageFormat, ... )
{
   va_list args;
   va_start(args, messageFormat);
   std::wstring ret = vaStringTools::Format( messageFormat, args );
   va_end(args);
   vaPlatformBase::Warning( ret.c_str() );
}

bool vaCore::MessageBoxYesNo( const wchar_t * title, const wchar_t * messageFormat, ... )
{
   va_list args;
   va_start(args, messageFormat);
   std::wstring ret = vaStringTools::Format( messageFormat, args );
   va_end(args);
   return vaPlatformBase::MessageBoxYesNo( title, ret.c_str() );
}

vaGUID vaCore::GUIDCreate( )
{
   vaGUID ret;
   ::CoCreateGuid( &ret ); 
   return ret;
}

vaGUID vaCore::GUIDNull( )
{
    return GUID_NULL;
}

wstring vaCore::GUIDToString( const vaGUID & id )
{
   wstring ret;
   wchar_t * buffer;
   RPC_STATUS s = UuidToStringW( &id, (RPC_WSTR*)&buffer );
   VA_ASSERT( s == RPC_S_OK, L"GUIDToString failed" );
   return wstring( buffer );
}

vaGUID vaCore::GUIDFromString( const wstring & str )
{
   vaGUID ret;
   RPC_STATUS s = UuidFromStringW( (RPC_WSTR)str.c_str(), &ret );
   VA_ASSERT( s == RPC_S_OK, L"GUIDFromString failed" );
   return ret;
}

// int32 vaCore::GUIDGetHashCode( const vaGUID & id )
// {
//     //
// }

vaInputKeyboardBase *    vaInputKeyboardBase::s_current = NULL;
vaInputMouseBase *       vaInputMouseBase::s_current = NULL;
