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

#include "System\vaPlatformSystemTimer.h"


namespace VertexAsylum
{

   class vaSystemTimer
   {
   public:

   protected:
      vaPlatformSystemTimerData              m_platformData;
      bool                                   m_isRunning;

   public:
      vaSystemTimer();
      ~vaSystemTimer();

   public:

      void                                   Start( );
      void                                   Stop( );
      bool                                   IsRunning( ) const         { return m_isRunning; }

      void                                   Tick( );

      // time elapsed (in seconds) from Start( ) to the last Tick( )
      double                                 GetTimeFromStart( ) const;

      // time elapsed (in seconds) between previous two Tick()s
      double                                 GetDeltaTime( ) const;

      double                                 GetCurrentTimeDouble( ) const;
   };
}

// Platform-specific implementation
#include "System\vaPlatformSystemTimer.inl"