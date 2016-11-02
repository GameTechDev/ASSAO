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
// 2016-09-07: filip.strugar@intel.com: first commit
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#define SSAODEMO_ENABLE_FINAL_INTEL_SSAO

#include "Rendering/Media/Shaders/vaShaderCore.h"

#include "Rendering/vaRendering.h"

#include "Rendering/vaTexture.h"

#include "IntegratedExternals/vaImguiIntegration.h"

#ifdef SSAODEMO_ENABLE_FINAL_INTEL_SSAO
#include "ASSAO/ASSAO.h"
#endif

namespace VertexAsylum
{
    class ASSAOWrapper : public vaRenderingModule, public vaImguiHierarchyObject
    {
        VA_RENDERING_MODULE_MAKE_FRIENDS( );
    private:

    protected:
#ifdef SSAODEMO_ENABLE_FINAL_INTEL_SSAO
        mutable ASSAO_Settings                  m_settings;
#endif

    protected:
        ASSAOWrapper( );
    public:
	    virtual ~ASSAOWrapper()                 { }

    public:

#ifdef SSAODEMO_ENABLE_FINAL_INTEL_SSAO
        ASSAO_Settings &                        GetSettings( )                                                  { return m_settings; }
#endif

        virtual void                                Draw( vaDrawContext & drawContext, vaTexture & depthTexture, bool blend, vaTexture * normalmapTexture = nullptr, const vaVector4i & scissorRect = vaVector4i( 0, 0, 0, 0 ) ) = 0;

    private:
#ifdef SSAODEMO_ENABLE_FINAL_INTEL_SSAO
        virtual string                              IHO_GetInstanceInfo( ) const { return "Intel ASSAO"; }
#else
        virtual string                              IHO_GetInstanceInfo( ) const { return "Intel ASSAO (disabled)"; }
#endif
        virtual void                                IHO_Draw( );

    };

} // namespace VertexAsylum

