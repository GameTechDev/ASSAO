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

#ifndef __VA_INTELSSAO_TYPES_H__
#define __VA_INTELSSAO_TYPES_H__
// #ifdef __INTELLISENSE__
// #undef INTEL_SSAO_INCLUDED_FROM_CPP
// #endif

#ifdef INTEL_SSAO_INCLUDED_FROM_CPP

namespace VertexAsylum
{

#else

#include "vaShaderCore.h"
// for definitions
#include "vaShared.hlsl"

#endif

#define SSAO_THREAD_GROUP_SIZE_X					8
#define SSAO_THREAD_GROUP_SIZE_Y					8

#define SSAO_SAMPLERS_SLOT0                         0
#define SSAO_SAMPLERS_SLOT1                         1
#define SSAO_SAMPLERS_SLOT2                         2
#define SSAO_SAMPLERS_SLOT3                         3

#define SSAO_NORMALMAP_OUT_UAV_SLOT                 4

#define SSAO_DEBUGGINGOUTPUT_OUT_UAV_SLOT           3

#define SSAO_CONSTANTS_BUFFERSLOT                   1

#define SSAO_TEXTURE_SLOT0                          0
#define SSAO_TEXTURE_SLOT1                          1
#define SSAO_TEXTURE_SLOT2                          2
#define SSAO_TEXTURE_SLOT3                          3
#define SSAO_TEXTURE_SLOT4                          4
#define SSAO_TEXTURE_SLOT5                          5

#define SSAO_MAX_TAPS                            32
#define SSAO_MAX_REF_TAPS                        512
#define SSAO_ADAPTIVE_TAP_BASE_COUNT             5
#define SSAO_ADAPTIVE_TAP_FLEXIBLE_COUNT         (SSAO_MAX_TAPS-SSAO_ADAPTIVE_TAP_BASE_COUNT)

#define SSAO_DEPTH_MIP_LEVELS                       4

#define SSAO_QUALITY_LEVEL_LOWEST                   0
#define SSAO_QUALITY_LEVEL_LOW                      1
#define SSAO_QUALITY_LEVEL_MEDIUM                   2
#define SSAO_QUALITY_LEVEL_HIGH                     3
#define SSAO_QUALITY_LEVEL_HIGHEST                  4
#define SSAO_QUALITY_LEVEL_REFERENCE                5
#define SSAO_QUALITY_LEVELS                         SSAO_QUALITY_LEVEL_REFERENCE + 1 // Lowest, Low, Medium, High, Highest, Reference

#define SSAO_MODE_PIXEL_SHADER                      0
#define SSAO_MODE_COMPUTE_SHADER                    1
#define SSAO_MODES                                  SSAO_MODE_COMPUTE_SHADER + 1

#define SSAO_ALLOW_INTERNAL_SHADER_DEBUGGING

// just for debugging
#ifdef SSAO_ALLOW_INTERNAL_SHADER_DEBUGGING
#define SSAO_DEBUGGING_DUMP_UAV_SLOT                2
#endif

#define SSAO_LOAD_COUNTER_UAV_SLOT                  4

// This lets the Low Quality preset use R8 instead of R8G8 texture for storing intermediate AO results: turns out there's little impact on most architectures (below 5%).
// There's added complexity to using this and a added requirement to re-create textures based on quality mode, so I'm keeping it out of the final code, but
// it's certainly an option especially if scratch memory size is of high importance on Low preset.
#define SSAO_USE_SEPARATE_LOWQ_AORESULTS_TEXTURE    0

struct ASSAOConstants
{
    vaVector2				ViewportSize;
    vaVector2               ViewportHalfSize;

    vaVector2               ViewportPixelSize;                      // .zw == 1.0 / ViewportSize.xy
    vaVector2               HalfViewportPixelSize;                  // .zw == 1.0 / ViewportHalfSize.xy

    vaVector2               DepthUnpackConsts;
    vaVector2               CameraTanHalfFOV;

    vaVector2               NDCToViewMul;
    vaVector2               NDCToViewAdd;

    vaVector2i              PerPassFullResCoordOffset;
    vaVector2               PerPassFullResUVOffset;

    vaVector2               Viewport2xPixelSize;
    vaVector2               Viewport2xPixelSize_x_025;              // Viewport2xPixelSize * 0.25 (for fusing add+mul into mad)

    float                   EffectRadius;                           // world (viewspace) maximum size of the shadow
    float                   EffectShadowStrength;                   // global strength of the effect (0 - 5)
    float                   EffectShadowPow;
    float                   EffectShadowClamp;

    float                   EffectFadeOutMul;                       // effect fade out from distance (ex. 25)
    float                   EffectFadeOutAdd;                       // effect fade out to distance   (ex. 100)
    float                   EffectHorizonAngleThreshold;            // limit errors on slopes and caused by insufficient geometry tessellation (0.05 to 0.5)
    float                   EffectSamplingRadiusNearLimitRec;       // if viewspace pixel closer than this, don't enlarge shadow sampling radius anymore (makes no sense to grow beyond some distance, not enough samples to cover everything, so just limit the shadow growth; could be SSAOSettingsFadeOutFrom * 0.1 or less)

    float                   DepthPrecisionOffsetMod;
    float                   NegRecEffectRadius;                     // -1.0 / EffectRadius
    float                   LoadCounterAvgDiv;                      // 1.0 / ( halfDepthMip[SSAO_DEPTH_MIP_LEVELS-1].sizeX * halfDepthMip[SSAO_DEPTH_MIP_LEVELS-1].sizeY )
    float                   AdaptiveSampleCountLimit;

    float                   InvSharpness;
    int                     PassIndex;
    vaVector2               QuarterResPixelSize;                    // used for importance map only

    vaVector4               PatternRotScaleMatrices[ 5 ];

    float                   UnusedNormalsUnpackMul;
    float                   UnusedNormalsUnpackAdd;
    float                   DetailAOStrength;
    int						QualityLevel;

    vaVector4               FullPatternRotScaleMatrices[ 20 ];

    // **********************************************************************************
    // ** stuff below not needed in production code **
    //
    // float                   MaxViewspaceDepth;
    // float                   MaxViewspaceDepthRec;
    // vaVector2               Dummy;
    //
    // for quality level 4 (reference) and similar - not needed in production code
    int                     AutoMatchSampleCount;
    float                   DebugRefSamplesDistribution;
    float                   Dummy1;
    float                   Dummy2;
    //
#ifdef SSAO_ALLOW_INTERNAL_SHADER_DEBUGGING
    vaVector2i              DebugCursorPos;
    vaVector2i              FullResOffset;
#endif
    //
    vaVector4               SamplesArray[ SSAO_MAX_REF_TAPS ];
    //
    // **********************************************************************************

};

#ifdef INTEL_SSAO_INCLUDED_FROM_CPP
}   // close the namespace
#endif


#endif // __VA_INTELSSAO_TYPES_H__