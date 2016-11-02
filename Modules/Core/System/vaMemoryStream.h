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

#include "vaStream.h"

struct _iobuf;
typedef struct _iobuf FILE;

namespace VertexAsylum
{

   class vaMemoryStream : public vaStream
   {
      uint8 *                 m_buffer;
      int64                   m_bufferSize;
      int64                   m_pos;
      //
      bool                    m_autoBuffer;
      int64                   m_autoBufferCapacity;
      //
   public:
      vaMemoryStream( uint8 * buffer, int64 bufferSize );
      vaMemoryStream( int64 initialSize, int64 reserve = 64 );
      virtual ~vaMemoryStream( void );

      virtual bool            CanSeek( )                          { return true; }
      virtual void            Seek( int64 position )              { assert(position <= 2147483647 ); m_pos = (int32)position; }
      virtual void            Close( )                            { assert(false); }
      virtual bool            IsOpen( )                           { return m_buffer != NULL; }
      virtual int64           GetLength( )                        { return m_bufferSize; }
      virtual int64           GetPosition( )                      { return m_pos; }
      virtual void            Truncate( )                         { assert( false ); }

      virtual bool            Read( void * buffer, int64 count, int64 * outCountRead = NULL );
      virtual bool            Write( const void * buffer, int64 count, int64 * outCountWritten = NULL );

      uint8 *                 GetBuffer()                         { return m_buffer; }
      void                    Resize( int64 newSize );

   private:
      void                    Grow( int64 nextCapacity );
   };


}

