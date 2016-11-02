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

#include "Core/System/vaThread.h"


namespace VertexAsylum
{

   class vaSemaphore
   {
      vaPlatformSemaphoreType          m_semaphore;

   private:
      vaSemaphore( const vaSemaphore & )   { assert( false ); } 

   public:
      inline vaSemaphore( );
      inline vaSemaphore( const int32 initialCount, const int32 maxCount );
      inline ~vaSemaphore();

      inline void                               Initialize( const int32 initialCount, const int32 maxCount );
      inline bool                               IsInitialized( ) const;

      inline void                               Release( uint32 releaseCount = 1, uint32 * releasedCount = NULL );
      inline ThreadingWaitResponseType          Wait( uint32 waitTimeout = 0xFFFFFFFF );

      inline vaPlatformSemaphoreType            GetSyncObjectHandle( )  { return m_semaphore; }

      
      inline static ThreadingWaitResponseType   WaitAll( vaPlatformSemaphoreType semaphores[], const int semaphoresCount, uint32 waitTimeout = 0xFFFFFFFF );
      inline static ThreadingWaitResponseType   WaitAny( vaPlatformSemaphoreType semaphores[], const int semaphoresCount, uint32 waitTimeout = 0xFFFFFFFF );
   };

}

// Implementation
#include "System\vaPlatformSemaphore.inl"
