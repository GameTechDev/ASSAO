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

// temporary include-all until I figure out what exactly to do with this...

#include <string>
#include <vector>
#include <deque>
#include <memory>
#include <algorithm>
#include <map>
#include <functional>

namespace VertexAsylum
{
    typedef std::string                 string;
    typedef std::wstring                wstring;

    template< class _Ty, class _Alloc = std::allocator<_Ty> >
    using vector = std::vector< _Ty, _Alloc>;

    template<class _Ty>
    using shared_ptr = std::shared_ptr< _Ty >;

    template<class _Ty>
    using unique_ptr = std::unique_ptr< _Ty >;

    template<class _Ty>
    using weak_ptr = std::weak_ptr< _Ty >;

    template<class _Ty1, class _Ty2>
    using pair = std::pair< _Ty1, _Ty2 >;

    template<class _Kty,
    class _Ty,
    class _Pr = std::less<_Kty>,
    class _Alloc = std::allocator<std::pair<const _Kty, _Ty> > >
    using map = std::map< _Kty, _Ty, _Pr, _Alloc >;


   //typedef
}