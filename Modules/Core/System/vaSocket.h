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

#include "Core/vaCore.h"

#include "System/vaPlatformSocket.h"


namespace VertexAsylum
{

   struct vaSocketAddress
   {
      // ipv6 in the future?
       unsigned char m_Address[128];
   };

   class vaSocket
   {
   protected:
      vaPlatformSocketType    m_socket;
      bool                    m_created;
      uint32                  m_maxConnections;

   protected:
      vaSocket();
      ~vaSocket();
   
   public:

      static vaSocket *       Create( bool typeTCP = true );
      static vaSocket *       Create( const vaPlatformSocketType & init );
      static void             Destroy( vaSocket * socket );

   public:
      bool                    Bind( uint16 port );
      bool                    Listen();
      bool                    Connect( const vaSocketAddress & serverAddress );
      vaSocket*               Accept();
      void                    Close()                                                                    ;
      int                     Receive( void *pBuffer, unsigned int size)                                 ;
      int                     ReceiveFrom( void *pBuffer, unsigned int size, vaSocketAddress & addr )    ;
      bool                    Send( void *pBuffer, unsigned int size )                                   ;
      bool                    SendTo( void *pBuffer, unsigned int Size, const vaSocketAddress & dest )   ;
      bool                    IsDataPending()                                                            ;
   };

}


