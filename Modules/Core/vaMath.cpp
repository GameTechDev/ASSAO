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

#include "vaMath.h"
#include "vaCore.h"
#include "vaRandom.h"

#//include <stdarg.h>
#include <assert.h>

using namespace VertexAsylum;

vaRandom vaRandom::Singleton;

void vaMath::Initialize( )
{
}

void vaMath::Deinitialize( )
{

}

inline float vaMath::Randf( )
{
    // yeah yeah I need better random implementation... 
#if RAND_MAX == 0x7fff
    return ( ( rand( ) << 15 ) ^ rand( ) ) / (float)( ( RAND_MAX << 15 ) | RAND_MAX );
#elif RAND_MAX == 0x7fffffff
    return rand( ) / (float)RAND_MAX;
#else
    error RAND_MAX value not supported : just look at the code below and a version for your system
#endif
}