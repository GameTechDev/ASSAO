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

#include "vaLighting.h"
#include "Rendering/Media/Shaders/vaSharedTypes.h"

using namespace VertexAsylum;

vaLighting::vaLighting( )
{
    m_debugInfo = "Lighting (uninitialized - forgot to call RenderTick?)";

    m_directionalLightDirection = vaVector3( 0.0f, 0.0f, -1.0f );
    m_directionalLightIntensity = vaVector3( 1.1f, 1.1f, 1.1f );
    m_ambientLightIntensity     = vaVector3( 0.15f, 0.15f, 0.15f );

    m_fogColor                  = vaVector3( 0.4f, 0.4f, 0.9f );
    m_fogDistanceMin = 100.0f;
    m_fogDensity = 0.0007f;

}

vaLighting::~vaLighting( )
{

}

void vaLighting::UpdateLightingGlobalConstants( vaDrawContext & drawContext, LightingGlobalConstants & consts )
{
    vaMatrix4x4 mat = drawContext.Camera.GetViewMatrix( ) * drawContext.Camera.GetProjMatrix( );

    consts.DirectionalLightWorldDirection       = vaVector4( m_directionalLightDirection, 0.0f );
    consts.DirectionalLightViewspaceDirection   = vaVector4( vaVector3::TransformNormal( m_directionalLightDirection, drawContext.Camera.GetViewMatrix( ) ), 1.0f );

    consts.DirectionalLightIntensity            = vaVector4( m_directionalLightIntensity, 0.0f );
    consts.AmbientLightIntensity                = vaVector4( m_ambientLightIntensity, 0.0f );

    consts.FogColor                             = vaVector4( m_fogColor, 0.0f );
    consts.FogDistanceMin                       = m_fogDistanceMin;
    consts.FogDensity                           = m_fogDensity;
    consts.FogDummy0                            = 0.0f;
    consts.FogDummy1                            = 0.0f;
}

void vaLighting::IHO_Draw( )
{

}