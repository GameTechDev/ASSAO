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

#include "vaDirectXCore.h"

namespace VertexAsylum
{

   // Implementation for this is class is very... unstable (crap) - should be reworked.
   class vaDirectXFont
   {
   protected:
      uint32      m_color;
      uint32      m_posX;
      uint32      m_posY;
      uint32      m_lineHeight;

   public:
   	vaDirectXFont( int nLineHeight = 18 );
      ~vaDirectXFont();

       void    SetInsertionPos( int x, int y )  { m_posX = x; m_posY = y; }
       void    SetForegroundColor( uint32 clr ) { m_color = clr; }
       void    Begin();
       HRESULT DrawFormattedTextLine( const WCHAR* strMsg, ... );
       HRESULT DrawTextLine( const WCHAR* strMsg );
       HRESULT DrawFormattedTextLine( RECT& rc, DWORD dwFlags, const WCHAR* strMsg, ... );
       HRESULT DrawTextLine( RECT& rc, DWORD dwFlags, const WCHAR* strMsg );
       void    End();

   private:
       friend class vaRenderDeviceDX11;
       static void InitializeFontGlobals();
       static void DeinitializeFontGlobals();
   };

}


//////////////////////////////////////////////////////////////////////////
