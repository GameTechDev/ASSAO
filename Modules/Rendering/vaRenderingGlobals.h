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

#include "vaRendering.h"

#include "Rendering/Media/Shaders/vaSharedTypes.h"


namespace VertexAsylum
{
    class vaSky;
    class vaTexture;

    class vaRenderingGlobals : public VertexAsylum::vaRenderingModule
    {
        VA_RENDERING_MODULE_MAKE_FRIENDS( );

    public:
        static const int            c_shaderDebugFloatOutputCount = SHADERGLOBAL_DEBUG_FLOAT_OUTPUT_COUNT;
        static const int            c_shaderDebugOutputSyncDelay  = 4;

    private:

    protected:
        float                       m_shaderDebugFloats[ c_shaderDebugFloatOutputCount ];

        std::shared_ptr<vaTexture>  m_shaderDebugOutputGPUTextures[c_shaderDebugOutputSyncDelay];
        std::unique_ptr<vaTexture>  m_shaderDebugOutputCPUTextures[c_shaderDebugOutputSyncDelay];

        int64                       m_frameIndex;

    protected:
        vaRenderingGlobals( );

    public:
        ~vaRenderingGlobals( );

    public:
        // array size is c_shaderDebugFloatOutputCount
        const float *               GetShaderDebugFloatOutput( )                                                            { return m_shaderDebugFloats; }
        void                        GetShaderDebugFloatOutput( float const * & fltArray, int & fltArrayCount )              { fltArray = m_shaderDebugFloats; fltArrayCount = SHADERGLOBAL_DEBUG_FLOAT_OUTPUT_COUNT; }


    public:
        void                        Tick( float deltaTime );
        int64                       GetFrameIndex( ) const                                                                  { return m_frameIndex; }

    public:
        virtual void                SetAPIGlobals( vaDrawContext & drawContext )                                        = 0;
        
        // if calling more than once per frame make sure c_shaderDebugOutputSyncDelay is big enough to avoid stalls
        virtual void                UpdateDebugOutputFloats( vaDrawContext & drawContext )                              = 0;

    protected:
        void                        MarkAPIGlobalsUpdated( vaDrawContext & drawContext )                                    { drawContext.renderingGlobalsUpdated = true; }
    };

}
