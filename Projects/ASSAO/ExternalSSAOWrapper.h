#pragma once


#include "Rendering/Media/Shaders/vaShaderCore.h"

#include "Rendering/vaRendering.h"

#include "Rendering/vaTexture.h"

#include "IntegratedExternals/vaImguiIntegration.h"


namespace VertexAsylum
{
    class ExternalSSAOWrapper : public vaRenderingModule, public vaImguiHierarchyObject
    {
        VA_RENDERING_MODULE_MAKE_FRIENDS( );
    private:

    protected:
        ExternalSSAOWrapper( )                      { }
    public:
	    virtual ~ExternalSSAOWrapper()              { }

    public:

        virtual void                                Draw( vaDrawContext & drawContext, vaTexture & depthTexture, vaTexture * normalmapTexture, bool blend, const vaVector4i & scissorRect = vaVector4i( 0, 0, 0, 0 ) )    { };

    };

} // namespace VertexAsylum

