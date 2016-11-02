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


#include "../vaCoreIncludes.h"

// stari primer koriscenja:
//m_dbgDisplayDeferred = 0;
//vaTelemetryServer::GetInstance()->RegisterVariable( "Render.Deferred.DisplayBufferIndex", &m_dbgDisplayDeferred, false );
//vaTelemetryServer::GetInstance()->UnregisterVariable( "Render.Deferred.DisplayBufferIndex" );


using namespace VertexAsylum;
using namespace std;

static const uint64 c_headerMarker = (((uint64)'v') << 56) + (((uint64)'a') << 48) + (((uint64)'d') << 40) + (((uint64)'c') << 32) + (((uint64)'n') << 24) + (((uint64)'m') << 16) + (((uint64)'s') << 8) + (((uint64)'g') << 0);


#ifdef VA_ENABLE_TELEMETRY

vaTelemetryItem::vaTelemetryItem( DataType type, void * pPtr, const char * pName )
{
   this->Type        = type;
   int namel         = vaMath::Max( (int)_countof( this->Name ) - 1, (int)strlen( pName ) );
   VA_ASSERT( namel >= 0, L"" );
   memcpy( this->Name, pName, namel );	this->Name[namel] = 0;
   this->pPtr        = pPtr;
   VA_ASSERT( this->pPtr != NULL, L"" );
   switch( this->Type )
   {
   case( vaTelemetryItem::DVT_Bool ):     this->iMaxVal = this->iMinVal = 0; break;
   case( vaTelemetryItem::DVT_Int32 ):    this->iMaxVal = INT_MAX;   this->iMinVal = INT_MIN;   break;
   case( vaTelemetryItem::DVT_UInt32 ):   this->uMaxVal = UINT_MAX;  this->uMinVal = 0;         break;
   case( vaTelemetryItem::DVT_Float ):    this->fMaxVal = FLT_MAX;   this->fMinVal = -FLT_MAX;  break;
   }
   this->Editable    = true;

   this->SyncID      = -1;
   this->SyncVersion = -1;
}

void vaTelemetryItem::ClientConnectedReset( int64 syncID )
{
   this->SyncID      = syncID;
   this->SyncVersion = 0;
}

vaTelemetryServer::vaTelemetryServer()
   : vaSystemManagerSingletonBase<vaTelemetryServer>( c_InitPriority, c_TickPriority, true ),
   m_initialized( false )
{
   m_NextItemSyncID        = 0;
   m_TotalTime             = 0;

   //m_FpsFrameCount				= c_FpsUpdateFrequencyFrames+1;
   //m_FpsLast					= 0.0f;
   //m_FpsFrameAccumulatedTime	= 0.0f;
   m_pServerSocket			= NULL;
   m_pClientSocket			= NULL;

   m_TimeToNextDataPing		= 0.0f;
   m_ConnectedInfoMsgSent  = false;

   m_avgFrameRate          = 0.0f;
}

vaTelemetryServer::~vaTelemetryServer()
{
}

void vaTelemetryServer::OnInitialize( InitializationPass pass )
{
   if( pass != IP_Regular ) return;

   VA_ASSERT( !m_initialized, L"Already initialized?" );

   m_pServerSocket = vaSocket::Create( true );

   m_pServerSocket->Bind( 9970 );
   m_pServerSocket->Listen( );

   m_initialized = true;
}

void vaTelemetryServer::OnDeinitialize( InitializationPass pass )
{
   if( pass != IP_Regular ) return;

   if( m_pServerSocket != NULL )
   {
      vaSocket::Destroy( m_pServerSocket );
      m_pServerSocket = NULL;
   }

   if( m_pClientSocket != NULL )
   {
      vaSocket::Destroy( m_pClientSocket );
      m_pClientSocket = NULL;
   }
   m_GlobalSyncVersionShouldChange = true;
   m_initialized = false;
}

void vaTelemetryServer::SetAvgFrameRate( float frameRate )
{
   m_avgFrameRate = frameRate;
}

void vaTelemetryServer::RegisterVariable( const char * name, int32 * pVal, int32 iMin, int32 iMax, bool editable )
{
   VA_ASSERT( m_initialized, L"vaTelemetryServer not initialized" );
   VA_ASSERT( FindVariableIndex( name ) == -1, L"vaTelemetryServer::RegisterVariable - name '%s' already registered!", name );

   vaTelemetryItem dvi( vaTelemetryItem::DVT_Int32, pVal, name );
   dvi.iMinVal = iMin;
   dvi.iMaxVal = iMax;
   dvi.Editable = editable;
   m_values.push_back( dvi );

   if( IsClientConnected() )
   {
      m_values.back().ClientConnectedReset( m_NextItemSyncID );
      m_NextItemSyncID++;
   }
   m_GlobalSyncVersionShouldChange = true;
}

void vaTelemetryServer::RegisterVariable( const char * name, uint32 * pVal, uint32 min, uint32 max, bool editable )
{
   VA_ASSERT( m_initialized, L"vaTelemetryServer not initialized" );
   VA_ASSERT( FindVariableIndex( name ) == -1, L"vaTelemetryServer::RegisterVariable - name '%s' already registered!", name );

   vaTelemetryItem dvi( vaTelemetryItem::DVT_Int32, pVal, name );
   dvi.uMinVal = min;
   dvi.uMaxVal = max;
   dvi.Editable = editable;
   m_values.push_back( dvi );

   if( IsClientConnected() )
   {
      m_values.back().ClientConnectedReset( m_NextItemSyncID );
      m_NextItemSyncID++;
   }
   m_GlobalSyncVersionShouldChange = true;
}

void vaTelemetryServer::RegisterVariable( const char * name, float * pVal, float fMin, float fMax, bool editable )
{
   VA_ASSERT( m_initialized, L"vaTelemetryServer not initialized" );
   VA_ASSERT( FindVariableIndex( name ) == -1, L"vaTelemetryServer::RegisterVariable - name '%s' already registered!", name );

   vaTelemetryItem dvi( vaTelemetryItem::DVT_Float, pVal, name );
   dvi.fMinVal = fMin;
   dvi.fMaxVal = fMax;
   dvi.Editable = editable;
   m_values.push_back( dvi );

   if( IsClientConnected() )
   {
      m_values.back().ClientConnectedReset( m_NextItemSyncID );
      m_NextItemSyncID++;
   }
   m_GlobalSyncVersionShouldChange = true;
}

void vaTelemetryServer::RegisterVariable( const char * name, bool * pVal, bool editable )
{
   VA_ASSERT( m_initialized, L"vaTelemetryServer not initialized" );
   VA_ASSERT( FindVariableIndex( name ) == -1, L"vaTelemetryServer::RegisterVariable - name '%s' already registered!", name );

   vaTelemetryItem dvi( vaTelemetryItem::DVT_Bool, pVal, name );
   dvi.fMinVal = 0;
   dvi.fMaxVal = 0;
   dvi.Editable = editable;
   m_values.push_back( dvi );

   if( IsClientConnected() )
   {
      m_values.back().ClientConnectedReset( m_NextItemSyncID );
      m_NextItemSyncID++;
   }
   m_GlobalSyncVersionShouldChange = true;
}

void vaTelemetryServer::UnregisterVariable( const char * name )
{
   int index = FindVariableIndex( name );
   VA_ASSERT( index != -1, L"vaTelemetryServer::UnregisterVariable - name '%s' not registered?", name );

   if( index != -1 )
   {
      m_values.erase( m_values.begin() + index );
   }
   m_GlobalSyncVersionShouldChange = true;
}

int vaTelemetryServer::FindVariableIndex( const char * name )
{
   for( int i = 0; i < (int)m_values.size(); i++ )
   {
      if( strcmp( m_values[i].Name, name ) == 0)
         return i;
   }
   return -1;
}

void vaTelemetryServer::OnClientConnected( )
{
   m_NextItemSyncID                 = 0;
   m_GlobalSyncVersionShouldChange  = true;
   m_GlobalSyncVersion              = 0;

   for( int i = 0; i < (int)m_values.size(); i++ )
   {
      m_values[i].ClientConnectedReset( m_NextItemSyncID );
      m_NextItemSyncID++;
   }
}

void vaTelemetryServer::OnTick( float deltaTime )
{
   VA_ASSERT( m_initialized, L"vaTelemetryServer not initialized" );

   m_TotalTime += deltaTime;

   if( m_pServerSocket == NULL )
      return;

   m_TimeToNextDataPing -= deltaTime;
   if( m_TimeToNextDataPing <= 0.0f )
   {
      m_TimeToNextDataPing = 1.0f;

      if( m_pClientSocket != NULL )
      {
         if( m_GlobalSyncVersionShouldChange )
            m_GlobalSyncVersion++;

         MsgMainSyncPing msg;
         msg.GlobalSyncVersion   = m_GlobalSyncVersion;
         msg.FPS			         = m_avgFrameRate;

         SendMessage( SCM_MainSyncPing, &msg, sizeof(msg) );
      }
   }

   if( (m_pClientSocket == NULL) && m_pServerSocket->IsDataPending() )
   {
      // accept connection
      m_pClientSocket = m_pServerSocket->Accept( );
      
      OnClientConnected( );
      m_ConnectedInfoMsgSent = false;
   }

   if( m_pClientSocket != NULL )
   {
      if( !m_ConnectedInfoMsgSent )
      {
         m_ConnectedInfoMsgSent = true;

         MsgConnectedInfo msg;
         memset( &msg, 0, sizeof(msg) );

         strcpy_s( msg.AppName, sizeof(msg.AppName), "currently unnamed" );

         SendMessage( SCM_ConnectedInfo, &msg, sizeof(msg) );
      }

      if( m_pClientSocket->IsDataPending() )
      {
         int size = m_pClientSocket->Receive( m_ReceiveBuffer, _countof(m_ReceiveBuffer) );
         VA_ASSERT( (size <= 0) || (size >= 4), L"Socket receive error" );
         if( size >= 0 )
         {
            unsigned int msgid = *((unsigned int*)m_ReceiveBuffer);
            OnMessageReceived( (ClientToServerMessageID)msgid, (size==4)?(NULL):(m_ReceiveBuffer+4), size-4 );
         }
         if( size == -1 )
         {
            OnMessageReceived( CSM_Disconnect, NULL, 0 );
         }
      }
   }
}


void vaTelemetryServer::SendMessage( ServerToClientMessageID MsgID, void * pBuffer, int size )
{
   VA_ASSERT( m_initialized, L"vaTelemetryServer not initialized" );

   *((unsigned int*)m_SendBuffer) = MsgID;

   if( size > (_countof(m_SendBuffer)-4) )
   {
      VA_ASSERT_ALWAYS( L"vaTelemetryServer::SendMessage - msg size too big for the buffer" );
      return;
   }

   memcpy( m_SendBuffer + 4, pBuffer, size );
   m_pClientSocket->Send( m_SendBuffer, size + 4 );
}

void vaTelemetryServer::OnMessageReceived( ClientToServerMessageID MsgID, void * pBuffer, int size )
{
   VA_ASSERT( m_initialized, L"vaTelemetryServer not initialized" );
   pBuffer;

   if( MsgID == CSM_Disconnect )
   {
      if( m_pClientSocket != NULL )
      {
         vaSocket::Destroy( m_pClientSocket ); // (a bug in PCSocket.cpp ln 158?)
         m_pClientSocket = NULL;
      }
   }

   if( MsgID == CSM_RequestItemSyncInfos )
   {
      if( size == 0 )
      {
         //send number of items (int) 
         //send all item SyncIDs (int64) + SyncVersions (int64)
      }
      else
      {
         //send full items for listed requests, one by one per message
      }
   }
}

#endif // VA_ENABLE_TELEMETRY

