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

namespace VertexAsylum
{

    // A collection of post-processing helpers
    class vaPostProcess : public vaRenderingModule //, public vaImguiHierarchyObject
    {
    protected:
        vaPostProcess( );

    protected:
        std::shared_ptr<vaTexture>                  m_comparisonResultsGPU;
        std::shared_ptr<vaTexture>                  m_comparisonResultsCPU;

    public:
        virtual ~vaPostProcess( );

    private:
        friend class vaPostProcessDX11;

    public:
        // these are here as I've got no better place for them at the moment - maybe they should go into something like vaImageTools, or simply to vaTexture
        virtual void                                SaveTextureToDDSFile( vaDrawContext & drawContext, const wstring & path, vaTexture & texture ) = 0;
        virtual void                                SaveTextureToPNGFile( vaDrawContext & drawContext, const wstring & path, vaTexture & texture ) = 0;
        virtual void                                CopyResource( vaDrawContext & drawContext, vaTexture & destinationTexture, vaTexture & sourceTexture ) = 0;

        virtual void                                DrawTexturedQuad( vaDrawContext & drawContext, vaTexture & texture ) = 0;
        virtual void                                StretchRect( vaDrawContext & drawContext, vaTexture & texture, const vaVector4 & srcRect, const vaVector4 & dstRect, bool linearFilter ) = 0;

        virtual vaVector4                           CompareImages( vaDrawContext & drawContext, vaTexture & textureA, vaTexture & textureB )  = 0;


//        virtual void                                GenerateNormalmap( vaDrawContext & drawContext, vaTexture & depthTexture ) = 0;
//        virtual void                                DepthToViewspaceLinear( vaDrawContext & drawContext, vaTexture & depthTexture )   = 0;
    };

}
