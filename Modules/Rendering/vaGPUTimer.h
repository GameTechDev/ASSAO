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

#include "Core/vaCoreIncludes.h"

#include "Rendering/vaRenderingIncludes.h"

#include <vector>

namespace VertexAsylum
{

    class vaGPUTimer : public vaRenderingModule
    {
        protected:
            string                                      m_name;

        protected:
            vaGPUTimer( )                               { }

        public:
            virtual ~vaGPUTimer( )                      { }

        public:
            void                                        SetName( const std::string & name ) { m_name = name; }
            const string &                              GetName( )                          { return m_name; }

        public:
            virtual void                                SetAPIContext( void * gpuAPIContext )   = 0;
            virtual void *                              GetAPIContext( )                        = 0;

            virtual void                                Start( ) = 0;
            virtual void                                Stop( ) = 0;

            virtual double                              GetLastTime( ) const = 0;
            virtual double                              GetAvgTime( ) const = 0;
    };

   
    class vaGPUTimer_AutoScopeProfile
    {
        vaGPUTimer &                        m_profiler;
        const bool                          m_doProfile;

    public:
        vaGPUTimer_AutoScopeProfile( vaGPUTimer & profiler, bool doProfile = true ) : m_profiler( profiler ), m_doProfile( doProfile )
        {
            if( m_doProfile ) m_profiler.Start( );
        }
        ~vaGPUTimer_AutoScopeProfile( )
        {
            if( m_doProfile ) m_profiler.Stop( );
        }
    };

}
