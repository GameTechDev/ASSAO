///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019, Intel Corporation
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
// 2019-01-16: rohit.shrinath.madayi.kolangarakath@intel.com, antoine.cohade@intel.com, john.g.gierach@intel.com:
//             compute shader path added!
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __INTELLISENSE__
#include "vaASSAO_types.h"
#endif

#ifdef SSAO_INTERNAL_SHADER_DEBUG
#include "vaSharedTypes.h"
RWTexture1D<float>              g_ShaderDebugOutputUAV              :   register( U_CONCATENATER( SSAO_DEBUGGING_DUMP_UAV_SLOT ) ); 
#endif

// progressive poisson-like pattern; x, y are in [-1, 1] range, .z is length( float2(x,y) ), .w is log2( z )
#include "vaASSAO_main_disk.h"

#if (INTELSSAO_MAIN_DISK_SAMPLE_COUNT) != (SSAO_MAX_TAPS)
    #error ASSAO : main disk number of samples mismatch!
#endif

// these values can be changed (up to SSAO_MAX_TAPS) with no changes required elsewhere; values for 4th and 5th preset are ignored but array needed to avoid compilation errors
static const lpuint g_numTaps[SSAO_QUALITY_LEVELS] = { 3, 3, 5, 11, 19, 0 }; // last one is for the reference super-quality version.

// an example of higher quality low/medium/high settings
// static const uint g_numTaps[5]   = { 4, 9, 16, 0, 0 };

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Optional parts that can be enabled for a required quality preset level and above (0 == Low, 1 == Medium, 2 == High, 3 == Highest/Adaptive, 4 == reference/unused )
// Each has its own cost. To disable just set to 5 or above.
//
// (experimental) tilts the disk (although only half of the samples!) towards surface normal; this helps with effect uniformity between objects but reduces effect distance and has other side-effects
#define SSAO_TILT_SAMPLES_ENABLE_AT_QUALITY_PRESET                      (99)        // to disable simply set to 99 or similar
#define SSAO_TILT_SAMPLES_AMOUNT                                        (0.4)
//
#define SSAO_HALOING_REDUCTION_ENABLE_AT_QUALITY_PRESET                 (SSAO_QUALITY_LEVEL_MEDIUM)         // to disable simply set to 99 or similar
#define SSAO_HALOING_REDUCTION_AMOUNT                                   (0.6)      // values from 0.0 - 1.0, 1.0 means max weighting (will cause artifacts, 0.8 is more reasonable)
//
#define SSAO_NORMAL_BASED_EDGES_ENABLE_AT_QUALITY_PRESET                (SSAO_QUALITY_LEVEL_HIGH)         // to disable simply set to 99 or similar
#define SSAO_NORMAL_BASED_EDGES_DOT_THRESHOLD                           (0.5)       // use 0-0.1 for super-sharp normal-based edges
//
#define SSAO_DETAIL_AO_ENABLE_AT_QUALITY_PRESET                         (SSAO_QUALITY_LEVEL_MEDIUM)         // whether to use DetailAOStrength; to disable simply set to 99 or similar
//
#define SSAO_DEPTH_MIPS_ENABLE_AT_QUALITY_PRESET                        (SSAO_QUALITY_LEVEL_HIGH)         // !!warning!! the MIP generation on the C++ side will be enabled on quality preset high regardless of this value, so if changing here, change the C++ side too
#define SSAO_DEPTH_MIPS_GLOBAL_OFFSET                                   (-4.3)      // best noise/quality/performance tradeoff, found empirically
//
// !!warning!! the edge handling is hard-coded to 'disabled' on quality level 0, and enabled above, on the C++ side; while toggling it here will work for 
// testing purposes, it will not yield performance gains (or correct results)
#define SSAO_DEPTH_BASED_EDGES_ENABLE_AT_QUALITY_PRESET                 (SSAO_QUALITY_LEVEL_MEDIUM)     
//
#define SSAO_REDUCE_RADIUS_NEAR_SCREEN_BORDER_ENABLE_AT_QUALITY_PRESET  (99)    // 99 simply means disabled; set to 1 to enable at "Medium" or 2 to enable at "High" preset
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_PASS_COUNT			4
cbuffer ASSAOConstantsBuffer : register(B_CONCATENATER( SSAO_CONSTANTS_BUFFERSLOT ) )
{
    ASSAOConstants g_ASSAOConsts;
}

SamplerState g_PointClampSampler : register(S_CONCATENATER( SSAO_SAMPLERS_SLOT0 ) );
SamplerState g_LinearClampSampler : register(S_CONCATENATER( SSAO_SAMPLERS_SLOT1 ) );
SamplerState g_PointMirrorSampler : register(S_CONCATENATER( SSAO_SAMPLERS_SLOT2 ) );
SamplerState g_ViewspaceDepthTapSampler : register(S_CONCATENATER( SSAO_SAMPLERS_SLOT3 ) );

Texture2D<float> g_DepthSource : register(T_CONCATENATER( SSAO_TEXTURE_SLOT0 ) );

Texture2D<float> g_ViewspaceDepthSource : register(T_CONCATENATER( SSAO_TEXTURE_SLOT0 ) );
Texture2D<float> g_ViewspaceDepthSource1 : register(T_CONCATENATER( SSAO_TEXTURE_SLOT1 ) );
Texture2D<float> g_ViewspaceDepthSource2 : register(T_CONCATENATER( SSAO_TEXTURE_SLOT2 ) );
Texture2D<float> g_ViewspaceDepthSource3 : register(T_CONCATENATER( SSAO_TEXTURE_SLOT3 ) );
Texture2D g_NormalmapSource : register(T_CONCATENATER( SSAO_TEXTURE_SLOT4 ) );

Texture2DArray<float>    g_ViewspaceDepthSourceArray2 : register(T_CONCATENATER(SSAO_TEXTURE_SLOT0));
Texture2D<float> g_ViewspaceDepthSourceArray[4] : register(T_CONCATENATER(SSAO_TEXTURE_SLOT0));

RWTexture2DArray<float> g_DepthOutputUAVArray : register(u0);

RWTexture2D<float> g_DepthOutputUAV0 : register(u0);
RWTexture2D<float> g_DepthOutputUAV1 : register(u1);
RWTexture2D<float> g_DepthOutputUAV2 : register(u2);
RWTexture2D<float> g_DepthOutputUAV3 : register(u3);

RWTexture2D<unorm float>	g_ImportanceMapOutputUAV	: register (u0);
Texture2D<float> g_ImportanceMap : register(T_CONCATENATER( SSAO_TEXTURE_SLOT3 ) );

Texture1D<uint> g_LoadCounter : register(T_CONCATENATER( SSAO_TEXTURE_SLOT2 ) );

Texture2D g_BlurInput : register(T_CONCATENATER( SSAO_TEXTURE_SLOT2 ) );
Texture2DArray g_BlurInputArray : register(T_CONCATENATER( SSAO_TEXTURE_SLOT2 ) );
RWTexture2DArray<unorm float2> g_OutputUAVArray : register (u0);
RWTexture2D<unorm float2> g_OutputUAV			: register (u1);

Texture2DArray g_FinalSSAO : register(T_CONCATENATER( SSAO_TEXTURE_SLOT5 ) );

RWTexture2D<unorm float4> g_NormalsOutputUAV    : register( U_CONCATENATER( SSAO_NORMALMAP_OUT_UAV_SLOT ) ); 
RWTexture1D<uint> g_LoadCounterOutputUAV : register(U_CONCATENATER( SSAO_LOAD_COUNTER_UAV_SLOT ) ); 

Texture2D g_DebuggingOutputSRV : register(T_CONCATENATER( SSAO_TEXTURE_SLOT5 ) );
RWTexture2D<unorm float4> g_DebuggingOutputUAV  : register( U_CONCATENATER( SSAO_DEBUGGINGOUTPUT_OUT_UAV_SLOT ) );

#define FOLD_CONSTANTS 0

#if FOLD_CONSTANTS
#define g_ViewportPixelSize float2(1.0f/1920.0f, 1.0f/1080.0f)
#define g_HalfViewportPixelSize float2(1.0f/960, 1.0f/540)
#define g_Viewport2xPixelSize float2(2.0f/1920.0f, 2.0f/1080.0f)
#define g_Viewport2xPixelSize_x_025 float2(1.0f/960, 1.0f/540)
#define g_ViewportHalfSize float2(960, 540)
#else 
#define g_ViewportPixelSize         g_ASSAOConsts.ViewportPixelSize
#define g_HalfViewportPixelSize     g_ASSAOConsts.HalfViewportPixelSize
#define g_Viewport2xPixelSize       g_ASSAOConsts.g_Viewport2xPixelSize
#define g_Viewport2xPixelSize_x_025 g_ASSAOConsts.Viewport2xPixelSize_x_025
#define g_ViewportHalfSize          g_ASSAOConsts.ViewportHalfSize
#endif

// packing/unpacking for edges; 2 bits per edge mean 4 gradient values (0, 0.33, 0.66, 1) for smoother transitions!
float PackEdges(float4 edgesLRTB)
{
//    int4 edgesLRTBi = int4( saturate( edgesLRTB ) * 3.0 + 0.5 );
//    return ( (edgesLRTBi.x << 6) + (edgesLRTBi.y << 4) + (edgesLRTBi.z << 2) + (edgesLRTBi.w << 0) ) / 255.0;

    // optimized, should be same as above
    edgesLRTB = round(saturate(edgesLRTB) * 3.05);
    return dot(edgesLRTB, float4(64.0 / 255.0, 16.0 / 255.0, 4.0 / 255.0, 1.0 / 255.0));
}

float4 UnpackEdges(float _packedVal)
{
    uint packedVal = (uint) (_packedVal * 255.5);
    float4 edgesLRTB;
    edgesLRTB.x = float((packedVal >> 6) & 0x03) / 3.0; // there's really no need for mask (as it's an 8 bit input) but I'll leave it in so it doesn't cause any trouble in the future
    edgesLRTB.y = float((packedVal >> 4) & 0x03) / 3.0;
    edgesLRTB.z = float((packedVal >> 2) & 0x03) / 3.0;
    edgesLRTB.w = float((packedVal >> 0) & 0x03) / 3.0;

    return saturate(edgesLRTB + g_ASSAOConsts.InvSharpness);
}

float ScreenSpaceToViewSpaceDepth(float screenDepth)
{
    float depthLinearizeMul = g_ASSAOConsts.DepthUnpackConsts.x;
    float depthLinearizeAdd = g_ASSAOConsts.DepthUnpackConsts.y;

    // Optimised version of "-cameraClipNear / (cameraClipFar - projDepth * (cameraClipFar - cameraClipNear)) * cameraClipFar"

    // Set your depthLinearizeMul and depthLinearizeAdd to:
    // depthLinearizeMul = ( cameraClipFar * cameraClipNear) / ( cameraClipFar - cameraClipNear );
    // depthLinearizeAdd = cameraClipFar / ( cameraClipFar - cameraClipNear );

    return depthLinearizeMul / (depthLinearizeAdd - screenDepth);
}

// from [0, width], [0, height] to [-1, 1], [-1, 1]
float2 ScreenSpaceToClipSpacePositionXY(float2 screenPos)
{
    return screenPos * g_ASSAOConsts.Viewport2xPixelSize.xy - float2(1.0f, 1.0f);
}

float3 ScreenSpaceToViewSpacePosition(float2 screenPos, float viewspaceDepth)
{
    return float3(g_ASSAOConsts.CameraTanHalfFOV.xy * viewspaceDepth * ScreenSpaceToClipSpacePositionXY(screenPos), viewspaceDepth);
}

float3 ClipSpaceToViewSpacePosition(float2 clipPos, float viewspaceDepth)
{
    return float3(g_ASSAOConsts.CameraTanHalfFOV.xy * viewspaceDepth * clipPos, viewspaceDepth);
}

float3 NDCToViewspace(float2 pos, float viewspaceDepth)
{
    float3 ret;

    ret.xy = (g_ASSAOConsts.NDCToViewMul * pos.xy + g_ASSAOConsts.NDCToViewAdd) * viewspaceDepth;

    ret.z = viewspaceDepth;

    return ret;
}

// calculate effect radius and fit our screen sampling pattern inside it
void CalculateRadiusParameters(const float pixCenterLength, const float2 pixelDirRBViewspaceSizeAtCenterZ, out float pixLookupRadiusMod, out float effectRadius, out float falloffCalcMulSq)
{
    effectRadius = g_ASSAOConsts.EffectRadius;

    // leaving this out for performance reasons: use something similar if radius needs to scale based on distance
    //effectRadius *= pow( pixCenterLength, g_ASSAOConsts.RadiusDistanceScalingFunctionPow);

    // when too close, on-screen sampling disk will grow beyond screen size; limit this to avoid closeup temporal artifacts
    const float tooCloseLimitMod = saturate(pixCenterLength * g_ASSAOConsts.EffectSamplingRadiusNearLimitRec) * 0.8 + 0.2;
    
    effectRadius *= tooCloseLimitMod;

    // 0.85 is to reduce the radius to allow for more samples on a slope to still stay within influence
    pixLookupRadiusMod = (0.85 * effectRadius) / pixelDirRBViewspaceSizeAtCenterZ.x;

    // used to calculate falloff (both for AO samples and per-sample weights)
    falloffCalcMulSq = -1.0f / (effectRadius * effectRadius);
}

float4 CalculateEdges(const float centerZ, const float leftZ, const float rightZ, const float topZ, const float bottomZ)
{
    // slope-sensitive depth-based edge detection
    float4 edgesLRTB = float4(leftZ, rightZ, topZ, bottomZ) - centerZ;
    float4 edgesLRTBSlopeAdjusted = edgesLRTB + edgesLRTB.yxwz;
    edgesLRTB = min(abs(edgesLRTB), abs(edgesLRTBSlopeAdjusted));
    return saturate((1.3 - edgesLRTB / (centerZ * 0.040)));

    // cheaper version but has artifacts
    // edgesLRTB = abs( float4( leftZ, rightZ, topZ, bottomZ ) - centerZ; );
    // return saturate( ( 1.3 - edgesLRTB / (pixZ * 0.06 + 0.1) ) );
}

// pass-through vertex shader
void VSMain(inout float4 Pos : SV_POSITION, inout float2 Uv : TEXCOORD0)
{
}

void PSPrepareDepths(in float4 inPos : SV_POSITION, out float out0 : SV_Target0, out float out1 : SV_Target1, out float out2 : SV_Target2, out float out3 : SV_Target3)
{
#if 0   // gather can be a bit faster but doesn't work with input depth buffers that don't match the working viewport
    float2 gatherUV = inPos.xy * g_ASSAOConsts.Viewport2xPixelSize;
    float4 depths = g_DepthSource.GatherRed( g_PointClampSampler, gatherUV );
    float a = depths.w;  // g_DepthSource.Load( int3( int2(inPos.xy) * 2, 0 ), int2( 0, 0 ) ).x;
    float b = depths.z;  // g_DepthSource.Load( int3( int2(inPos.xy) * 2, 0 ), int2( 1, 0 ) ).x;
    float c = depths.x;  // g_DepthSource.Load( int3( int2(inPos.xy) * 2, 0 ), int2( 0, 1 ) ).x;
    float d = depths.y;  // g_DepthSource.Load( int3( int2(inPos.xy) * 2, 0 ), int2( 1, 1 ) ).x;
#else
    int3 baseCoord = int3(int2(inPos.xy) * 2, 0);
    float a = g_DepthSource.Load(baseCoord, int2(0, 0)).x;
    float b = g_DepthSource.Load(baseCoord, int2(1, 0)).x;
    float c = g_DepthSource.Load(baseCoord, int2(0, 1)).x;
    float d = g_DepthSource.Load(baseCoord, int2(1, 1)).x;
#endif

    out0 = ScreenSpaceToViewSpaceDepth(a);
    out1 = ScreenSpaceToViewSpaceDepth(b);
    out2 = ScreenSpaceToViewSpaceDepth(c);
    out3 = ScreenSpaceToViewSpaceDepth(d);
}

void PSPrepareDepthsHalf(in float4 inPos : SV_POSITION, out float out0 : SV_Target0, out float out1 : SV_Target1)
{
    int3 baseCoord = int3(int2(inPos.xy) * 2, 0);
    float a = g_DepthSource.Load(baseCoord, int2(0, 0)).x;
    float d = g_DepthSource.Load(baseCoord, int2(1, 1)).x;

    out0 = ScreenSpaceToViewSpaceDepth(a);
    out1 = ScreenSpaceToViewSpaceDepth(d);
}


float3 CalculateNormal(const float4 edgesLRTB, float3 pixCenterPos, float3 pixLPos, float3 pixRPos, float3 pixTPos, float3 pixBPos)
{
    // Get this pixel's viewspace normal
    float4 acceptedNormals = float4(edgesLRTB.x * edgesLRTB.z, edgesLRTB.z * edgesLRTB.y, edgesLRTB.y * edgesLRTB.w, edgesLRTB.w * edgesLRTB.x);

    pixLPos = normalize(pixLPos - pixCenterPos);
    pixRPos = normalize(pixRPos - pixCenterPos);
    pixTPos = normalize(pixTPos - pixCenterPos);
    pixBPos = normalize(pixBPos - pixCenterPos);

    float3 pixelNormal = float3(0, 0, -0.0005);
    pixelNormal += (acceptedNormals.x) * cross(pixLPos, pixTPos);
    pixelNormal += (acceptedNormals.y) * cross(pixTPos, pixRPos);
    pixelNormal += (acceptedNormals.z) * cross(pixRPos, pixBPos);
    pixelNormal += (acceptedNormals.w) * cross(pixBPos, pixLPos);
    pixelNormal = normalize(pixelNormal);
    
    return pixelNormal;
}

void PSPrepareDepthsAndNormals(in float4 inPos : SV_POSITION, out float out0 : SV_Target0, out float out1 : SV_Target1, out float out2 : SV_Target2, out float out3 : SV_Target3)
{
    int2 baseCoords = ((int2) inPos.xy) * 2;
    float2 upperLeftUV = (inPos.xy - 0.25) * g_ASSAOConsts.Viewport2xPixelSize;

#if 0   // gather can be a bit faster but doesn't work with input depth buffers that don't match the working viewport
    float2 gatherUV = inPos.xy * g_ASSAOConsts.Viewport2xPixelSize;
    float4 depths = g_DepthSource.GatherRed( g_PointClampSampler, gatherUV );
    out0 = ScreenSpaceToViewSpaceDepth( depths.w );
    out1 = ScreenSpaceToViewSpaceDepth( depths.z );
    out2 = ScreenSpaceToViewSpaceDepth( depths.x );
    out3 = ScreenSpaceToViewSpaceDepth( depths.y );
#else
    int3 baseCoord = int3(int2(inPos.xy) * 2, 0);
    out0 = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(0, 0)).x);
    out1 = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(1, 0)).x);
    out2 = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(0, 1)).x);
    out3 = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(1, 1)).x);
#endif

    float pixZs[4][4];

    // middle 4
    pixZs[1][1] = out0;
    pixZs[2][1] = out1;
    pixZs[1][2] = out2;
    pixZs[2][2] = out3;
    // left 2
    pixZs[0][1] = ScreenSpaceToViewSpaceDepth(g_DepthSource.SampleLevel(g_PointClampSampler, upperLeftUV, 0.0, int2(-1, 0)).x);
    pixZs[0][2] = ScreenSpaceToViewSpaceDepth(g_DepthSource.SampleLevel(g_PointClampSampler, upperLeftUV, 0.0, int2(-1, 1)).x);
    // right 2
    pixZs[3][1] = ScreenSpaceToViewSpaceDepth(g_DepthSource.SampleLevel(g_PointClampSampler, upperLeftUV, 0.0, int2(2, 0)).x);
    pixZs[3][2] = ScreenSpaceToViewSpaceDepth(g_DepthSource.SampleLevel(g_PointClampSampler, upperLeftUV, 0.0, int2(2, 1)).x);
    // top 2
    pixZs[1][0] = ScreenSpaceToViewSpaceDepth(g_DepthSource.SampleLevel(g_PointClampSampler, upperLeftUV, 0.0, int2(0, -1)).x);
    pixZs[2][0] = ScreenSpaceToViewSpaceDepth(g_DepthSource.SampleLevel(g_PointClampSampler, upperLeftUV, 0.0, int2(1, -1)).x);
    // bottom 2
    pixZs[1][3] = ScreenSpaceToViewSpaceDepth(g_DepthSource.SampleLevel(g_PointClampSampler, upperLeftUV, 0.0, int2(0, 2)).x);
    pixZs[2][3] = ScreenSpaceToViewSpaceDepth(g_DepthSource.SampleLevel(g_PointClampSampler, upperLeftUV, 0.0, int2(1, 2)).x);

    float4 edges0 = CalculateEdges(pixZs[1][1], pixZs[0][1], pixZs[2][1], pixZs[1][0], pixZs[1][2]);
    float4 edges1 = CalculateEdges(pixZs[2][1], pixZs[1][1], pixZs[3][1], pixZs[2][0], pixZs[2][2]);
    float4 edges2 = CalculateEdges(pixZs[1][2], pixZs[0][2], pixZs[2][2], pixZs[1][1], pixZs[1][3]);
    float4 edges3 = CalculateEdges(pixZs[2][2], pixZs[1][2], pixZs[3][2], pixZs[2][1], pixZs[2][3]);

    float3 pixPos[4][4];

    // there is probably a way to optimize the math below; however no approximation will work, has to be precise.

    // middle 4
    pixPos[1][1] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(0.0, 0.0), pixZs[1][1]);
    pixPos[2][1] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(1.0, 0.0), pixZs[2][1]);
    pixPos[1][2] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(0.0, 1.0), pixZs[1][2]);
    pixPos[2][2] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(1.0, 1.0), pixZs[2][2]);
    // left 2
    pixPos[0][1] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(-1.0, 0.0), pixZs[0][1]);
    pixPos[0][2] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(-1.0, 1.0), pixZs[0][2]);
    // right 2                                                                                     
    pixPos[3][1] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(2.0, 0.0), pixZs[3][1]);
    pixPos[3][2] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(2.0, 1.0), pixZs[3][2]);
    // top 2                                                                                       
    pixPos[1][0] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(0.0, -1.0), pixZs[1][0]);
    pixPos[2][0] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(1.0, -1.0), pixZs[2][0]);
    // bottom 2                                                                                    
    pixPos[1][3] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(0.0, 2.0), pixZs[1][3]);
    pixPos[2][3] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(1.0, 2.0), pixZs[2][3]);

    float3 norm0 = CalculateNormal(edges0, pixPos[1][1], pixPos[0][1], pixPos[2][1], pixPos[1][0], pixPos[1][2]);
    float3 norm1 = CalculateNormal(edges1, pixPos[2][1], pixPos[1][1], pixPos[3][1], pixPos[2][0], pixPos[2][2]);
    float3 norm2 = CalculateNormal(edges2, pixPos[1][2], pixPos[0][2], pixPos[2][2], pixPos[1][1], pixPos[1][3]);
    float3 norm3 = CalculateNormal(edges3, pixPos[2][2], pixPos[1][2], pixPos[3][2], pixPos[2][1], pixPos[2][3]);

    g_NormalsOutputUAV[baseCoords + int2(0, 0)] = float4(norm0 * 0.5 + 0.5, 0.0);
    g_NormalsOutputUAV[baseCoords + int2(1, 0)] = float4(norm1 * 0.5 + 0.5, 0.0);
    g_NormalsOutputUAV[baseCoords + int2(0, 1)] = float4(norm2 * 0.5 + 0.5, 0.0);
    g_NormalsOutputUAV[baseCoords + int2(1, 1)] = float4(norm3 * 0.5 + 0.5, 0.0);
}

void PSPrepareDepthsAndNormalsHalf(in float4 inPos : SV_POSITION, out float out0 : SV_Target0, out float out1 : SV_Target1)
{
    int2 baseCoords = ((int2) inPos.xy) * 2;
    float2 upperLeftUV = (inPos.xy - 0.25) * g_ASSAOConsts.Viewport2xPixelSize;

    int3 baseCoord = int3(int2(inPos.xy) * 2, 0);
    float z0 = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(0, 0)).x);
    float z1 = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(1, 0)).x);
    float z2 = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(0, 1)).x);
    float z3 = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(1, 1)).x);

    out0 = z0;
    out1 = z3;

    float pixZs[4][4];

    // middle 4
    pixZs[1][1] = z0;
    pixZs[2][1] = z1;
    pixZs[1][2] = z2;
    pixZs[2][2] = z3;
    // left 2
    pixZs[0][1] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(-1, 0)).x);
    pixZs[0][2] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(-1, 1)).x);
	// right 2	  																		
    pixZs[3][1] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(2, 0)).x);
    pixZs[3][2] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(2, 1)).x);
	// top 2
    pixZs[1][0] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(0, -1)).x);
    pixZs[2][0] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(1, -1)).x);
	// bottom 2	  																		
    pixZs[1][3] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(0, 2)).x);
    pixZs[2][3] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(1, 2)).x);

    float4 edges0 = CalculateEdges(pixZs[1][1], pixZs[0][1], pixZs[2][1], pixZs[1][0], pixZs[1][2]);
    float4 edges1 = CalculateEdges(pixZs[2][1], pixZs[1][1], pixZs[3][1], pixZs[2][0], pixZs[2][2]);
    float4 edges2 = CalculateEdges(pixZs[1][2], pixZs[0][2], pixZs[2][2], pixZs[1][1], pixZs[1][3]);
    float4 edges3 = CalculateEdges(pixZs[2][2], pixZs[1][2], pixZs[3][2], pixZs[2][1], pixZs[2][3]);

    float3 pixPos[4][4];

    // there is probably a way to optimize the math below; however no approximation will work, has to be precise.

    // middle 4
    pixPos[1][1] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(0.0, 0.0), pixZs[1][1]);
    pixPos[2][1] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(1.0, 0.0), pixZs[2][1]);
    pixPos[1][2] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(0.0, 1.0), pixZs[1][2]);
    pixPos[2][2] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(1.0, 1.0), pixZs[2][2]);
    // left 2
    pixPos[0][1] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(-1.0, 0.0), pixZs[0][1]);
    //pixPos[0][2] = NDCToViewspace( upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2( -1.0,  1.0), pixZs[0][2] );
    // right 2                                                                                     
    //pixPos[3][1] = NDCToViewspace( upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(  2.0,  0.0), pixZs[3][1] );
    pixPos[3][2] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(2.0, 1.0), pixZs[3][2]);
    // top 2                                                                                       
    pixPos[1][0] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(0.0, -1.0), pixZs[1][0]);
    //pixPos[2][0] = NDCToViewspace( upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2( 1.0, -1.0 ), pixZs[2][0] );
    // bottom 2                                                                                    
    //pixPos[1][3] = NDCToViewspace( upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2( 0.0,  2.0 ), pixZs[1][3] );
    pixPos[2][3] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(1.0, 2.0), pixZs[2][3]);

    float3 norm0 = CalculateNormal(edges0, pixPos[1][1], pixPos[0][1], pixPos[2][1], pixPos[1][0], pixPos[1][2]);
    float3 norm3 = CalculateNormal(edges3, pixPos[2][2], pixPos[1][2], pixPos[3][2], pixPos[2][1], pixPos[2][3]);

    g_NormalsOutputUAV[baseCoords + int2(0, 0)] = float4(norm0 * 0.5 + 0.5, 0.0);
    g_NormalsOutputUAV[baseCoords + int2(1, 1)] = float4(norm3 * 0.5 + 0.5, 0.0);
}

//This function should be replaced by wave intrinsics option when moving to SM6.0
void CSPrepareDepthMip(const float2 inPos /*, const float2 inUV*/, int mipLevel, out float outD0, out float outD1, out float outD2, out float outD3)
{
    int2 baseCoords = int2(inPos.xy) * 2;

    float4 depthsArr[4];
    float depthsOutArr[4];

    // how to Gather a specific mip level?
    depthsArr[0].x = g_ViewspaceDepthSourceArray2[int3(baseCoords  + int2( 0, 0 ), 0)].x ;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[0].y = g_ViewspaceDepthSourceArray2[int3(baseCoords  + int2( 1, 0 ), 0)].x ;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[0].z = g_ViewspaceDepthSourceArray2[int3(baseCoords  + int2( 0, 1 ), 0)].x ;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[0].w = g_ViewspaceDepthSourceArray2[int3(baseCoords  + int2( 1, 1 ), 0)].x ;// * g_ASSAOConsts.MaxViewspaceDepth;

    depthsArr[1].x = g_ViewspaceDepthSourceArray2[int3(baseCoords + int2( 0, 0 ), 1)].x;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[1].y = g_ViewspaceDepthSourceArray2[int3(baseCoords + int2( 1, 0 ), 1)].x;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[1].z = g_ViewspaceDepthSourceArray2[int3(baseCoords + int2( 0, 1 ), 1)].x;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[1].w = g_ViewspaceDepthSourceArray2[int3(baseCoords + int2( 1, 1 ), 1)].x;// * g_ASSAOConsts.MaxViewspaceDepth;
    
    depthsArr[2].x = g_ViewspaceDepthSourceArray2[int3(baseCoords + int2( 0, 0 ), 2)].x;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[2].y = g_ViewspaceDepthSourceArray2[int3(baseCoords + int2( 1, 0 ), 2)].x;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[2].z = g_ViewspaceDepthSourceArray2[int3(baseCoords + int2( 0, 1 ), 2)].x;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[2].w = g_ViewspaceDepthSourceArray2[int3(baseCoords + int2( 1, 1 ), 2)].x;// * g_ASSAOConsts.MaxViewspaceDepth;
    
    depthsArr[3].x = g_ViewspaceDepthSourceArray2[int3(baseCoords + int2( 0, 0 ), 3)].x;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[3].y = g_ViewspaceDepthSourceArray2[int3(baseCoords + int2( 1, 0 ), 3)].x;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[3].z = g_ViewspaceDepthSourceArray2[int3(baseCoords + int2( 0, 1 ), 3)].x;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[3].w = g_ViewspaceDepthSourceArray2[int3(baseCoords + int2( 1, 1 ), 3)].x;// * g_ASSAOConsts.MaxViewspaceDepth;

    const uint2 SVPosui = uint2(inPos.xy);
    const uint pseudoRandomA = (SVPosui.x) + 2 * (SVPosui.y);

    float dummyUnused1;
    float dummyUnused2;
    float falloffCalcMulSq, falloffCalcAdd;
 

    for (int i = 0; i < 4; i++)
    {
        float4 depths = depthsArr[i];

        float closest = min(min(depths.x, depths.y), min(depths.z, depths.w));

        CalculateRadiusParameters(abs(closest), 1.0, dummyUnused1, dummyUnused2, falloffCalcMulSq);

        float4 dists = depths - closest.xxxx;

        float4 weights = saturate(dists * dists * falloffCalcMulSq + 1.0);

        float smartAvg = dot(weights, depths) / dot(weights, float4(1.0, 1.0, 1.0, 1.0));

        const uint pseudoRandomIndex = (pseudoRandomA + i) % 4;

        depthsOutArr[i] = smartAvg;
    }

    outD0 = depthsOutArr[0];
    outD1 = depthsOutArr[1];
    outD2 = depthsOutArr[2];
    outD3 = depthsOutArr[3];
}

//This function should be replaced by wave intrinsics option when moving to SM6.0
void PrepareDepthMip(const float2 inPos /*, const float2 inUV*/, int mipLevel, out float outD0, out float outD1, out float outD2, out float outD3)
{
    int2 baseCoords = int2(inPos.xy) * 2;

    float4 depthsArr[4];
    float depthsOutArr[4];

    // how to Gather a specific mip level?
    depthsArr[0].x = g_ViewspaceDepthSource[baseCoords + int2(0, 0)].x; // * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[0].y = g_ViewspaceDepthSource[baseCoords + int2(1, 0)].x; // * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[0].z = g_ViewspaceDepthSource[baseCoords + int2(0, 1)].x; // * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[0].w = g_ViewspaceDepthSource[baseCoords + int2(1, 1)].x; // * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[1].x = g_ViewspaceDepthSource1[baseCoords + int2(0, 0)].x; // * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[1].y = g_ViewspaceDepthSource1[baseCoords + int2(1, 0)].x; // * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[1].z = g_ViewspaceDepthSource1[baseCoords + int2(0, 1)].x; // * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[1].w = g_ViewspaceDepthSource1[baseCoords + int2(1, 1)].x; // * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[2].x = g_ViewspaceDepthSource2[baseCoords + int2(0, 0)].x; // * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[2].y = g_ViewspaceDepthSource2[baseCoords + int2(1, 0)].x; // * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[2].z = g_ViewspaceDepthSource2[baseCoords + int2(0, 1)].x; // * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[2].w = g_ViewspaceDepthSource2[baseCoords + int2(1, 1)].x; // * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[3].x = g_ViewspaceDepthSource3[baseCoords + int2(0, 0)].x; // * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[3].y = g_ViewspaceDepthSource3[baseCoords + int2(1, 0)].x; // * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[3].z = g_ViewspaceDepthSource3[baseCoords + int2(0, 1)].x; // * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[3].w = g_ViewspaceDepthSource3[baseCoords + int2(1, 1)].x; // * g_ASSAOConsts.MaxViewspaceDepth;

    const uint2 SVPosui = uint2(inPos.xy);
    const uint pseudoRandomA = (SVPosui.x) + 2 * (SVPosui.y);

    float dummyUnused1;
    float dummyUnused2;
    float falloffCalcMulSq, falloffCalcAdd;
 
    [unroll]
    for( int i = 0; i < 4; i++ )
    {
        float4 depths = depthsArr[i];

        float closest = min(min(depths.x, depths.y), min(depths.z, depths.w));

        CalculateRadiusParameters(abs(closest), 1.0, dummyUnused1, dummyUnused2, falloffCalcMulSq);

        float4 dists = depths - closest.xxxx;

        float4 weights = saturate(dists * dists * falloffCalcMulSq + 1.0);

        float smartAvg = dot(weights, depths) / dot(weights, float4(1.0, 1.0, 1.0, 1.0));

        const uint pseudoRandomIndex = (pseudoRandomA + i) % 4;

        depthsOutArr[i] = smartAvg;
    }

    outD0 = depthsOutArr[0];
    outD1 = depthsOutArr[1];
    outD2 = depthsOutArr[2];
    outD3 = depthsOutArr[3];
}

void PSPrepareDepthMip1(in float4 inPos : SV_POSITION, out float out0 : SV_Target0, out float out1 : SV_Target1, out float out2 : SV_Target2, out float out3 : SV_Target3)
{
    PrepareDepthMip(inPos.xy, 1, out0, out1, out2, out3);
}

void PSPrepareDepthMip2(in float4 inPos : SV_POSITION, out float out0 : SV_Target0, out float out1 : SV_Target1, out float out2 : SV_Target2, out float out3 : SV_Target3)
{
    PrepareDepthMip(inPos.xy, 2, out0, out1, out2, out3);
}

void PSPrepareDepthMip3(in float4 inPos : SV_POSITION, out float out0 : SV_Target0, out float out1 : SV_Target1, out float out2 : SV_Target2, out float out3 : SV_Target3)
{
    PrepareDepthMip(inPos.xy, 3, out0, out1, out2, out3);
}


float3 DecodeNormal(float3 encodedNormal)
{
    float3 normal = encodedNormal * 2.0.xxx - 1.0.xxx;

    // normal = normalize( normal );    // normalize adds around 2.5% cost on High settings but makes little (PSNR 66.7) visual difference when normals are as in the sample (stored in R8G8B8A8_UNORM,
    //                                  // decoded in the shader), however it will likely be required if using different encoding/decoding or the inputs are not normalized, etc.
        
    return normal;
}

float3 LoadNormal(int2 pos)
{
    float3 encodedNormal = g_NormalmapSource.Load(int3(pos, 0)).xyz;
    return DecodeNormal(encodedNormal);
}

float3 LoadNormal(int2 pos, int2 offset)
{
    float3 encodedNormal = g_NormalmapSource.Load(int3(pos, 0), offset).xyz;
    return DecodeNormal(encodedNormal);
}

// all vectors in viewspace
float CalculatePixelObscurance(float3 pixelNormal, float3 hitDelta, float falloffCalcMulSq)
{
    float lengthSq = dot(hitDelta, hitDelta);
    float NdotD = dot(pixelNormal, hitDelta) / sqrt(lengthSq);

    float falloffMult = max(0.0, lengthSq * falloffCalcMulSq + 1.0);

    return max(0, NdotD - g_ASSAOConsts.EffectHorizonAngleThreshold) * falloffMult;
}

void GenerateSSAOShadowsInternal(
    out float outShadowTerm,
    out float outEdgesPacked,
    out float outWeight,
    const lpuint2 SVPos,
    uniform int qualityLevel,
    bool useArrayIndex,
    const uint arrayIndex,
    Texture2D<float> depthTexture,
    Texture2DArray<float> depthTextureArray);

void SSAOTapInner(
	const int qualityLevel,
	inout float obscuranceSum,
	inout float weightSum,
	const lpfloat2 samplingUV,
	const lpfloat mipLevel,
	const float3 pixCenterPos,
	const float3 negViewspaceDir,
	float3 pixelNormal,
	const float falloffCalcMulSq,
	const float weightMod,
	const uint arrayIndex,
    Texture2D<float> depthTexture,
    Texture2DArray<float> depthTextureArray,
	const int dbgTapIndex)
{
	// get depth at sample
    float viewspaceSampleZ = depthTextureArray.SampleLevel(g_ViewspaceDepthTapSampler, lpfloat3(samplingUV.xy, (lpfloat) arrayIndex), mipLevel).x;

    // convert to viewspace
    float3 hitPos = NDCToViewspace(samplingUV.xy, viewspaceSampleZ).xyz;
    float3 hitDelta = hitPos - pixCenterPos;

    float obscurance = CalculatePixelObscurance(pixelNormal, hitDelta, falloffCalcMulSq);
    float weight = 1.0;

    if (qualityLevel >= SSAO_HALOING_REDUCTION_ENABLE_AT_QUALITY_PRESET)
    {
		//float reduct = max( 0, dot( hitDelta, negViewspaceDir ) );
        float reduct = max(0, -hitDelta.z); // cheaper, less correct version
        reduct = saturate(reduct * g_ASSAOConsts.NegRecEffectRadius + 2.0); // saturate( 2.0 - reduct / g_ASSAOConsts.EffectRadius );
        weight = SSAO_HALOING_REDUCTION_AMOUNT * reduct + (1.0 - SSAO_HALOING_REDUCTION_AMOUNT);
    }
    weight *= weightMod;
    obscuranceSum += obscurance * weight;
    weightSum += weight;

#ifdef SSAO_INTERNAL_SHADER_DEBUG
	g_ShaderDebugOutputUAV[4 + dbgTapIndex * 5 + 0] = samplingUV.x;
	g_ShaderDebugOutputUAV[4 + dbgTapIndex * 5 + 1] = samplingUV.y;
	g_ShaderDebugOutputUAV[4 + dbgTapIndex * 5 + 2] = clamp(mipLevel, 0.0, float(SSAO_DEPTH_MIP_LEVELS) - 1.0); // clamp not actually needed for above
	g_ShaderDebugOutputUAV[4 + dbgTapIndex * 5 + 3] = saturate(weight);
	g_ShaderDebugOutputUAV[4 + dbgTapIndex * 5 + 4] = saturate(1.0 - sqrt(obscurance * 1.3));
#endif
}

void SSAOTap(
	const int qualityLevel,
	inout float obscuranceSum,
	inout float weightSum,
	const int tapIndex,
	const lpfloat2x2 rotScale,
	const float3 pixCenterPos,
	const float3 negViewspaceDir,
	float3 pixelNormal,
	const float2 normalizedScreenPos,
	const lpfloat mipOffset,
	const float falloffCalcMulSq,
	float weightMod,
	lpfloat2 normXY,
	lpfloat normXYLength,
    const uint arrayIndex,
    Texture2D<float> depthTexture,
    Texture2DArray<float> depthTextureArray)
{
	lpfloat2 sampleOffset;
	lpfloat samplePow2Len;

	// patterns
    if (qualityLevel >= SSAO_QUALITY_LEVEL_REFERENCE) // reference pattern, not used for production code
    {
        lpfloat2 off = (lpfloat2) g_ASSAOConsts.SamplesArray[tapIndex].xy;

        lpfloat offLength = length(off);
        off *= pow(abs(offLength), (lpfloat) g_ASSAOConsts.DebugRefSamplesDistribution) / offLength;
        
        samplePow2Len = log2(length(off));

        sampleOffset = mul(rotScale, off);

        weightMod *= g_ASSAOConsts.SamplesArray[tapIndex].z;
    }
    else
	{
		lpfloat4 newSample = (lpfloat4) g_samplePatternMain[tapIndex];
        sampleOffset = mul(rotScale, newSample.xy);
        samplePow2Len = (lpfloat) newSample.w; // precalculated, same as: samplePow2Len = log2( length( newSample.xy ) );
        weightMod *= newSample.z;
    }

#if 0 // experimental - recovers some detail at distance by preventing use of samples that will be snapped to the center pixel and not have any occlusion; in reality it adds too much cost - better to simply increase the sample count
	{
		float pushOut = saturate(length(sampleOffset.xy) * 0.6);
		sampleOffset.xy /= pushOut;
	}
#endif

	// snap to pixel center (more correct obscurance math, avoids artifacts)
    sampleOffset = round(sampleOffset);

	// calculate MIP based on the sample distance from the centre, similar to as described 
	// in http://graphics.cs.williams.edu/papers/SAOHPG12/.
	lpfloat mipLevel = (qualityLevel < SSAO_DEPTH_MIPS_ENABLE_AT_QUALITY_PRESET) ? (0) : (samplePow2Len + mipOffset);

	lpfloat2 samplingUV = (lpfloat2) (sampleOffset * g_ASSAOConsts.Viewport2xPixelSize + normalizedScreenPos);

    SSAOTapInner(
		qualityLevel,
		obscuranceSum,
		weightSum,
		samplingUV,
		mipLevel,
		pixCenterPos,
		negViewspaceDir,
		pixelNormal,
		falloffCalcMulSq,
		weightMod,
        arrayIndex,
        depthTexture,
        depthTextureArray,
		tapIndex * 2);

	// for the second tap, just use the mirrored offset
	lpfloat2 sampleOffsetMirroredUV = -sampleOffset;

	// tilt the second set of samples so that the disk is effectively rotated by the normal
	// effective at removing one set of artifacts, but too expensive for lower quality settings
    if (qualityLevel >= SSAO_TILT_SAMPLES_ENABLE_AT_QUALITY_PRESET)
    {
		lpfloat dotNorm = dot(sampleOffsetMirroredUV, normXY);
        sampleOffsetMirroredUV -= dotNorm * normXYLength * normXY;
        sampleOffsetMirroredUV = round(sampleOffsetMirroredUV);
    }

	// snap to pixel center (more correct obscurance math, avoids artifacts)
	lpfloat2 samplingMirroredUV = (lpfloat2) (sampleOffsetMirroredUV * g_ASSAOConsts.Viewport2xPixelSize + normalizedScreenPos);

    SSAOTapInner(
		qualityLevel,
		obscuranceSum,
		weightSum,
		samplingMirroredUV,
		mipLevel,
		pixCenterPos,
		negViewspaceDir,
		pixelNormal,
		falloffCalcMulSq,
		weightMod,
        arrayIndex,
        depthTexture,
        depthTextureArray,
		tapIndex * 2 + 1);
}

// this function is designed to only work with half/half depth at the moment - there's a couple of hardcoded paths that expect pixel/texel size, so it will not work for full res
void GenerateSSAOShadowsInternal(
    out float outShadowTerm,
    out float outEdgesPacked,
    out float outWeight,
    const lpuint2 SVPos,
    uniform int qualityLevel,
    bool useArrayIndex,
    const uint arrayIndex,
    Texture2D<float> depthTexture,
    Texture2DArray<float> depthTextureArray)
{
    // qualityLevel has to be clamped to 0 to prevent out-of-bounds access for the lowest quality level (q-1).
    const int numberOfTaps = ((qualityLevel == SSAO_QUALITY_LEVEL_REFERENCE) ? (g_ASSAOConsts.AutoMatchSampleCount) : (g_numTaps[qualityLevel]));
    
    float pixZ, pixLZ, pixTZ, pixRZ, pixBZ;

    float4 valuesUL = depthTextureArray.GatherRed(g_PointMirrorSampler, float3(SVPos * g_ASSAOConsts.HalfViewportPixelSize, arrayIndex));
    float4 valuesBR = depthTextureArray.GatherRed(g_PointMirrorSampler, float3(SVPos * g_ASSAOConsts.HalfViewportPixelSize, arrayIndex), int2(1, 1));
    
    // get this pixel's viewspace depth
    pixZ = valuesUL.y; //float pixZ = depthTexture.SampleLevel( g_PointMirrorSampler, normalizedScreenPos, 0.0 ).x; // * g_ASSAOConsts.MaxViewspaceDepth;

    // get left right top bottom neighbouring pixels for edge detection (gets compiled out on qualityLevel == SSAO_QUALITY_LEVEL_LOW)
    pixLZ = valuesUL.x;
    pixTZ = valuesUL.z;
    pixRZ = valuesBR.z;
    pixBZ = valuesBR.x;

    float2 normalizedScreenPos = SVPos * g_ASSAOConsts.Viewport2xPixelSize + g_ASSAOConsts.Viewport2xPixelSize_x_025;
    float3 pixCenterPos = NDCToViewspace(normalizedScreenPos, pixZ); // g

    // Load this pixel's viewspace normal
    lpuint2 PerPassFullResCoordOffset =
        useArrayIndex ?
        lpuint2(arrayIndex % 2, arrayIndex / 2) :
        (lpuint2) g_ASSAOConsts.PerPassFullResCoordOffset.xy;
    lpuint2 fullResCoord = (lpuint2) SVPos * 2 + PerPassFullResCoordOffset.xy;
    float3 pixelNormal = LoadNormal(fullResCoord);

    const float2 pixelDirRBViewspaceSizeAtCenterZ = pixCenterPos.z * g_ASSAOConsts.NDCToViewMul * g_ASSAOConsts.Viewport2xPixelSize; // optimized approximation of:  float2 pixelDirRBViewspaceSizeAtCenterZ = NDCToViewspace( normalizedScreenPos.xy + g_ASSAOConsts.ViewportPixelSize.xy, pixCenterPos.z ).xy - pixCenterPos.xy;

    float pixLookupRadiusMod;
    float falloffCalcMulSq;

    // calculate effect radius and fit our screen sampling pattern inside it
    float effectViewspaceRadius;
    CalculateRadiusParameters(length(pixCenterPos), pixelDirRBViewspaceSizeAtCenterZ, pixLookupRadiusMod, effectViewspaceRadius, falloffCalcMulSq);

    // calculate samples rotation/scaling
    lpfloat2x2 rotScale;
    {
        // reduce effect radius near the screen edges slightly; ideally, one would render a larger depth buffer (5% on each side) instead
        if (qualityLevel >= SSAO_REDUCE_RADIUS_NEAR_SCREEN_BORDER_ENABLE_AT_QUALITY_PRESET)
        {
            float nearScreenBorder = min(min(normalizedScreenPos.x, 1.0 - normalizedScreenPos.x), min(normalizedScreenPos.y, 1.0 - normalizedScreenPos.y));
            nearScreenBorder = saturate(10.0 * nearScreenBorder + 0.6);
            pixLookupRadiusMod *= nearScreenBorder;
        }

        // load & update pseudo-random rotation matrix
        uint index = uint(SVPos.y * 2 + SVPos.x) % 5;
        lpuint pseudoRandomIndex = lpuint(index);
        lpfloat4 rs =
            useArrayIndex ?
            (lpfloat4) g_ASSAOConsts.FullPatternRotScaleMatrices[(arrayIndex * 5) + pseudoRandomIndex] :
            (lpfloat4) g_ASSAOConsts.PatternRotScaleMatrices[pseudoRandomIndex];
        rotScale = lpfloat2x2(rs.x * pixLookupRadiusMod, rs.y * pixLookupRadiusMod, rs.z * pixLookupRadiusMod, rs.w * pixLookupRadiusMod);
    }

    // the main obscurance & sample weight storage
    float obscuranceSum = 0.0;
    float weightSum = 0.0;

    // Move center pixel slightly towards camera to avoid imprecision artifacts due to using of 16bit depth buffer; a lot smaller offsets needed when using 32bit floats
    pixCenterPos *= g_ASSAOConsts.DepthPrecisionOffsetMod;

#ifdef SSAO_DEBUG_SHOWNORMALS
    g_DebuggingOutputUAV[SVPos * 2 + PerPassFullResCoordOffset] = float4(abs(pixelNormal.xyz * 0.5 + 0.5), 1.0);
#endif

#ifdef SSAO_INTERNAL_SHADER_DEBUG
    outEdgesPacked = 0;
    outShadowTerm = 0.0;
    outWeight = 0;

    int2 FullResOffset =
        useArrayIndex ?
        int2((int)arrayIndex % 2, (int)arrayIndex / 2) :
        (int2) g_ASSAOConsts.FullResOffset;        
    int2 fullResPos = SVPos.xy * 2 + FullResOffset;

    // This needs to happen for only one pass.
    bool proceed = all(fullResPos == g_ASSAOConsts.DebugCursorPos);
    if(useArrayIndex)
    {
        int pixelPassID = g_ASSAOConsts.DebugCursorPos.x % 2 + ( g_ASSAOConsts.DebugCursorPos.y % 2 ) * 2;
        proceed = proceed && ( pixelPassID == 0/*(int)arrayIndex*/ );
    }
    if (proceed)
    {
        int __count = (int)((float)(numberOfTaps * 2) + (int)SVPos.x / 16768.0);

        g_ShaderDebugOutputUAV[0] = asfloat(fullResPos.x); //asfloat( (int)numberOfTaps );
        g_ShaderDebugOutputUAV[1] = asfloat(fullResPos.y); //0.0;
        g_ShaderDebugOutputUAV[2] = asfloat(__count); //asfloat( (int)numberOfTaps );
        g_ShaderDebugOutputUAV[3] = 0;
    }
    else
    {
        discard;
        return;
    }
#endif

    const float globalMipOffset = SSAO_DEPTH_MIPS_GLOBAL_OFFSET;
    float mipOffset = (qualityLevel < SSAO_DEPTH_MIPS_ENABLE_AT_QUALITY_PRESET) ? (0) : (log2(pixLookupRadiusMod) + globalMipOffset);

    // Used to tilt the second set of samples so that the disk is effectively rotated by the normal
    // effective at removing one set of artifacts, but too expensive for lower quality settings
    lpfloat2 normXY = lpfloat2(pixelNormal.x, pixelNormal.y);
    lpfloat normXYLength = length(normXY);
    normXY /= normXYLength;
    normXY.y = -normXY.y;
    normXYLength *= SSAO_TILT_SAMPLES_AMOUNT;

    const float3 negViewspaceDir = -normalize(pixCenterPos);

    // standard, non-adaptive approach

    // NOTE: Need to add a branch specifically for quality level 4 as the unroll attribute cannot be applied in this case since the loop termination value 'numberOfTaps' 
    //       -is not determined at compile time for quality level 4. The compiler will be able to eliminate the branch when generating code because 'qualityLevel' is 
    //       -determined at compile time and can hence be folded; which should avoid degradation in performance.
    if (qualityLevel == SSAO_QUALITY_LEVEL_REFERENCE)
    {
        // NOTE: Cannot unroll the loop with quality level 5 as numberOfTaps is set to a constant buffer value.
        //[unroll] // <- doesn't seem to help on any platform, although the compilers seem to unroll anyway if const number of tap used!
        for (int i = 0; i < numberOfTaps; i++)
        {
            SSAOTap(
                qualityLevel,
                obscuranceSum,
                weightSum,
                i,
                (lpfloat2x2) rotScale,
                pixCenterPos,
                negViewspaceDir,
                pixelNormal,
                normalizedScreenPos,
                (lpfloat) mipOffset,
                falloffCalcMulSq,
                1.0,
                (lpfloat2) normXY,
                (lpfloat) normXYLength,
                arrayIndex,
                depthTexture,
                depthTextureArray);
        }
    }
    else
    {
        [unroll] // <- doesn't seem to help on any platform, although the compilers seem to unroll anyway if const number of tap used!
        for (int i = 0; i < numberOfTaps; i++)
        {
            SSAOTap(
                qualityLevel,
                obscuranceSum,
                weightSum,
                i,
                (lpfloat2x2) rotScale,
                pixCenterPos,
                negViewspaceDir,
                pixelNormal,
                normalizedScreenPos,
                (lpfloat) mipOffset,
                falloffCalcMulSq,
                1.0,
                (lpfloat2) normXY,
                (lpfloat) normXYLength,
                arrayIndex,
                depthTexture,
                depthTextureArray);
        }
    }    

#ifdef SSAO_DEBUG_SHOWSAMPLEHEATMAP
    float sampleCount = (float)numberOfTaps / (float)SSAO_MAX_TAPS;
    g_DebuggingOutputUAV[SVPos * 2 + PerPassFullResCoordOffset] = pow(saturate(float4(sampleCount, 1.0 - sampleCount, 0.0, 1.0)), 0.45);
#endif

    // edge mask for between this and left/right/top/bottom neighbour pixels - not used in quality level 0 so initialize to "no edge" (1 is no edge, 0 is edge)
    float4 edgesLRTB = float4(1.0, 1.0, 1.0, 1.0);

    if (qualityLevel >= SSAO_DEPTH_BASED_EDGES_ENABLE_AT_QUALITY_PRESET)
    {
        edgesLRTB = CalculateEdges(pixZ, pixLZ, pixRZ, pixTZ, pixBZ);
    }

    // adds a more high definition sharp effect, which gets blurred out (reuses left/right/top/bottom samples that we used for edge detection)
    [branch]
    if (any(edgesLRTB != 0.0f))
    {
        if (qualityLevel >= SSAO_DETAIL_AO_ENABLE_AT_QUALITY_PRESET)
        {
            // disable in case of quality level reference
            if (qualityLevel != SSAO_QUALITY_LEVEL_REFERENCE)
            {
                //approximate neighbouring pixels positions (actually just deltas or "positions - pixCenterPos" )
                float3 viewspaceDirZNormalized = float3(pixCenterPos.xy / pixCenterPos.zz, 1.0);
                float3 pixLDelta = float3(-pixelDirRBViewspaceSizeAtCenterZ.x, 0.0, 0.0) + viewspaceDirZNormalized * (pixLZ - pixCenterPos.z); // very close approximation of: float3 pixLPos  = NDCToViewspace( normalizedScreenPos + float2( -g_ASSAOConsts.HalfViewportPixelSize.x, 0.0 ), pixLZ ).xyz - pixCenterPos.xyz;
                float3 pixRDelta = float3(+pixelDirRBViewspaceSizeAtCenterZ.x, 0.0, 0.0) + viewspaceDirZNormalized * (pixRZ - pixCenterPos.z); // very close approximation of: float3 pixRPos  = NDCToViewspace( normalizedScreenPos + float2( +g_ASSAOConsts.HalfViewportPixelSize.x, 0.0 ), pixRZ ).xyz - pixCenterPos.xyz;
                float3 pixTDelta = float3(0.0, -pixelDirRBViewspaceSizeAtCenterZ.y, 0.0) + viewspaceDirZNormalized * (pixTZ - pixCenterPos.z); // very close approximation of: float3 pixTPos  = NDCToViewspace( normalizedScreenPos + float2( 0.0, -g_ASSAOConsts.HalfViewportPixelSize.y ), pixTZ ).xyz - pixCenterPos.xyz;
                float3 pixBDelta = float3(0.0, +pixelDirRBViewspaceSizeAtCenterZ.y, 0.0) + viewspaceDirZNormalized * (pixBZ - pixCenterPos.z); // very close approximation of: float3 pixBPos  = NDCToViewspace( normalizedScreenPos + float2( 0.0, +g_ASSAOConsts.HalfViewportPixelSize.y ), pixBZ ).xyz - pixCenterPos.xyz;

                const float rangeReductionConst = 4.0f; // this is to avoid various artifacts
                const float modifiedFalloffCalcMulSq = rangeReductionConst * falloffCalcMulSq;

                float4 additionalObscurance;
                additionalObscurance.x = CalculatePixelObscurance(pixelNormal, pixLDelta, modifiedFalloffCalcMulSq);
                additionalObscurance.y = CalculatePixelObscurance(pixelNormal, pixRDelta, modifiedFalloffCalcMulSq);
                additionalObscurance.z = CalculatePixelObscurance(pixelNormal, pixTDelta, modifiedFalloffCalcMulSq);
                additionalObscurance.w = CalculatePixelObscurance(pixelNormal, pixBDelta, modifiedFalloffCalcMulSq);

                obscuranceSum += g_ASSAOConsts.DetailAOStrength * dot(additionalObscurance, edgesLRTB);
            }
        }

        // Sharp normals also create edges - but this adds to the cost as well
        if (qualityLevel >= SSAO_NORMAL_BASED_EDGES_ENABLE_AT_QUALITY_PRESET)
        {
            float3 neighbourNormalL = LoadNormal(fullResCoord, int2(-2, 0));
            float3 neighbourNormalR = LoadNormal(fullResCoord, int2(2, 0));
            float3 neighbourNormalT = LoadNormal(fullResCoord, int2(0, -2));
            float3 neighbourNormalB = LoadNormal(fullResCoord, int2(0, 2));

            const float dotThreshold = SSAO_NORMAL_BASED_EDGES_DOT_THRESHOLD;

            float4 normalEdgesLRTB;
            normalEdgesLRTB.x = saturate((dot(pixelNormal, neighbourNormalL) + dotThreshold));
            normalEdgesLRTB.y = saturate((dot(pixelNormal, neighbourNormalR) + dotThreshold));
            normalEdgesLRTB.z = saturate((dot(pixelNormal, neighbourNormalT) + dotThreshold));
            normalEdgesLRTB.w = saturate((dot(pixelNormal, neighbourNormalB) + dotThreshold));

            //#define SSAO_SMOOTHEN_NORMALS // fixes some aliasing artifacts but kills a lot of high detail and adds to the cost - not worth it probably but feel free to play with it
#ifdef SSAO_SMOOTHEN_NORMALS
            //neighbourNormalL  = LoadNormal( fullResCoord, int2( -1,  0 ) );
            //neighbourNormalR  = LoadNormal( fullResCoord, int2(  1,  0 ) );
            //neighbourNormalT  = LoadNormal( fullResCoord, int2(  0, -1 ) );
            //neighbourNormalB  = LoadNormal( fullResCoord, int2(  0,  1 ) );
            pixelNormal += neighbourNormalL * edgesLRTB.x + neighbourNormalR * edgesLRTB.y + neighbourNormalT * edgesLRTB.z + neighbourNormalB * edgesLRTB.w;
            pixelNormal = normalize(pixelNormal);
#endif

            edgesLRTB *= normalEdgesLRTB;
        }
    }

    outEdgesPacked = PackEdges(edgesLRTB);    

#ifdef SSAO_DEBUG_SHOWEDGES
    g_DebuggingOutputUAV[SVPos * 2 + PerPassFullResCoordOffset] = 1.0 - float4(edgesLRTB.x, edgesLRTB.y * 0.5 + edgesLRTB.w * 0.5, edgesLRTB.z, 0.0);
#endif

    // calculate weighted average
    float obscurance = obscuranceSum / weightSum;

    // strength
    obscurance = g_ASSAOConsts.EffectShadowStrength * obscurance;

    // clamp
    obscurance = min(obscurance, g_ASSAOConsts.EffectShadowClamp);

    // leaving this in as a reference; cool edge aware blur but produces chunky output
    //if( useDDXDDYBlur )
    //{
    //    // smart version
    //    float2 zDeltaXY = abs( float2( ddx( pixCenterPos.z ), ddy( pixCenterPos.z ) ) );
    //    float2 zDeltaXYWeights = saturate( zDeltaXY * 2.0 * falloffCalcMul + falloffCalcAdd );
    //    
    //    float2 interleavedPattern  = (frac( SVPos.xy * 0.5.xx ) - 0.5.xx) * 2.0.xx;
    //    float averageObscurance = obscurance;
    //    averageObscurance -= ddx(averageObscurance) * interleavedPattern.x * zDeltaXYWeights.x * 0.33;
    //    averageObscurance -= ddy(averageObscurance) * interleavedPattern.y * zDeltaXYWeights.y * 0.33;
    //    obscurance = averageObscurance;
    //}

    // calculate fadeout (1 close, gradient, 0 far)
    float fadeOut = saturate(pixCenterPos.z * g_ASSAOConsts.EffectFadeOutMul + g_ASSAOConsts.EffectFadeOutAdd);

    // Reduce the SSAO shadowing if we're on the edge to remove artifacts on edges (we don't care for the lower quality one)
    if (qualityLevel >= SSAO_DEPTH_BASED_EDGES_ENABLE_AT_QUALITY_PRESET)
    {
        // float edgeCount = dot( 1.0-edgesLRTB, float4( 1.0, 1.0, 1.0, 1.0 ) );

        // when there's more than 2 opposite edges, start fading out the occlusion to reduce aliasing artifacts
        float edgeFadeoutFactor = saturate((1.0 - edgesLRTB.x - edgesLRTB.y) * 0.35) + saturate((1.0 - edgesLRTB.z - edgesLRTB.w) * 0.35);

        // (experimental) if you want to reduce the effect next to any edge
        // edgeFadeoutFactor += 0.1 * saturate( dot( 1 - edgesLRTB, float4( 1, 1, 1, 1 ) ) );

        fadeOut *= saturate(1.0 - edgeFadeoutFactor);
    }

    // same as a bove, but a lot more conservative version
    // fadeOut *= saturate( dot( edgesLRTB, float4( 0.9, 0.9, 0.9, 0.9 ) ) - 2.6 );

    // fadeout
    obscurance *= fadeOut;

    // conceptually switch to occlusion with the meaning being visibility (grows with visibility, occlusion == 1 implies full visibility), 
    // to be in line with what is more commonly used.
    float occlusion = 1.0 - obscurance;

    // modify the gradient
    // note: this cannot be moved to a later pass because of loss of precision after storing in the render target
    occlusion = pow(saturate(occlusion), g_ASSAOConsts.EffectShadowPow);

    // outputs!
    outShadowTerm = occlusion; // Our final 'occlusion' term (0 means fully occluded, 1 means fully lit)
    outWeight = weightSum;
}

float2 GenerateSSAOShadowsInternalWrapper(
    const lpuint3 inPos, 
    const int qualityLevel, 
    const bool isMerged, 
    const bool useArrayIndex, 
    const uint arrayIndex)
{
    float outShadowTerm = 0;
    float outWeight = 0;
    float outEdgesPacked = 0;
    GenerateSSAOShadowsInternal(
        outShadowTerm,
        outEdgesPacked,
        outWeight,
        (lpuint2) inPos.xy,
        qualityLevel,
        useArrayIndex,
        arrayIndex,
        g_ViewspaceDepthSource,
        g_ViewspaceDepthSourceArray2);
    float2 result = 
        (qualityLevel <= SSAO_QUALITY_LEVEL_LOW) ?
        float2(outShadowTerm, PackEdges(float4(1, 1, 1, 1))) :
        float2(outShadowTerm, outEdgesPacked);
    return result;
}

void PSGenerateLowestQuality(in float4 inPos : SV_POSITION, out float2 out0 : SV_Target0)
{
    const bool useArrayIndex = false;
    const int qualityLevel = SSAO_QUALITY_LEVEL_LOWEST;
    const uint arrayIndex = 0;
    lpuint3 inPos2 = lpuint3((lpuint2) trunc(inPos.xy), 0);
    const bool isMerged = false;
    out0 = GenerateSSAOShadowsInternalWrapper(inPos2, qualityLevel, isMerged, useArrayIndex, arrayIndex);
}

void PSGenerateLowQuality(in float4 inPos : SV_POSITION, out float2 out0 : SV_Target0)
{
    const bool useArrayIndex = false;
    const int qualityLevel = SSAO_QUALITY_LEVEL_LOW;
    const uint arrayIndex = 0;
    lpuint3 inPos2 = lpuint3((lpuint2) trunc(inPos.xy), 0);
    const bool isMerged = false;
    out0 = GenerateSSAOShadowsInternalWrapper(inPos2, qualityLevel, isMerged, useArrayIndex, arrayIndex);
}

void PSGenerateMediumQuality(in float4 inPos : SV_POSITION, out float2 out0 : SV_Target0)
{
    const bool useArrayIndex = false;
    const int qualityLevel = SSAO_QUALITY_LEVEL_MEDIUM;
    const uint arrayIndex = 0;
    lpuint3 inPos2 = lpuint3((lpuint2) trunc(inPos.xy), 0);
    const bool isMerged = false;
    out0 = GenerateSSAOShadowsInternalWrapper(inPos2, qualityLevel, isMerged, useArrayIndex, arrayIndex);
}

void PSGenerateHighQuality(in float4 inPos : SV_POSITION, out float2 out0 : SV_Target0)
{
    const bool useArrayIndex = false;
    const int qualityLevel = SSAO_QUALITY_LEVEL_HIGH;
    const uint arrayIndex = 0;
    lpuint3 inPos2 = lpuint3((lpuint2) trunc(inPos.xy), 0);
    const bool isMerged = false;
    out0 = GenerateSSAOShadowsInternalWrapper(inPos2, qualityLevel, isMerged, useArrayIndex, arrayIndex);
}

void PSGenerateHighestQuality(in float4 inPos : SV_POSITION, out float2 out0 : SV_Target0)
{
    const bool useArrayIndex = false;
    const int qualityLevel = SSAO_QUALITY_LEVEL_HIGHEST;
    const uint arrayIndex = 0;
    lpuint3 inPos2 = lpuint3((lpuint2) trunc(inPos.xy), 0);
    const bool isMerged = false;
    out0 = GenerateSSAOShadowsInternalWrapper(inPos2, qualityLevel, isMerged, useArrayIndex, arrayIndex);
}

void PSGenerateReferenceQuality(in float4 inPos : SV_POSITION, out float2 out0 : SV_Target0)
{
    bool useArrayIndex = false;
    int qualityLevel = SSAO_QUALITY_LEVEL_REFERENCE;
    uint arrayIndex = 0;
    lpuint3 inPos2 = lpuint3((lpuint2) trunc(inPos.xy), 0);
    const bool isMerged = false;
    out0 = GenerateSSAOShadowsInternalWrapper(inPos2, qualityLevel, isMerged, useArrayIndex, arrayIndex);
}

// ********************************************************************************************************
// Pixel shader that does smart blurring (to avoid bleeding)

void AddSample(float ssaoValue, float edgeValue, inout float sum, inout float sumWeight)
{
    float weight = edgeValue;

    sum += (weight * ssaoValue);
    sumWeight += weight;
}

float2 SampleBlurredWide(int2 inPos, float2 coord)
{
    float2 vC = g_BlurInput.SampleLevel(g_PointMirrorSampler, coord, 0.0, int2(0, 0)).xy;
    float2 vL = g_BlurInput.SampleLevel(g_PointMirrorSampler, coord, 0.0, int2(-2, 0)).xy;
    float2 vT = g_BlurInput.SampleLevel(g_PointMirrorSampler, coord, 0.0, int2(0, -2)).xy;
    float2 vR = g_BlurInput.SampleLevel(g_PointMirrorSampler, coord, 0.0, int2(2, 0)).xy;
    float2 vB = g_BlurInput.SampleLevel(g_PointMirrorSampler, coord, 0.0, int2(0, 2)).xy;

    float packedEdges = vC.y;
    float4 edgesLRTB = UnpackEdges(packedEdges);
    edgesLRTB.x *= UnpackEdges(vL.y).y;
    edgesLRTB.z *= UnpackEdges(vT.y).w;
    edgesLRTB.y *= UnpackEdges(vR.y).x;
    edgesLRTB.w *= UnpackEdges(vB.y).z;

    //this would be more mathematically correct but there's little difference at cost
    //edgesLRTB.x         = sqrt( edgesLRTB.x );
    //edgesLRTB.z         = sqrt( edgesLRTB.z );
    //edgesLRTB.y         = sqrt( edgesLRTB.y );
    //edgesLRTB.w         = sqrt( edgesLRTB.w );

    float ssaoValue = vC.x;
    float ssaoValueL = vL.x;
    float ssaoValueT = vT.x;
    float ssaoValueR = vR.x;
    float ssaoValueB = vB.x;

    float sumWeight = 0.8f;
    float sum = ssaoValue * sumWeight;

    AddSample(ssaoValueL, edgesLRTB.x, sum, sumWeight);
    AddSample(ssaoValueR, edgesLRTB.y, sum, sumWeight);
    AddSample(ssaoValueT, edgesLRTB.z, sum, sumWeight);
    AddSample(ssaoValueB, edgesLRTB.w, sum, sumWeight);

    float ssaoAvg = sum / sumWeight;

    ssaoValue = ssaoAvg; //min( ssaoValue, ssaoAvg ) * 0.2 + ssaoAvg * 0.8;

    return float2(ssaoValue, packedEdges);
}

float2 SampleBlurred(int2 inPos, float2 coord)
{
    float packedEdges = g_BlurInput.Load(int3(inPos.xy, 0)).y;
    float4 edgesLRTB = UnpackEdges(packedEdges);

    float4 valuesUL = g_BlurInput.GatherRed(g_PointMirrorSampler, coord - g_ASSAOConsts.HalfViewportPixelSize * 0.5);
    float4 valuesBR = g_BlurInput.GatherRed(g_PointMirrorSampler, coord + g_ASSAOConsts.HalfViewportPixelSize * 0.5);

    float ssaoValue = valuesUL.y;
    float ssaoValueL = valuesUL.x;
    float ssaoValueT = valuesUL.z;
    float ssaoValueR = valuesBR.z;
    float ssaoValueB = valuesBR.x;

    float sumWeight = 0.5f;
    float sum = ssaoValue * sumWeight;

    AddSample(ssaoValueL, edgesLRTB.x, sum, sumWeight);
    AddSample(ssaoValueR, edgesLRTB.y, sum, sumWeight);

    AddSample(ssaoValueT, edgesLRTB.z, sum, sumWeight);
    AddSample(ssaoValueB, edgesLRTB.w, sum, sumWeight);

    float ssaoAvg = sum / sumWeight;

    ssaoValue = ssaoAvg; //min( ssaoValue, ssaoAvg ) * 0.2 + ssaoAvg * 0.8;

    return float2(ssaoValue, packedEdges);
}

// edge-sensitive blur
float2 PSSmartBlur(in float4 inPos : SV_POSITION, in float2 inUV : TEXCOORD0) : SV_Target
{
    return SampleBlurred(int2(inPos.xy), inUV);
}

// edge-sensitive blur (wider kernel)
float2 PSSmartBlurWide(in float4 inPos : SV_POSITION, in float2 inUV : TEXCOORD0) : SV_Target
{
    return SampleBlurredWide(int2(inPos.xy), inUV);
}

float4 PSApply(in float4 inPos : SV_POSITION) : SV_Target
{
    float ao;
    uint2 pixPos = (uint2) inPos.xy;
    uint2 pixPosHalf = pixPos / uint2(2, 2);

#if defined(SSAO_DEBUG_SHOWNORMALS) || defined( SSAO_DEBUG_SHOWEDGES ) || defined( SSAO_DEBUG_SHOWSAMPLEHEATMAP )
    return pow( abs( g_DebuggingOutputSRV.Load( int3( pixPos, 0 ) ) ), 2.2 );
#endif

    // calculate index in the four deinterleaved source array texture
    int mx = (pixPos.x % 2);
    int my = (pixPos.y % 2);
    int ic = mx + my * 2; // center index
    int ih = (1 - mx) + my * 2; // neighbouring, horizontal
    int iv = mx + (1 - my) * 2; // neighbouring, vertical
    int id = (1 - mx) + (1 - my) * 2; // diagonal
    
    float2 centerVal = g_FinalSSAO.Load(int4(pixPosHalf, ic, 0)).xy;
    
    ao = centerVal.x;

#if 1   // for debugging - set to 0 to disable last pass high-res blur
    float4 edgesLRTB = UnpackEdges(centerVal.y);

    // convert index shifts to sampling offsets
    float fmx = (float) mx;
    float fmy = (float) my;
    
    // in case of an edge, push sampling offsets away from the edge (towards pixel center)
    float fmxe = (edgesLRTB.y - edgesLRTB.x);
    float fmye = (edgesLRTB.w - edgesLRTB.z);

    // calculate final sampling offsets and sample using bilinear filter
    float2 uvH = (inPos.xy + float2(fmx + fmxe - 0.5, 0.5 - fmy)) * 0.5 * g_ASSAOConsts.HalfViewportPixelSize;
    float aoH = g_FinalSSAO.SampleLevel(g_LinearClampSampler, float3(uvH, ih), 0).x;
    float2 uvV = (inPos.xy + float2(0.5 - fmx, fmy - 0.5 + fmye)) * 0.5 * g_ASSAOConsts.HalfViewportPixelSize;
    float aoV = g_FinalSSAO.SampleLevel(g_LinearClampSampler, float3(uvV, iv), 0).x;
    float2 uvD = (inPos.xy + float2(fmx - 0.5 + fmxe, fmy - 0.5 + fmye)) * 0.5 * g_ASSAOConsts.HalfViewportPixelSize;
    float aoD = g_FinalSSAO.SampleLevel(g_LinearClampSampler, float3(uvD, id), 0).x;

    // reduce weight for samples near edge - if the edge is on both sides, weight goes to 0
    float4 blendWeights;
    blendWeights.x = 1.0;
    blendWeights.y = (edgesLRTB.x + edgesLRTB.y) * 0.5;
    blendWeights.z = (edgesLRTB.z + edgesLRTB.w) * 0.5;
    blendWeights.w = (blendWeights.y + blendWeights.z) * 0.5;

    // calculate weighted average
    float blendWeightsSum = dot(blendWeights, float4(1.0, 1.0, 1.0, 1.0));
    ao = dot(float4(ao, aoH, aoV, aoD), blendWeights) / blendWeightsSum;
#endif

    return float4(ao.xxx, 1.0);
}


// edge-ignorant blur in x and y directions, 9 pixels touched (for the lowest quality level 0)
float2 PSNonSmartBlur(in float4 inPos : SV_POSITION, in float2 inUV : TEXCOORD0) : SV_Target
{
    float2 halfPixel = g_ASSAOConsts.HalfViewportPixelSize * 0.5f;

    float2 centre = g_BlurInput.SampleLevel(g_LinearClampSampler, inUV, 0.0).xy;

    float4 vals;
    vals.x = g_BlurInput.SampleLevel(g_LinearClampSampler, inUV + float2(-halfPixel.x * 3, -halfPixel.y), 0.0).x;
    vals.y = g_BlurInput.SampleLevel(g_LinearClampSampler, inUV + float2(+halfPixel.x, -halfPixel.y * 3), 0.0).x;
    vals.z = g_BlurInput.SampleLevel(g_LinearClampSampler, inUV + float2(-halfPixel.x, +halfPixel.y * 3), 0.0).x;
    vals.w = g_BlurInput.SampleLevel(g_LinearClampSampler, inUV + float2(+halfPixel.x * 3, +halfPixel.y), 0.0).x;

    return float2(dot(vals, 0.2.xxxx) + centre.x * 0.2, centre.y);
}



// edge-ignorant blur & apply (for the Low quality level 0)
float4 PSNonSmartApply(in float4 inPos : SV_POSITION, in float2 inUV : TEXCOORD0) : SV_Target
{
#if defined(SSAO_DEBUG_SHOWNORMALS) || defined( SSAO_DEBUG_SHOWEDGES ) || defined( SSAO_DEBUG_SHOWSAMPLEHEATMAP )
    return pow( abs( g_DebuggingOutputSRV.Load( int3( (uint2)inPos.xy, 0 ) ) ), 2.2 );
#endif

    float a = g_FinalSSAO.SampleLevel(g_LinearClampSampler, float3(inUV.xy, 0), 0.0).x;
    float b = g_FinalSSAO.SampleLevel(g_LinearClampSampler, float3(inUV.xy, 1), 0.0).x;
    float c = g_FinalSSAO.SampleLevel(g_LinearClampSampler, float3(inUV.xy, 2), 0.0).x;
    float d = g_FinalSSAO.SampleLevel(g_LinearClampSampler, float3(inUV.xy, 3), 0.0).x;
    float avg = (a + b + c + d) * 0.25;
    return float4(avg.xxx, 1.0);
}

// edge-ignorant blur & apply (for the Low quality level 0and quality level -1)
// Extra optimization for versions that use non-smarth blur.
// Since results are pre_averaged in the blur pass, we just need to return the value
float4 PSNonSmartApplySingleSource(in float4 inPos : SV_POSITION, in float2 inUV : TEXCOORD0) : SV_Target
{
#if defined(SSAO_DEBUG_SHOWNORMALS) || defined( SSAO_DEBUG_SHOWEDGES ) || defined( SSAO_DEBUG_SHOWSAMPLEHEATMAP )
    return pow(abs(g_DebuggingOutputSRV.Load(int3((uint2)inPos.xy, 0))), 2.2);
#endif

    float a = g_FinalSSAO.SampleLevel(g_LinearClampSampler, float3(inUV.xy, 0), 0.0).x;
    return float4(a.xxx, 1.0);
}

// edge-ignorant blur & apply, skipping half pixels in checkerboard pattern (for the Lowest quality level and Settings::SkipHalfPixelsOnLowQualityLevel == true )
float4 PSNonSmartHalfApply(in float4 inPos : SV_POSITION, in float2 inUV : TEXCOORD0) : SV_Target
{
#if defined(SSAO_DEBUG_SHOWNORMALS) || defined( SSAO_DEBUG_SHOWEDGES ) || defined( SSAO_DEBUG_SHOWSAMPLEHEATMAP )
    if( ( (uint(inPos.x) % 2) != 0 ) || ( (uint(inPos.y) % 2) != 0 ) ) return float4( 0, 0, 0, 1 );
    return pow( abs( g_DebuggingOutputSRV.Load( int3( (uint2)inPos.xy, 0 ) ) ), 2.2 );
#endif

    float a = g_FinalSSAO.SampleLevel(g_LinearClampSampler, float3(inUV.xy, 0), 0.0).x;
    float d = g_FinalSSAO.SampleLevel(g_LinearClampSampler, float3(inUV.xy, 3), 0.0).x;
    float avg = (a + d) * 0.5;
    return float4(avg.xxx, 1.0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Compute Shader version below
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

[numthreads(SSAO_THREAD_GROUP_SIZE_X, SSAO_THREAD_GROUP_SIZE_Y, 1)]
void CSPrepareDepths(uint3 dispatchThreadID : SV_DispatchThreadID)
{
#if 1   // gather can be a bit faster but doesn't work with input depth buffers that don't match the working viewport
    float2 gatherUV = float2(dispatchThreadID.xy + 0.5) * g_ASSAOConsts.Viewport2xPixelSize;
    float4 depths = g_DepthSource.GatherRed(g_PointClampSampler, gatherUV);
    float a = depths.w; // g_DepthSource.Load( int3( int2(inPos.xy) * 2, 0 ), int2( 0, 0 ) ).x;
    float b = depths.z; // g_DepthSource.Load( int3( int2(inPos.xy) * 2, 0 ), int2( 1, 0 ) ).x;
    float c = depths.x; // g_DepthSource.Load( int3( int2(inPos.xy) * 2, 0 ), int2( 0, 1 ) ).x;
    float d = depths.y; // g_DepthSource.Load( int3( int2(inPos.xy) * 2, 0 ), int2( 1, 1 ) ).x;
#else
	int3 baseCoord = int3(dispatchThreadID.xy * 2, 0);
	float a = g_DepthSource.Load(baseCoord, int2(0, 0)).x;
	float b = g_DepthSource.Load(baseCoord, int2(1, 0)).x;
	float c = g_DepthSource.Load(baseCoord, int2(0, 1)).x;
	float d = g_DepthSource.Load(baseCoord, int2(1, 1)).x;
#endif

	g_DepthOutputUAVArray[uint3(dispatchThreadID.xy, 0)] = ScreenSpaceToViewSpaceDepth(a);
	g_DepthOutputUAVArray[uint3(dispatchThreadID.xy, 1)] = ScreenSpaceToViewSpaceDepth(b);
	g_DepthOutputUAVArray[uint3(dispatchThreadID.xy, 2)] = ScreenSpaceToViewSpaceDepth(c);
	g_DepthOutputUAVArray[uint3(dispatchThreadID.xy, 3)] = ScreenSpaceToViewSpaceDepth(d);
}

[numthreads(SSAO_THREAD_GROUP_SIZE_X, SSAO_THREAD_GROUP_SIZE_Y, 1)]
void CSPrepareDepthsHalf(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int3 baseCoord = int3(int2(dispatchThreadID.xy) * 2, 0);
    float a = g_DepthSource.Load(baseCoord, int2(0, 0)).x;
    float d = g_DepthSource.Load(baseCoord, int2(1, 1)).x;

	g_DepthOutputUAVArray[uint3(dispatchThreadID.xy, 0)] = ScreenSpaceToViewSpaceDepth(a);
	g_DepthOutputUAVArray[uint3(dispatchThreadID.xy, 3)] = ScreenSpaceToViewSpaceDepth(d);
}

[numthreads(SSAO_THREAD_GROUP_SIZE_X, SSAO_THREAD_GROUP_SIZE_Y, 1)]
void CSPrepareDepthsAndNormals(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 halfViewPortSize = g_ASSAOConsts.ViewportHalfSize;
    if ((dispatchThreadID.x >= halfViewPortSize.x) ||
		(dispatchThreadID.y >= halfViewPortSize.y))
    {
        return;
    }

    int2 baseCoords = dispatchThreadID.xy * 2;
    float2 upperLeftUV = (float2(dispatchThreadID.xy) - 0.25) * g_ASSAOConsts.Viewport2xPixelSize;

    float pixZs[4][4];

#if 0   // gather can be a bit faster but doesn't work with input depth buffers that don't match the working viewport
	float2 gatherUV = float2(dispatchThreadID.xy) * g_ASSAOConsts.Viewport2xPixelSize;
	float4 depths = g_DepthSource.GatherRed(g_PointClampSampler, gatherUV);
	pixZs[1][1] = ScreenSpaceToViewSpaceDepth(depths.w);
	pixZs[2][1] = ScreenSpaceToViewSpaceDepth(depths.z);
	pixZs[1][2] = ScreenSpaceToViewSpaceDepth(depths.x);
	pixZs[2][2] = ScreenSpaceToViewSpaceDepth(depths.y);
#else
    int3 baseCoord = int3(dispatchThreadID.xy * 2, 0);
    pixZs[1][1] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(0, 0)).x);
    pixZs[2][1] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(1, 0)).x);
    pixZs[1][2] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(0, 1)).x);
    pixZs[2][2] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(1, 1)).x);
#endif

    g_DepthOutputUAVArray[uint3(dispatchThreadID.xy, 0)] = pixZs[1][1];
    g_DepthOutputUAVArray[uint3(dispatchThreadID.xy, 1)] = pixZs[2][1];
    g_DepthOutputUAVArray[uint3(dispatchThreadID.xy, 2)] = pixZs[1][2];
    g_DepthOutputUAVArray[uint3(dispatchThreadID.xy, 3)] = pixZs[2][2];

	// left 2
    pixZs[0][1] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(-1, 0)).x);
    pixZs[0][2] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(-1, 1)).x);
	// right 2	  																		
    pixZs[3][1] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(2, 0)).x);
    pixZs[3][2] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(2, 1)).x);
	// top 2
    pixZs[1][0] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(0, -1)).x);
    pixZs[2][0] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(1, -1)).x);
	// bottom 2	  																		
    pixZs[1][3] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(0, 2)).x);
    pixZs[2][3] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(1, 2)).x);

    float4 edges0 = CalculateEdges(pixZs[1][1], pixZs[0][1], pixZs[2][1], pixZs[1][0], pixZs[1][2]);
    float4 edges1 = CalculateEdges(pixZs[2][1], pixZs[1][1], pixZs[3][1], pixZs[2][0], pixZs[2][2]);
    float4 edges2 = CalculateEdges(pixZs[1][2], pixZs[0][2], pixZs[2][2], pixZs[1][1], pixZs[1][3]);
    float4 edges3 = CalculateEdges(pixZs[2][2], pixZs[1][2], pixZs[3][2], pixZs[2][1], pixZs[2][3]);

    float3 pixPos[4][4];

	// there is probably a way to optimize the math below; however no approximation will work, has to be precise.

	// middle 4
    pixPos[1][1] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(0.0, 0.0), pixZs[1][1]);
    pixPos[2][1] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(1.0, 0.0), pixZs[2][1]);
    pixPos[1][2] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(0.0, 1.0), pixZs[1][2]);
    pixPos[2][2] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(1.0, 1.0), pixZs[2][2]);
	// left 2
    pixPos[0][1] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(-1.0, 0.0), pixZs[0][1]);
    pixPos[0][2] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(-1.0, 1.0), pixZs[0][2]);
	// right 2                                                                                     
    pixPos[3][1] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(2.0, 0.0), pixZs[3][1]);
    pixPos[3][2] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(2.0, 1.0), pixZs[3][2]);
	// top 2                                                                                       
    pixPos[1][0] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(0.0, -1.0), pixZs[1][0]);
    pixPos[2][0] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(1.0, -1.0), pixZs[2][0]);
	// bottom 2                                                                                    
    pixPos[1][3] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(0.0, 2.0), pixZs[1][3]);
    pixPos[2][3] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(1.0, 2.0), pixZs[2][3]);

    float3 norm0 = CalculateNormal(edges0, pixPos[1][1], pixPos[0][1], pixPos[2][1], pixPos[1][0], pixPos[1][2]);
    float3 norm1 = CalculateNormal(edges1, pixPos[2][1], pixPos[1][1], pixPos[3][1], pixPos[2][0], pixPos[2][2]);
    float3 norm2 = CalculateNormal(edges2, pixPos[1][2], pixPos[0][2], pixPos[2][2], pixPos[1][1], pixPos[1][3]);
    float3 norm3 = CalculateNormal(edges3, pixPos[2][2], pixPos[1][2], pixPos[3][2], pixPos[2][1], pixPos[2][3]);

    g_NormalsOutputUAV[baseCoords + int2(0, 0)] = float4(norm0 * 0.5 + 0.5, 0.0);
    g_NormalsOutputUAV[baseCoords + int2(1, 0)] = float4(norm1 * 0.5 + 0.5, 0.0);
    g_NormalsOutputUAV[baseCoords + int2(0, 1)] = float4(norm2 * 0.5 + 0.5, 0.0);
    g_NormalsOutputUAV[baseCoords + int2(1, 1)] = float4(norm3 * 0.5 + 0.5, 0.0);
}


[numthreads(SSAO_THREAD_GROUP_SIZE_X, SSAO_THREAD_GROUP_SIZE_Y, 1)]
void CSPrepareDepthsAndNormalsHalf(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 halfViewPortSize = g_ASSAOConsts.ViewportHalfSize;
    if ((dispatchThreadID.x >= halfViewPortSize.x) ||
		(dispatchThreadID.y >= halfViewPortSize.y))
    {
        return;
    }

    int2 baseCoords = (dispatchThreadID.xy) * 2;
    float2 upperLeftUV = (float2(dispatchThreadID.xy) - 0.25) * g_ASSAOConsts.Viewport2xPixelSize;

    int3 baseCoord = int3(dispatchThreadID.xy * 2, 0);
    float z0 = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(0, 0)).x);
    float z1 = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(1, 0)).x);
    float z2 = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(0, 1)).x);
    float z3 = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(1, 1)).x);

    g_DepthOutputUAV0[dispatchThreadID.xy] = z0;
    g_DepthOutputUAV1[dispatchThreadID.xy] = z3;

    float pixZs[4][4];

	// middle 4
    pixZs[1][1] = z0;
    pixZs[2][1] = z1;
    pixZs[1][2] = z2;
    pixZs[2][2] = z3;
	// left 2
    pixZs[0][1] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(-1, 0)).x);
    pixZs[0][2] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(-1, 1)).x);
	// right 2	  																		
    pixZs[3][1] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(2, 0)).x);
    pixZs[3][2] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(2, 1)).x);
	// top 2
    pixZs[1][0] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(0, -1)).x);
    pixZs[2][0] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(1, -1)).x);
	// bottom 2	  																		
    pixZs[1][3] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(0, 2)).x);
    pixZs[2][3] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(1, 2)).x);

    float4 edges0 = CalculateEdges(pixZs[1][1], pixZs[0][1], pixZs[2][1], pixZs[1][0], pixZs[1][2]);
    float4 edges1 = CalculateEdges(pixZs[2][1], pixZs[1][1], pixZs[3][1], pixZs[2][0], pixZs[2][2]);
    float4 edges2 = CalculateEdges(pixZs[1][2], pixZs[0][2], pixZs[2][2], pixZs[1][1], pixZs[1][3]);
    float4 edges3 = CalculateEdges(pixZs[2][2], pixZs[1][2], pixZs[3][2], pixZs[2][1], pixZs[2][3]);

    float3 pixPos[4][4];

	// there is probably a way to optimize the math below; however no approximation will work, has to be precise.

	// middle 4
    pixPos[1][1] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(0.0, 0.0), pixZs[1][1]);
    pixPos[2][1] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(1.0, 0.0), pixZs[2][1]);
    pixPos[1][2] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(0.0, 1.0), pixZs[1][2]);
    pixPos[2][2] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(1.0, 1.0), pixZs[2][2]);
	// left 2
    pixPos[0][1] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(-1.0, 0.0), pixZs[0][1]);
	// right 2                                                                                     
	pixPos[3][2] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(2.0, 1.0), pixZs[3][2]);
	// top 2                                                                                       
    pixPos[1][0] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(0.0, -1.0), pixZs[1][0]);
	// bottom 2                                                                                    
	pixPos[2][3] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(1.0, 2.0), pixZs[2][3]);

    float3 norm0 = CalculateNormal(edges0, pixPos[1][1], pixPos[0][1], pixPos[2][1], pixPos[1][0], pixPos[1][2]);
    float3 norm3 = CalculateNormal(edges3, pixPos[2][2], pixPos[1][2], pixPos[3][2], pixPos[2][1], pixPos[2][3]);

    g_NormalsOutputUAV[baseCoords + int2(0, 0)] = float4(norm0 * 0.5 + 0.5, 0.0);
    g_NormalsOutputUAV[baseCoords + int2(1, 1)] = float4(norm3 * 0.5 + 0.5, 0.0);
}

[numthreads(SSAO_THREAD_GROUP_SIZE_X, SSAO_THREAD_GROUP_SIZE_Y, 1)]
void CSPrepareDepthMip1(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 halfViewPortSize = g_ASSAOConsts.ViewportHalfSize;
    if ((dispatchThreadID.x >= halfViewPortSize.x / 2) ||
		(dispatchThreadID.y >= halfViewPortSize.y / 2))
    {
        return;
    }

    float out0, out1, out2, out3;
    CSPrepareDepthMip(float2(dispatchThreadID.xy) , 1, out0, out1, out2, out3);
	g_DepthOutputUAVArray[uint3(dispatchThreadID.xy, 0)] = out0;
	g_DepthOutputUAVArray[uint3(dispatchThreadID.xy, 1)] = out1;
	g_DepthOutputUAVArray[uint3(dispatchThreadID.xy, 2)] = out2;
	g_DepthOutputUAVArray[uint3(dispatchThreadID.xy, 3)] = out3;
}

[numthreads(SSAO_THREAD_GROUP_SIZE_X, SSAO_THREAD_GROUP_SIZE_Y, 1)]
void CSPrepareDepthMip2(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 halfViewPortSize = g_ASSAOConsts.ViewportHalfSize;
    if ((dispatchThreadID.x >= halfViewPortSize.x / 4) ||
		(dispatchThreadID.y >= halfViewPortSize.y / 4))
    {
        return;
    }

    float out0, out1, out2, out3;
    CSPrepareDepthMip(float2(dispatchThreadID.xy) , 2, out0, out1, out2, out3);
	g_DepthOutputUAVArray[uint3(dispatchThreadID.xy, 0)] = out0;
	g_DepthOutputUAVArray[uint3(dispatchThreadID.xy, 1)] = out1;
	g_DepthOutputUAVArray[uint3(dispatchThreadID.xy, 2)] = out2;
	g_DepthOutputUAVArray[uint3(dispatchThreadID.xy, 3)] = out3;
}

[numthreads(SSAO_THREAD_GROUP_SIZE_X, SSAO_THREAD_GROUP_SIZE_Y, 1)]
void CSPrepareDepthMip3(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 halfViewPortSize = g_ASSAOConsts.ViewportHalfSize;
    if ((dispatchThreadID.x >= halfViewPortSize.x / 8) ||
		(dispatchThreadID.y >= halfViewPortSize.y / 8))
    {
        return;
    }

    float out0, out1, out2, out3;
    CSPrepareDepthMip(float2(dispatchThreadID.xy) , 3, out0, out1, out2, out3);
	g_DepthOutputUAVArray[uint3(dispatchThreadID.xy, 0)] = out0;
	g_DepthOutputUAVArray[uint3(dispatchThreadID.xy, 1)] = out1;
	g_DepthOutputUAVArray[uint3(dispatchThreadID.xy, 2)] = out2;
	g_DepthOutputUAVArray[uint3(dispatchThreadID.xy, 3)] = out3;
}

[numthreads(SSAO_THREAD_GROUP_SIZE_X, SSAO_THREAD_GROUP_SIZE_Y, 1)]
void CSGenerateLowestQualityMergedSingleTarget(lpuint3 dispatchThreadID : SV_DispatchThreadID)
{
    lpuint2 halfViewPortSize = (lpuint2)g_ASSAOConsts.ViewportHalfSize;
    if ((dispatchThreadID.x >= halfViewPortSize.x) ||
        (dispatchThreadID.y >= halfViewPortSize.y))
    {
        return;
    }

    // NOTE: Attempted to merged this and CSGenerateLowQualityMergedSingleTarget's code into GenerateSSAOShadowsInternalWrapper but that resulted in entirely
    //       separate code paths within that function just for these two functions which defeated the purpose of having a wrapper in the first place.
    float outShadowTerm = 0;
    float outWeight = 0;
    float outEdgesPacked = 0;
    const int qualityLevel = SSAO_QUALITY_LEVEL_LOWEST;
    const bool useArrayIndex = true;
    const bool isMerged = true;
    [unroll]
    for (uint i = 0; i < 2; ++i)
    {
        const uint arrayIndex = i * 3;                    
        float tempShadowTerm = 0.f;
        GenerateSSAOShadowsInternal(
            tempShadowTerm,
            outEdgesPacked,
            outWeight,
            (lpuint2) dispatchThreadID.xy,
            qualityLevel,
            useArrayIndex,
            arrayIndex,
            g_ViewspaceDepthSource,
            g_ViewspaceDepthSourceArray2);
        outShadowTerm += tempShadowTerm;
    }
    outShadowTerm *= 0.5f;
    float2 result = float2(outShadowTerm, PackEdges(float4(1, 1, 1, 1)));
    g_OutputUAVArray[uint3(dispatchThreadID.xy, 0)] = result;
}

[numthreads(SSAO_THREAD_GROUP_SIZE_X, SSAO_THREAD_GROUP_SIZE_Y, 1)]
void CSGenerateLowQualityMergedSingleTarget(lpuint3 dispatchThreadID : SV_DispatchThreadID)
{
    lpuint2 halfViewPortSize = (lpuint2)g_ASSAOConsts.ViewportHalfSize;
    if ((dispatchThreadID.x >= halfViewPortSize.x) ||
        (dispatchThreadID.y >= halfViewPortSize.y))
    {
        return;
    }

    // NOTE: Attempted to merged this and CSGenerateLowestQualityMergedSingleTarget's code into GenerateSSAOShadowsInternalWrapper but that resulted in entirely
    //       separate code paths within that function just for these two functions which defeated the purpose of having a wrapper in the first place.
    float outShadowTerm = 0;
    float outWeight = 0;
    float outEdgesPacked = 0;
    const int qualityLevel = SSAO_QUALITY_LEVEL_LOW;
    const bool useArrayIndex = true;
    const bool isMerged = true;
    [unroll]
    for (uint i = 0; i < MAX_PASS_COUNT; ++i)
    {
        const uint arrayIndex = i;
        float tempShadowTerm = 0.f;
        GenerateSSAOShadowsInternal(
            tempShadowTerm,
            outEdgesPacked,
            outWeight,
            (lpuint2) dispatchThreadID.xy,
            qualityLevel,
            useArrayIndex,
            arrayIndex,
            g_ViewspaceDepthSource,
            g_ViewspaceDepthSourceArray2);
        outShadowTerm += tempShadowTerm;
    }
    outShadowTerm /= MAX_PASS_COUNT;
    float2 result = float2(outShadowTerm, PackEdges(float4(1, 1, 1, 1)));
    g_OutputUAVArray[uint3(dispatchThreadID.xy, 0)] = result;
}

[numthreads(SSAO_THREAD_GROUP_SIZE_X, SSAO_THREAD_GROUP_SIZE_Y, 1)]
void CSGenerateMediumQualityMerged(lpuint3 dispatchThreadID : SV_DispatchThreadID)
{
    lpuint2 halfViewPortSize = (lpuint2) g_ASSAOConsts.ViewportHalfSize;
    if ((dispatchThreadID.x >= halfViewPortSize.x) ||
		(dispatchThreadID.y >= halfViewPortSize.y))
    {
        return;
    }

    bool useArrayIndex = true;
    int qualityLevel = SSAO_QUALITY_LEVEL_MEDIUM;
    const bool isMerged = true;
    uint arrayIndex = dispatchThreadID.z;
    g_OutputUAVArray[uint3(dispatchThreadID.xy, arrayIndex)] = GenerateSSAOShadowsInternalWrapper(dispatchThreadID, qualityLevel, isMerged, useArrayIndex, arrayIndex);
}

[numthreads(SSAO_THREAD_GROUP_SIZE_X, SSAO_THREAD_GROUP_SIZE_Y, 1)]
void CSGenerateHighQualityMerged(lpuint3 dispatchThreadID : SV_DispatchThreadID)
{
    lpuint2 halfViewPortSize = (lpuint2) g_ASSAOConsts.ViewportHalfSize;
    if ((dispatchThreadID.x >= halfViewPortSize.x) ||
		(dispatchThreadID.y >= halfViewPortSize.y))
    {
        return;
    }

    bool useArrayIndex = true;
    int qualityLevel = SSAO_QUALITY_LEVEL_HIGH;
    const bool isMerged = true;
    uint arrayIndex = dispatchThreadID.z;
    g_OutputUAVArray[uint3(dispatchThreadID.xy, arrayIndex)] = GenerateSSAOShadowsInternalWrapper(dispatchThreadID, qualityLevel, isMerged, useArrayIndex, arrayIndex);
}

[numthreads(SSAO_THREAD_GROUP_SIZE_X, SSAO_THREAD_GROUP_SIZE_Y, 1)]
void CSGenerateHighestQualityMerged(lpuint3 dispatchThreadID : SV_DispatchThreadID)
{
    lpuint2 halfViewPortSize = (lpuint2) g_ASSAOConsts.ViewportHalfSize;
    if ((dispatchThreadID.x >= halfViewPortSize.x) ||
		(dispatchThreadID.y >= halfViewPortSize.y))
    {
        return;
    }

    bool useArrayIndex = true;
    int qualityLevel = SSAO_QUALITY_LEVEL_HIGHEST;
    const bool isMerged = true;
    uint arrayIndex = dispatchThreadID.z;
    g_OutputUAVArray[uint3(dispatchThreadID.xy, arrayIndex)] = GenerateSSAOShadowsInternalWrapper(dispatchThreadID, qualityLevel, isMerged, useArrayIndex, arrayIndex);
}

[numthreads(SSAO_THREAD_GROUP_SIZE_X, SSAO_THREAD_GROUP_SIZE_Y, 1)]
void CSGenerateReferenceQualityMerged(lpuint3 dispatchThreadID : SV_DispatchThreadID)
{
    lpuint2 halfViewPortSize = (lpuint2) g_ASSAOConsts.ViewportHalfSize;
    if ((dispatchThreadID.x >= halfViewPortSize.x) ||
		(dispatchThreadID.y >= halfViewPortSize.y))
    {
        return;
    }

    bool useArrayIndex = true;
    int qualityLevel = SSAO_QUALITY_LEVEL_REFERENCE;
    float outShadowTerm;
	float outWeight;
	float outEdgesPacked;
    uint arrayIndex = dispatchThreadID.z;
    GenerateSSAOShadowsInternal(
        outShadowTerm,
        outEdgesPacked,
        outWeight,
        (lpuint2) dispatchThreadID.xy,
        qualityLevel,
        useArrayIndex,
        arrayIndex,
        g_ViewspaceDepthSource,
        g_ViewspaceDepthSourceArray2);
	float2 result = float2(outShadowTerm, outEdgesPacked);
	g_OutputUAVArray[uint3(dispatchThreadID.xy, arrayIndex)] = result;
}

// edge-sensitive blur
[numthreads(SSAO_THREAD_GROUP_SIZE_X, SSAO_THREAD_GROUP_SIZE_Y, 1)]
void CSSmartBlurFinal(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 halfViewPortSize = g_ASSAOConsts.ViewportHalfSize;
    if ((dispatchThreadID.x >= halfViewPortSize.x) ||
		(dispatchThreadID.y >= halfViewPortSize.y))
    {
        return;
    }
    float2 inUV = float2(dispatchThreadID.x, dispatchThreadID.y);
	// add half pixel offset
    inUV += float2(0.5f, 0.5f);


    inUV *= g_ASSAOConsts.HalfViewportPixelSize;

    float2 result = SampleBlurred(int2(dispatchThreadID.xy), inUV);
    g_OutputUAVArray[int3(dispatchThreadID.xy, 0)] = result;
}

[numthreads(SSAO_THREAD_GROUP_SIZE_X, SSAO_THREAD_GROUP_SIZE_Y, 1)]
void CSSmartBlur(uint3 dispatchThreadID : SV_DispatchThreadID)
{

    uint2 halfViewPortSize = g_ASSAOConsts.ViewportHalfSize;
    if ((dispatchThreadID.x >= halfViewPortSize.x) ||
		(dispatchThreadID.y >= halfViewPortSize.y))
    {
        return;
    }
    float2 inUV = float2(dispatchThreadID.x, dispatchThreadID.y);
	// add half pixel offset
    inUV += float2(0.5f, 0.5f);


    inUV *= g_ASSAOConsts.HalfViewportPixelSize;

    float2 result = SampleBlurred(int2(dispatchThreadID.xy), inUV);
    g_OutputUAV[dispatchThreadID.xy] = result;
}




// edge-sensitive blur (wider kernel)
[numthreads(SSAO_THREAD_GROUP_SIZE_X, SSAO_THREAD_GROUP_SIZE_Y, 1)]
void CSSmartBlurWideFinal(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 halfViewPortSize = g_ASSAOConsts.ViewportHalfSize;
    if ((dispatchThreadID.x >= halfViewPortSize.x) ||
		(dispatchThreadID.y >= halfViewPortSize.y))
    {
        return;
    }
    float2 inUV = float2(dispatchThreadID.x, dispatchThreadID.y);
	// add half pixel offset
    inUV += float2(0.5f, 0.5f);


    inUV *= g_ASSAOConsts.HalfViewportPixelSize;

    float2 result = SampleBlurredWide(int2(dispatchThreadID.xy), inUV);
    g_OutputUAVArray[int3(dispatchThreadID.xy, 0)] = result;
}

float2 CSSampleBlurredMerged(int3 inPos, float2 coord)
{
    float packedEdges = g_BlurInputArray.Load(int4(inPos.xyz, 0)).y;
    float4 edgesLRTB = UnpackEdges(packedEdges);

    float4 valuesBR = g_BlurInputArray.GatherRed(g_PointMirrorSampler, float3(coord + g_ASSAOConsts.HalfViewportPixelSize * 0.5, inPos.z));
    float4 valuesUL = g_BlurInputArray.GatherRed(g_PointMirrorSampler, float3(coord - g_ASSAOConsts.HalfViewportPixelSize * 0.5, inPos.z));

    float ssaoValue = valuesUL.y;
    float ssaoValueL = valuesUL.x;
    float ssaoValueT = valuesUL.z;
    float ssaoValueR = valuesBR.z;
    float ssaoValueB = valuesBR.x;

    float sumWeight = 0.5f;
    float sum = ssaoValue * sumWeight;

    AddSample(ssaoValueL, edgesLRTB.x, sum, sumWeight);
    AddSample(ssaoValueR, edgesLRTB.y, sum, sumWeight);

    AddSample(ssaoValueT, edgesLRTB.z, sum, sumWeight);
    AddSample(ssaoValueB, edgesLRTB.w, sum, sumWeight);

    float ssaoAvg = sum / sumWeight;

    ssaoValue = ssaoAvg; //min( ssaoValue, ssaoAvg ) * 0.2 + ssaoAvg * 0.8;

    return float2(ssaoValue, packedEdges);
}

// edge-sensitive blur
[numthreads(SSAO_THREAD_GROUP_SIZE_X, SSAO_THREAD_GROUP_SIZE_Y, 1)]
void CSSmartBlurMerged(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 viewPortSize = (uint2) g_ASSAOConsts.ViewportHalfSize;
    if ((dispatchThreadID.x >= viewPortSize.x) ||
		(dispatchThreadID.y >= viewPortSize.y))
    {
        return;
    }

    float2 inUV = float2(dispatchThreadID.x, dispatchThreadID.y);
	// add half pixel offset
    inUV += float2(0.5f, 0.5f);

    inUV *= g_ASSAOConsts.HalfViewportPixelSize;


	[unroll]
    for (int i = 0; i < MAX_PASS_COUNT; ++i)
    {
        float2 result = CSSampleBlurredMerged(int3(dispatchThreadID.xy, i), inUV);
        g_OutputUAVArray[uint3(dispatchThreadID.xy, i)] = result;
    }
}


float2 CSSampleBlurredWideMerged(int3 inPos, float2 coord)
{
    float2 vL = g_BlurInputArray.SampleLevel(g_PointMirrorSampler, float3(coord, inPos.z), 0.0, int2(-2, 0)).xy;
    float2 vT = g_BlurInputArray.SampleLevel(g_PointMirrorSampler, float3(coord, inPos.z), 0.0, int2(0, -2)).xy;
    float2 vR = g_BlurInputArray.SampleLevel(g_PointMirrorSampler, float3(coord, inPos.z), 0.0, int2(2, 0)).xy;
    float2 vB = g_BlurInputArray.SampleLevel(g_PointMirrorSampler, float3(coord, inPos.z), 0.0, int2(0, 2)).xy;
    float2 vC = g_BlurInputArray.SampleLevel(g_PointMirrorSampler, float3(coord, inPos.z), 0.0, int2(0, 0)).xy;

    float packedEdges = vC.y;
    float4 edgesLRTB = UnpackEdges(packedEdges);
    edgesLRTB.x *= UnpackEdges(vL.y).y;
    edgesLRTB.z *= UnpackEdges(vT.y).w;
    edgesLRTB.y *= UnpackEdges(vR.y).x;
    edgesLRTB.w *= UnpackEdges(vB.y).z;

	//this would be more mathematically correct but there's little difference at cost
	//edgesLRTB.x         = sqrt( edgesLRTB.x );
	//edgesLRTB.z         = sqrt( edgesLRTB.z );
	//edgesLRTB.y         = sqrt( edgesLRTB.y );
	//edgesLRTB.w         = sqrt( edgesLRTB.w );

    float ssaoValue = vC.x;
    float ssaoValueL = vL.x;
    float ssaoValueT = vT.x;
    float ssaoValueR = vR.x;
    float ssaoValueB = vB.x;

    float sumWeight = 0.8f;
    float sum = ssaoValue * sumWeight;

    AddSample(ssaoValueL, edgesLRTB.x, sum, sumWeight);
    AddSample(ssaoValueR, edgesLRTB.y, sum, sumWeight);
    AddSample(ssaoValueT, edgesLRTB.z, sum, sumWeight);
    AddSample(ssaoValueB, edgesLRTB.w, sum, sumWeight);

    float ssaoAvg = sum / sumWeight;

    ssaoValue = ssaoAvg; //min( ssaoValue, ssaoAvg ) * 0.2 + ssaoAvg * 0.8;

    return float2(ssaoValue, packedEdges);

}

// edge-sensitive blur (wider kernel)
[numthreads(SSAO_THREAD_GROUP_SIZE_X, SSAO_THREAD_GROUP_SIZE_Y, 1)]
void CSSmartBlurWide(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 halfViewPortSize = g_ASSAOConsts.ViewportHalfSize;
    if ((dispatchThreadID.x >= halfViewPortSize.x) ||
		(dispatchThreadID.y >= halfViewPortSize.y))
    {
        return;
    }

    float2 inUV = float2(dispatchThreadID.x, dispatchThreadID.y);
	// add half pixel offset
    inUV += float2(0.5f, 0.5f);

    inUV *= g_ASSAOConsts.HalfViewportPixelSize;

    float2 result = SampleBlurredWide(int2(dispatchThreadID.xy), inUV);
    g_OutputUAV[dispatchThreadID.xy] = result;
}

// edge-sensitive blur (wider kernel)
[numthreads(SSAO_THREAD_GROUP_SIZE_X, SSAO_THREAD_GROUP_SIZE_Y, 1)]
void CSSmartBlurWideMerged(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 viewPortSize = (uint2) g_ASSAOConsts.ViewportHalfSize;
    if ((dispatchThreadID.x >= viewPortSize.x) ||
		(dispatchThreadID.y >= viewPortSize.y))
    {
        return;
    }

    float2 inUV = float2(dispatchThreadID.x, dispatchThreadID.y);
	// add half pixel offset
    inUV += float2(0.5f, 0.5f);

    inUV *= g_ASSAOConsts.HalfViewportPixelSize;

	[unroll]
    for (int i = 0; i < MAX_PASS_COUNT; ++i)
    {
        float2 result = CSSampleBlurredWideMerged(int3(dispatchThreadID.xy, i), inUV);
        g_OutputUAVArray[uint3(dispatchThreadID.xy, i)] = result;
    }

}


// edge-ignorant blur in x and y directions, 9 pixels touched (for the lowest quality level 0)
[numthreads(SSAO_THREAD_GROUP_SIZE_X, SSAO_THREAD_GROUP_SIZE_Y, 1)]
void CSNonSmartBlur(uint3 DTid : SV_DispatchThreadID)
{
    uint2 halfViewPortSize = g_ASSAOConsts.ViewportHalfSize;
    if ((DTid.x >= halfViewPortSize.x) ||
		(DTid.y >= halfViewPortSize.y))
    {
        return;
    }
    float2 inUV = float2(DTid.x, DTid.y);
	// add half pixel offset
    inUV += float2(0.5f, 0.5f);


    inUV *= g_ASSAOConsts.HalfViewportPixelSize;
    float2 halfPixel = g_ASSAOConsts.HalfViewportPixelSize * 0.5f;

    float2 centre = g_BlurInput.SampleLevel(g_LinearClampSampler, inUV, 0.0).xy;

    float4 vals;
    vals.x = g_BlurInput.SampleLevel(g_LinearClampSampler, inUV + float2(-halfPixel.x * 3, -halfPixel.y), 0.0).x;
    vals.y = g_BlurInput.SampleLevel(g_LinearClampSampler, inUV + float2(+halfPixel.x, -halfPixel.y * 3), 0.0).x;
    vals.z = g_BlurInput.SampleLevel(g_LinearClampSampler, inUV + float2(-halfPixel.x, +halfPixel.y * 3), 0.0).x;
    vals.w = g_BlurInput.SampleLevel(g_LinearClampSampler, inUV + float2(+halfPixel.x * 3, +halfPixel.y), 0.0).x;
    float2 result = float2(dot(vals, 0.2.xxxx) + centre.x * 0.2, centre.y);
    g_OutputUAVArray[int3(DTid.xy, 0)] = result;
}

// edge-ignorant blur in x and y directions, 9 pixels touched (for the lowest quality level 0)
[numthreads(SSAO_THREAD_GROUP_SIZE_X, SSAO_THREAD_GROUP_SIZE_Y, 1)]
void CSNonSmartBlurMerged(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 viewPortSize = (uint2) g_ASSAOConsts.ViewportHalfSize;
    if ((dispatchThreadID.x >= viewPortSize.x) ||
		(dispatchThreadID.y >= viewPortSize.y))
    {
        return;
    }

    float2 inUV = float2(dispatchThreadID.x, dispatchThreadID.y);
	// add half pixel offset
    inUV += float2(0.5f, 0.5f);

    inUV *= g_ASSAOConsts.HalfViewportPixelSize;
    float2 halfPixel = g_ASSAOConsts.HalfViewportPixelSize * 0.5f;

    float4 vals;
    lpuint multiplier = 1;
    if (g_ASSAOConsts.QualityLevel == SSAO_QUALITY_LEVEL_LOWEST)
        multiplier = 3;
	[unroll]
    for (uint i = 0; i < MAX_PASS_COUNT; ++i)
    {
        i *= multiplier;
        float2 centre = g_BlurInputArray.SampleLevel(g_LinearClampSampler, float3(inUV, i), 0.0).xy;
        vals.y = g_BlurInputArray.SampleLevel(g_LinearClampSampler, float3(inUV + float2(+halfPixel.x, -halfPixel.y * 3), i), 0.0).x;
        vals.z = g_BlurInputArray.SampleLevel(g_LinearClampSampler, float3(inUV + float2(-halfPixel.x, +halfPixel.y * 3), i), 0.0).x;
        vals.w = g_BlurInputArray.SampleLevel(g_LinearClampSampler, float3(inUV + float2(+halfPixel.x * 3, +halfPixel.y), i), 0.0).x;
        vals.x = g_BlurInputArray.SampleLevel(g_LinearClampSampler, float3(inUV + float2(-halfPixel.x * 3, -halfPixel.y), i), 0.0).x;

        float2 result = float2(dot(vals, 0.2.xxxx) + centre.x * 0.2, centre.y);
        g_OutputUAVArray[int3(dispatchThreadID.xy, i)] = result;
    }
}

// edge-ignorant blur in x and y directions, 9 pixels touched (for the lowest quality level 0)
[numthreads(SSAO_THREAD_GROUP_SIZE_X, SSAO_THREAD_GROUP_SIZE_Y, 1)]
void CSNonSmartBlurMergedSingleTarget(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 viewPortSize = (uint2) g_ASSAOConsts.ViewportHalfSize;
    if ((dispatchThreadID.x >= viewPortSize.x) ||
		(dispatchThreadID.y >= viewPortSize.y))
    {
        return;
    }

    float2 inUV = float2(dispatchThreadID.x, dispatchThreadID.y);
	// add half pixel offset
    inUV += float2(0.5f, 0.5f);

    inUV *= g_ASSAOConsts.HalfViewportPixelSize;
    float2 halfPixel = g_ASSAOConsts.HalfViewportPixelSize * 0.5f;

    float4 vals;
 //   lpuint multiplier = 1;
 //   if (g_ASSAOConsts.QualityLevel == SSAO_QUALITY_LEVEL_LOWEST)
 //       multiplier = 3;
	//[unroll]
 //   for (uint i = 0; i < MAX_PASS_COUNT; ++i)
    uint i = 0;
    {
        //i *= multiplier;
        float2 centre = g_BlurInputArray.SampleLevel(g_LinearClampSampler, float3(inUV, i), 0.0).xy;
        vals.y = g_BlurInputArray.SampleLevel(g_LinearClampSampler, float3(inUV + float2(+halfPixel.x, -halfPixel.y * 3), i), 0.0).x;
        vals.z = g_BlurInputArray.SampleLevel(g_LinearClampSampler, float3(inUV + float2(-halfPixel.x, +halfPixel.y * 3), i), 0.0).x;
        vals.w = g_BlurInputArray.SampleLevel(g_LinearClampSampler, float3(inUV + float2(+halfPixel.x * 3, +halfPixel.y), i), 0.0).x;
        vals.x = g_BlurInputArray.SampleLevel(g_LinearClampSampler, float3(inUV + float2(-halfPixel.x * 3, -halfPixel.y), i), 0.0).x;

        float2 result = float2(dot(vals, 0.2.xxxx) + centre.x * 0.2, centre.y);
        g_OutputUAVArray[int3(dispatchThreadID.xy, i)] = result;
    }
}

#ifdef SSAO_PREPAREDEPTHSMERGED_ONLY

RWTexture2DArray<float> g_DepthOutputUAVArrayMip0 : register(u0);
RWTexture2DArray<float> g_DepthOutputUAVArrayMip1 : register(u1);
RWTexture2DArray<float> g_DepthOutputUAVArrayMip2 : register(u2);
RWTexture2DArray<float> g_DepthOutputUAVArrayMip3 : register(u3);

float PrepareDepthMipMergeSlice(float4 values)
{
    float dummyUnused1;
    float dummyUnused2;
    float falloffCalcMulSq, falloffCalcAdd;


    float4 depths = values;

    float closest = min(min(depths.x, depths.y), min(depths.z, depths.w));

    CalculateRadiusParameters(abs(closest), 1.0, dummyUnused1, dummyUnused2, falloffCalcMulSq);

    float4 dists = depths - closest.xxxx;

    float4 weights = saturate(dists * dists * falloffCalcMulSq + 1.);

    float smartAvg = dot(weights, depths) / dot(weights, float4(1.0, 1.0, 1.0, 1.0));

    return smartAvg;
}

groupshared float4 group_shared_memory[SSAO_THREAD_GROUP_SIZE_X * SSAO_THREAD_GROUP_SIZE_Y];

uint GroupSharedMemoryIndex(const uint x, const uint y)
{
    // Column-major 2x2 GSM. Orders data such that each 2x2 is contiguous and the 2x2s are stored in column-major order.
    uint2 lid = uint2(x % 2, y % 2);
    uint subIndex = lid.y * 2 + lid.x;
    uint subOffset = ((y >> 1) + (x >> 1) * 4) * 4;
    return (subOffset + subIndex);
}

void ComputeQuad(const uint x, const uint y, out uint2 quadPts[4])
{
    // src mip x,y coordinates.
    const uint
        slx = x * 2, srx = slx + 1,
        suy = y * 2, sby = suy + 1;

    // src mip quad coords.
    quadPts[0] = uint2(slx, suy);
    quadPts[1] = uint2(srx, suy);
    quadPts[2] = uint2(slx, sby);
    quadPts[3] = uint2(srx, sby);
}

[numthreads(SSAO_THREAD_GROUP_SIZE_X, SSAO_THREAD_GROUP_SIZE_Y, 1)]
void CSPrepareDepthsMerged(uint GI : SV_GroupIndex, uint3 localThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Mip 0 quad's coordinates and data.
    // srcMipData is arrayed and mipQuadPts isn't (4 uint2's make 1 quad) because each quad in mip0 will have the same coordinates across array levels but different data.
    uint2 mipQuadPts[4];
    float4x4 srcMipData;

    // Compute the mip1 quad's coordinates.
    ComputeQuad(dispatchThreadID.x, dispatchThreadID.y, mipQuadPts);

    // Compute the coordinates and data for the mip 0 quads.
    uint2 srcDepthQuadPts[4];
    [unroll]
    for (uint i = 0; i < 4; ++i)
    {
        const uint2 currentSrc = mipQuadPts[i];
        // Src depth quad's coordinates and data.
        ComputeQuad(currentSrc.x, currentSrc.y, srcDepthQuadPts);

        // Load srcDepthData and store in the i'th pixel of the current quad for each array level of mip 0.
        // Ensure that the coordinates are inside of the respective mip's bounds.
        srcMipData[0][i] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(uint3(srcDepthQuadPts[0], 0)).x);
        srcMipData[1][i] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(uint3(srcDepthQuadPts[1], 0)).x);
        srcMipData[2][i] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(uint3(srcDepthQuadPts[2], 0)).x);
        srcMipData[3][i] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(uint3(srcDepthQuadPts[3], 0)).x);

        g_DepthOutputUAVArrayMip0[uint3(currentSrc, 0)] = srcMipData[0][i];
        g_DepthOutputUAVArrayMip0[uint3(currentSrc, 1)] = srcMipData[1][i];
        g_DepthOutputUAVArrayMip0[uint3(currentSrc, 2)] = srcMipData[2][i];
        g_DepthOutputUAVArrayMip0[uint3(currentSrc, 3)] = srcMipData[3][i];

#if SSAO_PREPAREDEPTHSMERGED_GENERATE_NORMALS
        float pixZs[4][4];
        pixZs[1][1] = srcMipData[0][i];
        pixZs[2][1] = srcMipData[1][i];
        pixZs[1][2] = srcMipData[2][i];
        pixZs[2][2] = srcMipData[3][i];
        
        int2 baseCoords = (int2)srcDepthQuadPts[0];
        int3 baseCoord = int3(baseCoords, 0);
	    // left 2
        pixZs[0][1] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(-1, 0)).x);
        pixZs[0][2] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(-1, 1)).x);
	    // right 2	  																		
        pixZs[3][1] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(2, 0)).x);
        pixZs[3][2] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(2, 1)).x);
	    // top 2
        pixZs[1][0] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(0, -1)).x);
        pixZs[2][0] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(1, -1)).x);
	    // bottom 2	  																		
        pixZs[1][3] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(0, 2)).x);
        pixZs[2][3] = ScreenSpaceToViewSpaceDepth(g_DepthSource.Load(baseCoord, int2(1, 2)).x);

        float4 edges0 = CalculateEdges(pixZs[1][1], pixZs[0][1], pixZs[2][1], pixZs[1][0], pixZs[1][2]);
        float4 edges1 = CalculateEdges(pixZs[2][1], pixZs[1][1], pixZs[3][1], pixZs[2][0], pixZs[2][2]);
        float4 edges2 = CalculateEdges(pixZs[1][2], pixZs[0][2], pixZs[2][2], pixZs[1][1], pixZs[1][3]);
        float4 edges3 = CalculateEdges(pixZs[2][2], pixZs[1][2], pixZs[3][2], pixZs[2][1], pixZs[2][3]);

        float3 pixPos[4][4];
        float2 upperLeftUV = (float2(currentSrc.xy) - 0.25) * g_ASSAOConsts.Viewport2xPixelSize;

	    // there is probably a way to optimize the math below; however no approximation will work, has to be precise.

	    // middle 4
        pixPos[1][1] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(0.0, 0.0), pixZs[1][1]);
        pixPos[2][1] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(1.0, 0.0), pixZs[2][1]);
        pixPos[1][2] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(0.0, 1.0), pixZs[1][2]);
        pixPos[2][2] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(1.0, 1.0), pixZs[2][2]);
	    // left 2
        pixPos[0][1] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(-1.0, 0.0), pixZs[0][1]);
        pixPos[0][2] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(-1.0, 1.0), pixZs[0][2]);
	    // right 2                                                                                     
        pixPos[3][1] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(2.0, 0.0), pixZs[3][1]);
        pixPos[3][2] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(2.0, 1.0), pixZs[3][2]);
	    // top 2                                                                                       
        pixPos[1][0] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(0.0, -1.0), pixZs[1][0]);
        pixPos[2][0] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(1.0, -1.0), pixZs[2][0]);
	    // bottom 2                                                                                    
        pixPos[1][3] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(0.0, 2.0), pixZs[1][3]);
        pixPos[2][3] = NDCToViewspace(upperLeftUV + g_ASSAOConsts.ViewportPixelSize * float2(1.0, 2.0), pixZs[2][3]);

        float3 norm0 = CalculateNormal(edges0, pixPos[1][1], pixPos[0][1], pixPos[2][1], pixPos[1][0], pixPos[1][2]);
        float3 norm1 = CalculateNormal(edges1, pixPos[2][1], pixPos[1][1], pixPos[3][1], pixPos[2][0], pixPos[2][2]);
        float3 norm2 = CalculateNormal(edges2, pixPos[1][2], pixPos[0][2], pixPos[2][2], pixPos[1][1], pixPos[1][3]);
        float3 norm3 = CalculateNormal(edges3, pixPos[2][2], pixPos[1][2], pixPos[3][2], pixPos[2][1], pixPos[2][3]);

        g_NormalsOutputUAV[baseCoords + int2(0, 0)] = float4(norm0 * 0.5 + 0.5, 0.0);
        g_NormalsOutputUAV[baseCoords + int2(1, 0)] = float4(norm1 * 0.5 + 0.5, 0.0);
        g_NormalsOutputUAV[baseCoords + int2(0, 1)] = float4(norm2 * 0.5 + 0.5, 0.0);
        g_NormalsOutputUAV[baseCoords + int2(1, 1)] = float4(norm3 * 0.5 + 0.5, 0.0);
#endif
    }

    // Compute the current pixel value for each array level in mip 1.
    float4 dstMipData;
    [unroll]
    for (uint j = 0; j < 4; ++j)
    {
        dstMipData[j] = PrepareDepthMipMergeSlice(srcMipData[j]);
        g_DepthOutputUAVArrayMip1[uint3(dispatchThreadID.xy, j)] = dstMipData[j];
    }

    // Store the current pixel value for each array level in mip 1.
    uint groupSharedMemoryIndex = GroupSharedMemoryIndex(localThreadID.x, localThreadID.y);
    group_shared_memory[groupSharedMemoryIndex] = dstMipData;

    // Sync the group shared memory (GSM).
    GroupMemoryBarrierWithGroupSync();

    // Proceed only if localThreadID.xy is even.
    bool continueThread = false;
    [branch]
    if (
        ((localThreadID.x & 1) == 0) &&
        ((localThreadID.y & 1) == 0)
        )
    {
        continueThread = true;
        localThreadID.xy = localThreadID.xy >> 1;
        dispatchThreadID.xy = dispatchThreadID.xy >> 1;

        // Compute the quad in GSM that will act as the source for the current pixel in mip 2.
        ComputeQuad(localThreadID.x, localThreadID.y, mipQuadPts);

        // Load group_shared_memory[mipQuadPts] into srcMipData and transpose it so that the data for each array level is arranged row-wise.
        srcMipData[0] = group_shared_memory[GroupSharedMemoryIndex(mipQuadPts[0].x, mipQuadPts[0].y)];
        srcMipData[1] = group_shared_memory[GroupSharedMemoryIndex(mipQuadPts[1].x, mipQuadPts[1].y)];
        srcMipData[2] = group_shared_memory[GroupSharedMemoryIndex(mipQuadPts[2].x, mipQuadPts[2].y)];
        srcMipData[3] = group_shared_memory[GroupSharedMemoryIndex(mipQuadPts[3].x, mipQuadPts[3].y)];
        srcMipData = transpose(srcMipData);

        [unroll]
        for (uint k = 0; k < 4; ++k)
        {
            dstMipData[k] = PrepareDepthMipMergeSlice(srcMipData[k]);
            g_DepthOutputUAVArrayMip2[uint3(dispatchThreadID.xy, k)] = dstMipData[k];
        }

        // Store the current pixel's mip 2 results in SLM.
        groupSharedMemoryIndex = GroupSharedMemoryIndex(localThreadID.x, localThreadID.y);
        group_shared_memory[groupSharedMemoryIndex] = dstMipData;
    }

    // Sync the group shared memory (GSM).
    GroupMemoryBarrierWithGroupSync();

    // Proceed only if this thread participated in the previous pass and localThreadID.xy is even.
    [branch]
    if (
        continueThread &&
        ((localThreadID.x & 1) == 0) &&
        ((localThreadID.y & 1) == 0)
        )
    {
        localThreadID.xy = localThreadID.xy >> 1;
        dispatchThreadID.xy = dispatchThreadID.xy >> 1;

        // Compute the quad in GSM that will act as the source for the current pixel in mip 3.
        ComputeQuad(localThreadID.x, localThreadID.y, mipQuadPts);

        // Load group_shared_memory[mipQuadPts] into srcMipData and transpose it so that the data for each array level is arranged row-wise.
        srcMipData[0] = group_shared_memory[GroupSharedMemoryIndex(mipQuadPts[0].x, mipQuadPts[0].y)];
        srcMipData[1] = group_shared_memory[GroupSharedMemoryIndex(mipQuadPts[1].x, mipQuadPts[1].y)];
        srcMipData[2] = group_shared_memory[GroupSharedMemoryIndex(mipQuadPts[2].x, mipQuadPts[2].y)];
        srcMipData[3] = group_shared_memory[GroupSharedMemoryIndex(mipQuadPts[3].x, mipQuadPts[3].y)];
        srcMipData = transpose(srcMipData);

        [unroll]
        for (uint l = 0; l < 4; ++l)
        {
            g_DepthOutputUAVArrayMip3[uint3(dispatchThreadID.xy, l)] = PrepareDepthMipMergeSlice(srcMipData[l]);
        }
    }
}

#endif // #ifdef SSAO_PREPAREDEPTHSMERGED_ONLY