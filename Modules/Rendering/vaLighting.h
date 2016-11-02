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

#include "vaTexture.h"

//#include "Rendering/Media/Shaders/vaSharedTypes.h"

#include "IntegratedExternals/vaImguiIntegration.h"

namespace VertexAsylum
{
    class vaGBuffer;
    struct LightingGlobalConstants;

    // A helper for GBuffer rendering - does creation and updating of render target textures and provides debug views of them.
    class vaLighting : public vaRenderingModule, public vaImguiHierarchyObject
    {
    public:

    protected:
        string                                          m_debugInfo;

        vaVector3                                       m_directionalLightDirection;
        vaVector3                                       m_directionalLightIntensity;
        vaVector3                                       m_ambientLightIntensity;

        vaVector3                                       m_fogColor;
        float                                           m_fogDistanceMin;
        float                                           m_fogDensity;

    protected:
        vaLighting( );
    public:
        virtual ~vaLighting( );

    private:
        friend class vaLightingDX11;

    public:
        void                                            UpdateLightingGlobalConstants( vaDrawContext & drawContext, LightingGlobalConstants & consts );

    public:
        const vaVector3 &                               GetDirectionalLightDirection( )                                                     { return m_directionalLightDirection;   }
        const vaVector3 &                               GetDirectionalLightIntensity( )                                                     { return m_directionalLightIntensity;   }
        const vaVector3 &                               GetAmbientLightIntensity( )                                                         { return m_ambientLightIntensity;       }

        void                                            SetDirectionalLightDirection(  const vaVector3 & newValue )                         { m_directionalLightDirection = newValue; }
        void                                            SetDirectionalLightIntensity(  const vaVector3 & newValue )                         { m_directionalLightIntensity = newValue; }
        void                                            SetAmbientLightIntensity( const vaVector3 & newValue )                              { m_ambientLightIntensity     = newValue; }

        //void                                            GetFogParams
        void                                            SetFogParams( const vaVector3 & fogColor, float fogDistanceMin, float fogDensity )  { m_fogColor = fogColor; m_fogDistanceMin = fogDistanceMin; m_fogDensity = fogDensity; }

    public:
        virtual void                                    ApplyDirectionalAmbientLighting( vaDrawContext & drawContext, vaGBuffer & GBuffer ) = 0;

        virtual void                                    ApplyDynamicLighting( vaDrawContext & drawContext, vaGBuffer & GBuffer )            = 0;

        //virtual void                                    ApplyTonemap( vaDrawContext & drawContext, vaGBuffer & GBuffer )                    = 0;

    private:
        virtual string                                  IHO_GetInstanceInfo( ) const { return m_debugInfo; }
        virtual void                                    IHO_Draw( );

    };

}