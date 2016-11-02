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

#include "vaShared.hlsl"

#include "vaSimpleShadowMap.hlsl"

//#include "IntelExtensions.hlsl"


struct GenericBillboardSpriteVertexTransformed
{
    float4 Position             : SV_Position;
    float4 Color                : COLOR;
    float4 Texcoord0            : TEXCOORD0;            // .z == 0 means run AVSM 'rebalancing' pass
    float4 ViewspacePos         : TEXCOORD1;
};

Texture2D           g_textureColor              : register(t0);
// Texture2D           g_textureNormal             : register(t1);
// Texture2D           g_textureHeight             : register(t2);


GenericBillboardSpriteVertex SimpleParticleVS( const in GenericBillboardSpriteVertex input )
{
    return input;
}

[maxvertexcount(8)]
void SimpleParticleGS( point GenericBillboardSpriteVertex inputArr[1],  inout TriangleStream<GenericBillboardSpriteVertexTransformed> triStream, in uint primitiveID : SV_PrimitiveID )
{
    GenericBillboardSpriteVertex input = inputArr[0];

    float3 viewspacePos = mul( g_ParticleSystemConstants.WorldView, float4( input.Position_CreationID.xyz, 1) ).xyz;

    // this clipping is handy for performance reasons but a gradual fadeout when getting closer to clipspace is probably a good idea
    if( viewspacePos.z < 0.0 )
        return;

    uint uintCreationID  = asuint(input.Position_CreationID.w);
    uint tempInt = uintCreationID ^ ( ( uintCreationID ^ 0x85ebca6b ) >> 13 ) * 0xc2b2ae3;
    float perInstanceNoise = frac( tempInt / 4096.0 );

    float3 viewDir      = normalize( viewspacePos );

    float3 dirRight     = float3( 1.0f, 0.0f, 0.0f );
    float3 dirBottom    = float3( 0.0f, 1.0f, 0.0f );

    float2x2 rotScale   = float2x2( input.Transform2D.x, input.Transform2D.y, input.Transform2D.z, input.Transform2D.w );

#ifdef VA_VOLUME_SHADOWS_PLUGIN_USE
#ifdef SHADOWS_GENERATE
    const float sizeScale = (uint(input.Position_CreationID.w) % g_SimpleAVSMGlobal.SkipFactor) == 0;
    rotScale[0][0] *= sizeScale;
    rotScale[1][0] *= sizeScale;
    rotScale[0][1] *= sizeScale;
    rotScale[1][1] *= sizeScale;
#endif
#endif

    // rotate & scale in viewspace
    dirRight.xy         = mul( rotScale, dirRight.xy );
    dirBottom.xy        = mul( rotScale, dirBottom.xy );


    GenericBillboardSpriteVertexTransformed spriteTris[4];

    float size = length( dirRight + dirBottom );

    spriteTris[0].ViewspacePos  = float4( viewspacePos.xyz - dirRight - dirBottom, size );
    spriteTris[1].ViewspacePos  = float4( viewspacePos.xyz + dirRight - dirBottom, size );
    spriteTris[2].ViewspacePos  = float4( viewspacePos.xyz - dirRight + dirBottom, size );
    spriteTris[3].ViewspacePos  = float4( viewspacePos.xyz + dirRight + dirBottom, size );
 
    spriteTris[0].Position      = mul( g_ParticleSystemConstants.Proj, float4( spriteTris[0].ViewspacePos.xyz, 1.0 ) );
    spriteTris[1].Position      = mul( g_ParticleSystemConstants.Proj, float4( spriteTris[1].ViewspacePos.xyz, 1.0 ) );
    spriteTris[2].Position      = mul( g_ParticleSystemConstants.Proj, float4( spriteTris[2].ViewspacePos.xyz, 1.0 ) );
    spriteTris[3].Position      = mul( g_ParticleSystemConstants.Proj, float4( spriteTris[3].ViewspacePos.xyz, 1.0 ) );

    // ************************************************************************************************************************************
    // we can do culling here - just create bounding box from above positions.xyz/positions.w and see if it intersect clipping cube
    // ************************************************************************************************************************************

    // .z used for force rebalance flag
    spriteTris[0].Texcoord0     = float4( 0.0, 0.0, 0.0, perInstanceNoise );
    spriteTris[1].Texcoord0     = float4( 1.0, 0.0, 0.0, perInstanceNoise );
    spriteTris[2].Texcoord0     = float4( 0.0, 1.0, 0.0, perInstanceNoise );
    spriteTris[3].Texcoord0     = float4( 1.0, 1.0, 0.0, perInstanceNoise );

    spriteTris[0].Color         = input.Color;
    spriteTris[1].Color         = input.Color;
    spriteTris[2].Color         = input.Color;
    spriteTris[3].Color         = input.Color;

    for( int i = 0; i < 4; i++ )
        triStream.Append( spriteTris[i] );

    triStream.RestartStrip( );

#ifndef AVSM_ENABLE_CACHE_FLUSH_SIMD_SYNC

#ifdef VA_VOLUME_SHADOWS_PLUGIN_USE
#ifdef SHADOWS_GENERATE
#ifdef VA_VOLUME_SHADOWS_PLUGIN_REQUIRES_FULLSCREEN_REBALANCE_PASS

#define MINI_QUAD_UPDATE

//    1.) [DONE] remove g_AVSMRebalancingInfoUAV and related stuff
//    2.) move this to VolumeShadowGetRebalancingQuad (and rename appropriately? rebalance? cache flush?)
//    3.) ...
//    4.) $$$?

#ifdef MINI_QUAD_UPDATE
    // update frequency determined empirically
    const uint miniBlockUpdateFrequency = AVSM_NODE_CACHE_SIZE * 25 / 4;
    const uint  miniBlockSize   = 32;
    const int   borderExpand    = 1;
    float2 topLeftClipspace, botRightClipspace;
    [branch]
    //if( (primitiveID % miniBlockUpdateFrequency) == 0 )
    if( (uintCreationID % miniBlockUpdateFrequency) == 0 )
    {
      // trigger rebalance flag
      spriteTris[0].Texcoord0.z = 1.0;
      spriteTris[1].Texcoord0.z = 1.0;
      spriteTris[2].Texcoord0.z = 1.0;
      spriteTris[3].Texcoord0.z = 1.0;

      float2 quarterPixelOffset = 0.25 * float2( 2.0 / AVSM_RESOLUTION, -2.0 / AVSM_RESOLUTION );
      float2 blockSize          = float2( 2.0 * miniBlockSize / AVSM_RESOLUTION, 2.0 * miniBlockSize / AVSM_RESOLUTION );
    
      float2 normalizedPos0 = spriteTris[0].Position.xy / spriteTris[0].Position.w;
      float2 normalizedPos1 = spriteTris[1].Position.xy / spriteTris[1].Position.w;
      float2 normalizedPos2 = spriteTris[2].Position.xy / spriteTris[2].Position.w;
      float2 normalizedPos3 = spriteTris[3].Position.xy / spriteTris[3].Position.w;

      float2 minXY = min( min( normalizedPos0, normalizedPos1 ), min( normalizedPos2, normalizedPos3 ) );
      float2 maxXY = max( max( normalizedPos0, normalizedPos1 ), max( normalizedPos2, normalizedPos3 ) );

      // snap min to smaller mod of borderExpand, and snap max to larger mod of borderExpand
      minXY = float2( int2( minXY / blockSize ) - (minXY < 0) - borderExpand    ) * blockSize;
      maxXY = float2( int2( maxXY / blockSize ) + (maxXY > 0) + borderExpand    ) * blockSize;

      // quarterPixelOffset is used just to avoid any precision issues by putting the output quad right in the middle between edge and pixel centre;
      topLeftClipspace          = float2( minXY.x, maxXY.y ) + quarterPixelOffset;
      botRightClipspace         = float2( maxXY.x, minXY.y ) - quarterPixelOffset;

      spriteTris[0].Position    = float4( topLeftClipspace.x,  topLeftClipspace.y,  0.99, 1.0 );
      spriteTris[1].Position    = float4( botRightClipspace.x, topLeftClipspace.y,  0.99, 1.0 );
      spriteTris[2].Position    = float4( topLeftClipspace.x,  botRightClipspace.y, 0.99, 1.0 );
      spriteTris[3].Position    = float4( botRightClipspace.x, botRightClipspace.y, 0.99, 1.0 );

      for( int i = 0; i < 4; i++ )
          triStream.Append( spriteTris[i] );

      triStream.RestartStrip( );
  }
#else
    float2 topLeftClipspace, botRightClipspace;
    [branch]
    if( VolumeShadowGetRebalancingQuad( primitiveID, topLeftClipspace, botRightClipspace ) )
    {
      // trigger rebalance flag
      spriteTris[0].Texcoord0.z = 1.0;
      spriteTris[1].Texcoord0.z = 1.0;
      spriteTris[2].Texcoord0.z = 1.0;
      spriteTris[3].Texcoord0.z = 1.0;

      spriteTris[0].Position    = float4( topLeftClipspace.x,  topLeftClipspace.y,     0.5, 1.0 );
      spriteTris[1].Position    = float4( botRightClipspace.x, topLeftClipspace.y,     0.5, 1.0 );
      spriteTris[2].Position    = float4( topLeftClipspace.x,  botRightClipspace.y,    0.5, 1.0 );
      spriteTris[3].Position    = float4( botRightClipspace.x, botRightClipspace.y,    0.5, 1.0 );

      for( int i = 0; i < 4; i++ )
          triStream.Append( spriteTris[i] );

      triStream.RestartStrip( );
  }
#endif 

#endif // #ifdef VA_VOLUME_SHADOWS_PLUGIN_REQUIRES_FULLSCREEN_REBALANCE_PASS
#endif // #ifdef SHADOWS_GENERATE
#endif // #ifdef VA_VOLUME_SHADOWS_PLUGIN_USE

#endif // #ifndef AVSM_ENABLE_CACHE_FLUSH_SIMD_SYNC
  
}

float4 SimpleParticleColor( const in GenericBillboardSpriteVertexTransformed input, float4 color, const float shadowTerm )
{
    if( g_Global.WireframePass > 0.0 )
    {
        return float4( 0.5, 0.0, 0.0, 1.0 );
    }

    color.rgb *= (shadowTerm * 0.8) + 0.2;

//    // premult alpha blend
//    color.rgb  *= color.aaa;

    return color; // float4( input.Texcoord0.xy, 0.0, 1.0 );
}

// inverseTransmittance is Alpha
void CalcParticleVolumeParams( const in GenericBillboardSpriteVertexTransformed input, const float textureAlpha, const float translucencyMultiplier, out float3 outEntryViewspace, out float3 outExitViewspace, out float oneMinusTransmittance )
{
    float3 eyeVec           = normalize( input.ViewspacePos.xyz );

    float particleSize      = input.ViewspacePos.w;

    float alphaFromTexture  = textureAlpha;
    float alphaFromVertex   = input.Color.a;
    float alphaCombined     = saturate( alphaFromTexture * alphaFromVertex );
    float transmittance     = 1.0 - alphaCombined;
    
    // For translucent media with constant absorption we can use Beer's Law to approximate translucence " T(d) = exp( -k * d ) " where d is depth travelled, and k is absorption function.
    // In this function we do this in reverse, having to calculate approximate thickness from the particle max thicknes (particleSize) and transucence given by alpha (obtained, 
    // for example, from an artist-generated texture).
    //
    // The absorptionConstant controls the approximation of the thickness (the smaller the absorptionConstant is, the more correct it becomes, and the larger it is, 
    // the more linear is the relationship to input alpha)
    // use "plot -log(1-x*(1-e^(-1/z))) * z, x = 0.0 to 1.0, y = 0.0 to 1.0, z = 0.3" to plot it in wolfram alpha
    const float absorptionConstant              = 1.0 / 0.4; // the lower the constant, the more logarithmy-like curve is
    const float maxAlphaForRelativeThicknessOne = 1.0 - exp( -1.0 * absorptionConstant );

    float relativeThickness     = saturate( -log( 1.0 - alphaCombined * maxAlphaForRelativeThicknessOne ) * absorptionConstant );

    oneMinusTransmittance       = 1.0 - pow( transmittance, translucencyMultiplier );
    outEntryViewspace           = input.ViewspacePos.xyz - (relativeThickness * 0.5 * particleSize ) * eyeVec;
    outExitViewspace            = input.ViewspacePos.xyz + (relativeThickness * 0.5 * particleSize ) * eyeVec;
}

float4 SimpleParticlePS( const in GenericBillboardSpriteVertexTransformed input ) : SV_Target
{
    float4 textureRGBA = g_textureColor.Sample( g_samplerAnisotropicClamp, input.Texcoord0.xy );

    float3 entryViewspace;
    float3 exitViewspace;
    float oneMinusTransmittance;    // a.k.a. alpha

     // alpha/translucency is in .x at the moment
    if( textureRGBA.x < (1.0 / 255.0) ) discard;    // early out on low alpha
    CalcParticleVolumeParams( input, textureRGBA.x, 1.0, entryViewspace, exitViewspace, oneMinusTransmittance );

    float4 color = float4( input.Color.rgb, oneMinusTransmittance );

    float4 ret = SimpleParticleColor( input, color, 1.0 );
//    if( ret.a < 1.0 / 255.0 )
//        discard;
    return ret;
}

float4 SimpleParticleShadowedPS( const in GenericBillboardSpriteVertexTransformed input ) : SV_Target
{
//    if( input.Texcoord0.z != 0.0 )
//        return float4( 0, 1, 0, 0.5 );
//    else
//        return float4( 0, 0, 1, 0.5 );

    float4 textureRGBA = g_textureColor.Sample( g_samplerAnisotropicClamp, input.Texcoord0.xy );

    float3 entryViewspace;
    float3 exitViewspace;
    float oneMinusTransmittance;    // a.k.a. alpha

     // alpha/translucency is in .x at the moment
    if( textureRGBA.x < (1.0 / 255.0) ) discard;    // early out on low alpha
    CalcParticleVolumeParams( input, textureRGBA.x, 1.0, entryViewspace, exitViewspace, oneMinusTransmittance );

    float4 color = float4( input.Color.rgb, oneMinusTransmittance );

    const float simpleNoise = frac( sin( dot( sin( input.Texcoord0.xy ), float2( 12.9898, 78.233 ) ) ) * 43758.5453 );
    const float perParticleNoise = frac( input.Texcoord0.w + simpleNoise );
    float shadowTerm = SimpleShadowMapSampleVolumetricMedQ( entryViewspace, exitViewspace, perParticleNoise, oneMinusTransmittance );

    float4 ret = SimpleParticleColor( input, color, shadowTerm );

    // debugging the UV
    //float4 shadowClipSpace = mul( g_SimpleShadowsGlobal.CameraViewToShadowUVNormalizedSpace, float4( entryViewspace, 1.0 ) );
    //shadowClipSpace.xyz /= shadowClipSpace.w;
    //return float4( frac( shadowClipSpaceEntry.xy * 20 ) , 0, 1 );
    
    // debugging the viewspace z
    //float4 shadowViewspace = mul( g_SimpleShadowsGlobal.CameraViewToShadowView, float4( entryViewspace, 1.0 ) );
    //shadowViewspace.xyz /= shadowViewspace.w;
    //return float4( frac( shadowViewspace.z ), 0 , 0, 1 );

    //ret.xyz = abs(exitViewspace.z - entryViewspace.z) > 1.0;

    return ret;
}

#ifdef SHADOWS_GENERATE

void SimpleParticleGenerateVolumeShadowPS( const in GenericBillboardSpriteVertexTransformed input )
{
#ifdef VA_VOLUME_SHADOWS_PLUGIN_USE

    // alpha/translucency is in .x at the moment
    float4 textureRGBA          = float4( 0, 0, 0, 0 );
    float entryZ                = 0.0;
    float thickness             = 0.0;
    float segmentTransmittance  = 1.0;

    bool forceRebalanceFlag = false;

#ifdef AVSM_ENABLE_CACHE_FLUSH_SIMD_SYNC
    int currentCacheCount = VolumeShadowDeferredResolveIfAnySIMDLaneCacheFull( (int2)input.Position.xy ); 
#else
#ifdef VA_VOLUME_SHADOWS_PLUGIN_REQUIRES_FULLSCREEN_REBALANCE_PASS
    forceRebalanceFlag = input.Texcoord0.z != 0.0;
    if( forceRebalanceFlag )
    {
        VolumeShadowDeferredResolve( (int2)input.Position.xy ); 

        return;
    }
#endif
#endif

    textureRGBA = g_textureColor.Sample( g_samplerAnisotropicClamp, input.Texcoord0.xy );

    // early out on low alpha, but not in case rebalancing is used as it harms thread coherency and causes more harm than good
#ifndef VA_VOLUME_SHADOWS_PLUGIN_REQUIRES_FULLSCREEN_REBALANCE_PASS
    if( textureRGBA.x < (1.0 / 255.0) ) 
        discard;    // early out on low alpha
#endif

    const float volumetricShadowCastOpacityModifier = g_SimpleAVSMGlobal.ShadowOpacityScale;
    const float volumetricShadowMinNodeThickness    = 0.01;

    float3 entryViewspace       = float3( 0.0, 0.0, 0.0 );
    float3 exitViewspace        = float3( 0.0, 0.0, 0.0 );
    float oneMinusTransmittance = 0.0;    // a.k.a. alpha
    CalcParticleVolumeParams( input, textureRGBA.x, volumetricShadowCastOpacityModifier, entryViewspace, exitViewspace, oneMinusTransmittance );
    
    entryZ                  = entryViewspace.z;
    thickness               = max( exitViewspace.z-entryViewspace.z, volumetricShadowMinNodeThickness );
    segmentTransmittance    = 1.0 - oneMinusTransmittance;

    // using shadow (light) viewspace depth for all calculations
#ifdef AVSM_ENABLE_CACHE_FLUSH_SIMD_SYNC
    VolumeShadowStore( (int2)input.Position.xy, entryZ, thickness, segmentTransmittance, currentCacheCount );
#else
    VolumeShadowStore( (int2)input.Position.xy, entryZ, thickness, segmentTransmittance, forceRebalanceFlag );
#endif

#endif
}

#endif