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

#include "vaRenderingToolsDX11.h"

#include "Rendering/vaRenderingTools.h"

#include "Rendering/DirectX/vaDirectXIncludes.h"
#include "Rendering/DirectX/vaDirectXTools.h"

#include "Rendering/vaRenderingIncludes.h"

#include "Rendering/DirectX/vaTriangleMeshDX11.h"
#include "Rendering/DirectX/vaTextureDX11.h"
#include "Rendering/DirectX/vaGPUTimerDX11.h"

using namespace VertexAsylum;

void RegisterCanvasDX11();
void RegisterSimpleShadowMapDX11(  );
void RegisterSkyDX11( );
void RegisterASSAODX11( );
void RegisterSimpleParticleSystemDX11( );
void RegisterRenderingGlobals( );
void RegisterRenderMeshDX11( );
void RegisterRenderMaterialDX11( );
void RegisterGBufferDX11( );
void RegisterPostProcessDX11( );
void RegisterLightingDX11( );
void RegisterPostProcessTonemapDX11( );
void RegisterPostProcessBlurDX11( );

void vaRenderingCore::InitializePlatform( )
{
    VA_RENDERING_MODULE_REGISTER( vaTexture, vaTextureDX11 );
    VA_RENDERING_MODULE_REGISTER( vaTriangleMesh_PositionColorNormalTangentTexcoord1Vertex, vaTriangleMeshDX11_PositionColorNormalTangentTexcoord1Vertex );

    VA_RENDERING_MODULE_REGISTER( vaGPUTimer, vaGPUTimerDX11 );

    RegisterCanvasDX11( );
    RegisterSimpleShadowMapDX11( );
    RegisterSkyDX11( );
    RegisterASSAODX11( );
    RegisterSimpleParticleSystemDX11( );
    RegisterRenderingGlobals( );
    RegisterRenderMaterialDX11( );
    RegisterRenderMeshDX11( );
    RegisterGBufferDX11( );
    RegisterPostProcessDX11( );
    RegisterLightingDX11( );
    RegisterPostProcessTonemapDX11( );
    RegisterPostProcessBlurDX11( );
}
