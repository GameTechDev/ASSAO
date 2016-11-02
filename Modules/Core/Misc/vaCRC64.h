// Based on PostgreSQL code (only CRC64 part used), original license below:
// * ---------------------------------------------------------------------
// * Copyright (c) 2006-2015, Pittsburgh Supercomputing Center (PSC).
// * All rights reserved.
// *
// * Permission to use, copy, modify, and distribute this software for any
// * purpose with or without fee is hereby granted, provided that the
// * above copyright notice and this permission notice appear in all
// * copies.
// *
// * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
// * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
// * WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL THE
// * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
// * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
// * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
// * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// * PERFORMANCE OF THIS SOFTWARE.
// * --------------------------------------------------------------------

#pragma once

#include "../vaCore.h"

// also check https://github.com/QubesOS/qubes-vmm-xen-win-pvdrivers-xeniface/blob/master/src/xencontrol-test/crc64.c

namespace VertexAsylum
{
   class vaCRC64
   {
   private:
      static uint64        s_table[256]; 

      uint64               m_current;

   public:
      vaCRC64()                                             { m_current = 0xffffffffffffffff; }
      ~vaCRC64()                                            { }

      uint64               GetCurrent( ) const              { return m_current ^ ((uint64)0xffffffffffffffff); }

      operator             uint64( ) const                  { return GetCurrent(); }

      inline void          AddBytes( const void * dataPtr, int64 length )
      {
         const byte * data = (byte*)dataPtr;
         for( int64 i = 0; i < length; i++ )
         {
            uint8 tableIndex = ((uint8) (m_current >> 56) ^ data[i]) & 0xFF;
            m_current = s_table[tableIndex] ^ (m_current << 8);
         }
      }

      inline void          AddString( const string & str )
      {
         int length = (int)str.length();
         AddValue( length );
         if( length > 0 )  AddBytes( str.c_str(), length );
      }

      inline void          AddString( const wstring & str )
      {
         int length = (int)str.length() * 2;
         AddValue( length );
         if( length > 0 )  AddBytes( str.c_str(), length );
      }

      template< class ValueType >
      inline void          AddValue( ValueType val )
      {
         AddBytes( &val, sizeof(val) );
      }

      void                 XORAnother( uint64 val )
      {
         m_current ^= val;
      }

   private:

   };

}
