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

#include "vaThread.h"

// this should probably be simply switched to use std::mutex for simplicity. or maybe leave it like this 
// because CRITICAL_SECTION is a bit faster than std::mutex, and use std::mutex in code where that is not
// relevant? 

namespace VertexAsylum
{

   class vaCriticalSection
   {
      vaPlatformCriticalSectionType          m_platformObject;

   private:
      vaCriticalSection( const vaCriticalSection & )   { assert( false ); } 

   public:
      inline vaCriticalSection();
      inline ~vaCriticalSection();

      inline void          Enter();
      inline bool          TryEnter();
      inline void          Leave();
   };

   class vaCriticalSectionScopeLock
   {
   private:
      vaCriticalSection &  m_cs;
      vaCriticalSectionScopeLock & operator=( vaCriticalSectionScopeLock & from ) { from; assert( false ); }
   public:
      vaCriticalSectionScopeLock( vaCriticalSection & cs ) : m_cs(cs) { m_cs.Enter(); }
      ~vaCriticalSectionScopeLock( ) { m_cs.Leave(); }
   };

   class vaCriticalSectionScopeTryLock
   {
   private:
       vaCriticalSection &  m_cs;
       bool                 m_hasLock;
       vaCriticalSectionScopeTryLock & operator=( vaCriticalSectionScopeTryLock & from ) { from; assert( false ); }
   public:
       vaCriticalSectionScopeTryLock( vaCriticalSection & cs ) : m_cs( cs ) { m_hasLock = m_cs.TryEnter( ); }
       ~vaCriticalSectionScopeTryLock( ) { if( m_hasLock ) m_cs.Leave( ); }
       
       bool                 HasLock( )  { return m_hasLock; }
   };

}


// Platform-specific implementation
#include "System\vaPlatformCriticalSection.inl"