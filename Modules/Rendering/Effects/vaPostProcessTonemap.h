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

#include "Rendering/vaRendering.h"

#include "Rendering/vaTexture.h"

#include "Rendering/Media/Shaders/vaSharedTypes.h"

//////////////////////////////////////////////////////////////////////////
// read:
//  - http://gpuopen.com/optimized-reversible-tonemapper-for-resolve/
//  - https://developer.nvidia.com/preparing-real-hdr
//  - https://developer.nvidia.com/implementing-hdr-rise-tomb-raider
//  - 
//////////////////////////////////////////////////////////////////////////

namespace VertexAsylum
{

    class vaPostProcessTonemap : public vaRenderingModule //, public vaImguiHierarchyObject
    {
    protected:
        vaPostProcessTonemap( );

    protected:
//        std::shared_ptr<vaTexture>                  m_comparisonResultsGPU;
//        std::shared_ptr<vaTexture>                  m_comparisonResultsCPU;

    public:
        virtual ~vaPostProcessTonemap( );

    public:
        virtual void                            Tonemap( vaDrawContext & drawContext, vaTexture & srcRadiance )             = 0;

    };

}
