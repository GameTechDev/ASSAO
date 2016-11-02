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


namespace VertexAsylum
{
   ////////////////////////////////////////////////////////////////////////////////////////////////
   // vaMemory
   class vaMemory
   {
   private:
      friend class vaCore;

      static void						Initialize( );
      static void						Deinitialize( );

   public:

      // NOT THREAD SAFE AT THE MOMENT!!
      // (completely ad-hoc, needs work)
      static void *					    AllocTempBuffer( int size );
      static void						FreeTempBuffer( void * buffer );
   };

   // (completely ad-hoc, needs work)
   class vaTempMemoryBuffer
   {
   public:
      void *		Buffer;
      int			Size;

      vaTempMemoryBuffer( int size )		{ Buffer = vaMemory::AllocTempBuffer(size); Size = size; }
      ~vaTempMemoryBuffer( )					{ vaMemory::FreeTempBuffer( Buffer ); }
   };

   // Just a generic buffer class
   // this should be deleted and vaMemoryStream used instead
   class vaMemoryBuffer
   {
   private:
      byte *		m_data;
      int64			m_size;

   public:
      vaMemoryBuffer( int64 size )        { VA_ASSERT( size > 0, L"size parameter incorrect" ); m_data = new byte[(int)size]; m_size = (int)size; }
      ~vaMemoryBuffer( )                  { delete[] m_data; }

      byte *      GetData( ) const        { return m_data; }
      int64       GetSize( ) const        { return m_size; }
   };
}