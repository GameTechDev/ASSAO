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

#ifndef VA_SHADER_SHARED_TYPES_HLSL
#define VA_SHADER_SHARED_TYPES_HLSL

#include "vaShaderCore.h"

#include "vaSharedTypes_PostProcess.h"

#ifndef VA_COMPILED_AS_SHADER_CODE
namespace VertexAsylum
{
#endif

#define SHADERGLOBAL_CONSTANTSBUFFERSLOT            10

#define SHADERGLOBAL_POINTCLAMP_SAMPLERSLOT         10
#define SHADERGLOBAL_POINTWRAP_SAMPLERSLOT          11
#define SHADERGLOBAL_LINEARCLAMP_SAMPLERSLOT        12
#define SHADERGLOBAL_LINEARWRAP_SAMPLERSLOT         13
#define SHADERGLOBAL_ANISOTROPICCLAMP_SAMPLERSLOT   14
#define SHADERGLOBAL_ANISOTROPICWRAP_SAMPLERSLOT    15


#define SHADERGLOBAL_DEBUG_FLOAT_OUTPUT_COUNT       4096

#define SHADERSIMPLESHADOWSGLOBAL_CONSTANTSBUFFERSLOT       11
#define SHADERSIMPLESHADOWSGLOBAL_CMPSAMPLERSLOT            9
#define SHADERSIMPLESHADOWSGLOBAL_TEXTURESLOT               16
#define SHADERSIMPLESHADOWSGLOBAL_QUALITY                   2

#define POSTPROCESS_CONSTANTS_BUFFERSLOT                    1
#define RENDERMESHMATERIAL_CONSTANTS_BUFFERSLOT             1
#define RENDERMESH_CONSTANTS_BUFFERSLOT                     2
#define SIMPLEPARTICLESYSTEM_CONSTANTS_BUFFERSLOT           2
#define SIMPLESKY_CONSTANTS_BUFFERSLOT                      4
#define GBUFFER_CONSTANTS_BUFFERSLOT                        5


#define RENDERMESH_TEXTURE_SLOT0                          0
#define RENDERMESH_TEXTURE_SLOT1                          1
#define RENDERMESH_TEXTURE_SLOT2                          2
#define RENDERMESH_TEXTURE_SLOT3                          3
#define RENDERMESH_TEXTURE_SLOT4                          4
#define RENDERMESH_TEXTURE_SLOT5                          5

struct LightingGlobalConstants
{
    vaVector4               DirectionalLightWorldDirection;
    vaVector4               DirectionalLightViewspaceDirection;
    vaVector4               DirectionalLightIntensity;
    vaVector4               AmbientLightIntensity;

    vaVector4               FogColor;
    float                   FogDistanceMin;
    float                   FogDensity;
    float                   FogDummy0;
    float                   FogDummy1;
};

struct ShaderGlobalConstants
{
    LightingGlobalConstants Lighting;

    vaMatrix4x4             View;
    vaMatrix4x4             Proj;
    vaMatrix4x4             ViewProj;

    vaVector2               ViewportSize;           // ViewportSize.x, ViewportSize.y
    vaVector2               ViewportPixelSize;      // 1.0 / ViewportSize.x, 1.0 / ViewportSize.y

    vaVector2               ViewportHalfSize;         // ViewportSize.x * 0.5, ViewportSize.y * 0.5
    vaVector2               ViewportPixel2xSize;    // 2.0 / ViewportSize.x, 2.0 / ViewportSize.y

    vaVector2               DepthUnpackConsts;
    vaVector2               CameraTanHalfFOV;
    vaVector2               CameraNearFar;
    vaVector2               Dummy;

    float                   TransparencyPass;
    float                   WireframePass;
    float                   Dummy0;
    float                   Dummy1;
};

struct ShaderSimpleShadowsGlobalConstants
{
    vaMatrix4x4         View;
    vaMatrix4x4         Proj;
    vaMatrix4x4         ViewProj;

    vaMatrix4x4         CameraViewToShadowView;
    vaMatrix4x4         CameraViewToShadowViewProj;
    vaMatrix4x4         CameraViewToShadowUVNormalizedSpace;

    float               ShadowMapRes;
    float               OneOverShadowMapRes;
    float               Dummy2;
    float               Dummy3;
};

struct ShaderSimpleMeshPerInstanceConstants
{
    vaMatrix4x4         World;
    vaMatrix4x4         WorldView;
    vaMatrix4x4         ShadowWorldViewProj;
};

struct ShaderSimpleParticleSystemConstants
{
    vaMatrix4x4         World;
    vaMatrix4x4         WorldView;
    vaMatrix4x4         Proj;
};

struct SimpleSkyConstants
{
    vaMatrix4x4          ProjToWorld;

    vaVector4            SunDir;

    vaVector4            SkyColorLow;
    vaVector4            SkyColorHigh;

    vaVector4            SunColorPrimary;
    vaVector4            SunColorSecondary;

    float                SkyColorLowPow;
    float                SkyColorLowMul;

    float                SunColorPrimaryPow;
    float                SunColorPrimaryMul;
    float                SunColorSecondaryPow;
    float                SunColorSecondaryMul;

    float               Dummy0;
    float               Dummy1;
};

struct RenderMeshMaterialConstants
{
    vaVector4           ColorMultAlbedo;
    vaVector4           ColorMultSpecular;
};

struct RenderMeshConstants
{
    vaMatrix4x4         World;
    vaMatrix4x4         WorldView;
    vaMatrix4x4         ShadowWorldViewProj;
    vaVector4           Color;
};

struct GBufferConstants
{
    float               Dummy0;
    float               Dummy1;
    float               Dummy2;
    float               Dummy3;
};

#ifndef VA_COMPILED_AS_SHADER_CODE
} // namespace VertexAsylum
#endif

#ifdef VA_COMPILED_AS_SHADER_CODE


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Global buffers
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

cbuffer GlobalConstantsBuffer                       : register( B_CONCATENATER( SHADERGLOBAL_CONSTANTSBUFFERSLOT ) )
{
    ShaderGlobalConstants                   g_Global;
}

cbuffer ShaderSimpleShadowsGlobalConstantsBuffer    : register( B_CONCATENATER( SHADERSIMPLESHADOWSGLOBAL_CONSTANTSBUFFERSLOT ) )
{
    ShaderSimpleShadowsGlobalConstants      g_SimpleShadowsGlobal;
}

cbuffer SimpleSkyConstantsBuffer                    : register( B_CONCATENATER( SIMPLESKY_CONSTANTS_BUFFERSLOT ) )
{
    SimpleSkyConstants                      g_SimpleSkyGlobal;
}

cbuffer RenderMeshMaterialConstantsBuffer : register( B_CONCATENATER( RENDERMESHMATERIAL_CONSTANTS_BUFFERSLOT ) )
{
    RenderMeshMaterialConstants             g_RenderMeshMaterialGlobal;
}

cbuffer RenderMeshConstantsBuffer                   : register( B_CONCATENATER( RENDERMESH_CONSTANTS_BUFFERSLOT ) )
{
    RenderMeshConstants                     g_RenderMeshGlobal;
}

cbuffer ShaderSimpleParticleSystemConstantsBuffer   : register( B_CONCATENATER( SIMPLEPARTICLESYSTEM_CONSTANTS_BUFFERSLOT ) )
{
    ShaderSimpleParticleSystemConstants     g_ParticleSystemConstants;
}

cbuffer GBufferConstantsBuffer                      : register( B_CONCATENATER( GBUFFER_CONSTANTS_BUFFERSLOT ) )
{
    GBufferConstants                        g_GBufferConstants;
}



#endif

#endif // VA_SHADER_SHARED_TYPES_HLSL