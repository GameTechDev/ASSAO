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

#include "System\vaThread.h"
#include "System\vaCriticalSection.h"
#include "System\vaSocket.h"

#include <string>
#include <vector>

#define VA_ENABLE_TELEMETRY

namespace VertexAsylum
{

   struct vaTelemetryItem
   {
      enum DataType
      {
         DVT_Bool,
         DVT_Int32,
         DVT_UInt32,
         DVT_Float,
         DVT_MaxItems
      };

      DataType       Type;
      void *         pPtr;
      char           Name[128];
      union
      {
         int32       iMinVal;
         uint32      uMinVal;
         float       fMinVal;
      };
      union
      {
         int32       iMaxVal;
         uint32      uMaxVal;
         float		   fMaxVal;
      };

      bool           Editable;

      int64          SyncID;
      int64          SyncVersion;

   public:
                     vaTelemetryItem( DataType type, void * pPtr, const char * pName );

      void           ClientConnectedReset( int64 syncID );
   };

   /*
   class vaTelemetryServer : public vaSystemManagerSingletonBase<vaTelemetryServer>
   {
   public:
      static const int                    c_InitPriority = -10;
      static const int                    c_TickPriority = -10;

   protected:
      enum ServerToClientMessageID
      {
         SCM_ConnectedInfo          = 1,
         SCM_MainSyncPing           = 2,
         SCM_GlobalDataSyncPing     = 3,
         SCM_ItemUpdate             = 4,
      };

      enum ClientToServerMessageID
      {
         CSM_Disconnect             = 1,
         CSM_RequestItemSyncInfos   = 2,
      };

      // server->client
      struct MsgConnectedInfo
      {
         char              AppName[256];
      };

      // server->client
      struct MsgMainSyncPing
      {
         float             FPS;
         int64             GlobalSyncVersion;
      };

      // server->client
      struct MsgGlobalDataSyncPing
      {
         unsigned int	   NumberOfItems;
         //				... followed by NumberOfItems items:
         //              unsigned int	ItemID;
         //				unsigned int	ItemHash;
      };

      // server->client
      struct MsgItemUpdate
      {
         unsigned int	   ItemID;
         unsigned int	   DataSize;
         //				... followed by DataSize buffer
      };

   private:
      std::vector<vaTelemetryItem>
                           m_values;

      bool                 m_initialized;

      int64                m_NextItemSyncID;

      bool                 m_GlobalSyncVersionShouldChange;
      int64                m_GlobalSyncVersion;

      vaSocket *           m_pServerSocket;
      vaSocket *           m_pClientSocket;

      char                 m_ReceiveBuffer[ 1024 * 32 ];
      char                 m_SendBuffer[ 1024 * 32 + 4 ];

      bool                 m_ConnectedInfoMsgSent;
      double               m_TotalTime;
      float                m_TimeToNextDataPing;

      float                m_avgFrameRate;

   protected:
      friend class vaCore;
      vaTelemetryServer();

   public:
      virtual ~vaTelemetryServer();

   protected:
      // vaSystemManagerSingletonBase<vaTelemetryServer> implementation
      virtual void         OnInitialize( InitializationPass pass );
      virtual void         OnDeinitialize( InitializationPass pass );
      virtual void         OnTick( float deltaTime );

   public:
      void                 SetAvgFrameRate( float frameRate );

#ifdef VA_ENABLE_TELEMETRY
   public:
      void                 RegisterVariable( const char * name, int32 * pVal, int32 min = 0, int32 max = 0, bool editable = true );
      void                 RegisterVariable( const char * name, uint32 * pVal, uint32 min = 0, uint32 max = 0, bool editable = true );
      void                 RegisterVariable( const char * name, float * pVal, float min = 0.0f, float max = 0.0f, bool editable = true );
      void                 RegisterVariable( const char * name, bool * pVal, bool editable = true );
      void                 UnregisterVariable( const char * name );

   private:
      int                  FindVariableIndex( const char * name );

      bool                 IsClientConnected( )          { return m_pClientSocket != NULL; }
      void                 OnClientConnected( );

      void						SendMessage( ServerToClientMessageID MsgID, void * pBuffer, int size );
      void						OnMessageReceived( ClientToServerMessageID MsgID, void * pBuffer, int size );

#else
      public:
      void                
         ( const char * pName, int * pVal, int iMin, int iMax, int iStep = 1 )              { }
      void                 RegisterVariable( const char * pName, float * pVal, float fMin, float fMax, float fStep = 0.05f )  { }
      void                 UnregisterVariable( const char * name ) { }
      //vaTelemetryItem *  FindVariable( const char * name );
      void                 Update( float DeltaTime )                                                                          { }
#endif

   };
   */

}
   