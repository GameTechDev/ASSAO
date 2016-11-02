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

#ifndef VASHARED_HLSL_INCLUDED
#define VASHARED_HLSL_INCLUDED

#ifndef VA_COMPILED_AS_SHADER_CODE
#error not intended to be included outside of HLSL!
#endif

#include "vaSharedTypes.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Global sampler slots
//SamplerState                            g_PointClampSampler         : register( S_CONCATENATER( SHADERGLOBAL_POINTCLAMP_SAMPLERSLOT ) ); 

SamplerState                            g_samplerPointClamp         : register( S_CONCATENATER( SHADERGLOBAL_POINTCLAMP_SAMPLERSLOT         ) ); 
SamplerState                            g_samplerPointWrap          : register( S_CONCATENATER( SHADERGLOBAL_POINTWRAP_SAMPLERSLOT          ) ); 
SamplerState                            g_samplerLinearClamp        : register( S_CONCATENATER( SHADERGLOBAL_LINEARCLAMP_SAMPLERSLOT        ) ); 
SamplerState                            g_samplerLinearWrap         : register( S_CONCATENATER( SHADERGLOBAL_LINEARWRAP_SAMPLERSLOT         ) ); 
SamplerState                            g_samplerAnisotropicClamp   : register( S_CONCATENATER( SHADERGLOBAL_ANISOTROPICCLAMP_SAMPLERSLOT   ) ); 
SamplerState                            g_samplerAnisotropicWrap    : register( S_CONCATENATER( SHADERGLOBAL_ANISOTROPICWRAP_SAMPLERSLOT    ) ); 


SamplerComparisonState                  g_SimpleShadowMapCmpSampler     : register( S_CONCATENATER( SHADERSIMPLESHADOWSGLOBAL_CMPSAMPLERSLOT ) ); 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Global texture slots
Texture2D<float>                        g_SimpleShadowMapTexture        : register( T_CONCATENATER( SHADERSIMPLESHADOWSGLOBAL_TEXTURESLOT ) ); 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Global types
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct GenericSceneVertex
{
    float4 Position             : SV_Position;
    float4 Color                : COLOR;
    float4 Normal               : NORMAL;
    float4 Tangent              : TANGENT;
    float4 Texcoord             : TEXCOORD0;
};

struct GenericSceneVertexTransformed
{
    float4 Position             : SV_Position;
    float4 Color                : COLOR;
    float4 ViewspacePos         : TEXCOORD0;
    float4 ViewspaceNormal      : NORMAL0;
    float4 ViewspaceTangent     : NORMAL1;
    float4 ViewspaceBitangent   : NORMAL2;
    float4 Texcoord0            : TEXCOORD1;
};

struct GenericBillboardSpriteVertex
{
    float4 Position_CreationID  : SV_Position;
    float4 Color                : COLOR;
    float4 Transform2D          : TEXCOORD0;
};

struct GBufferOutput
{
    float4  Albedo              : SV_Target0;
    float4  Normal              : SV_Target1;
};

struct GBufferDecodedPixel
{
    float3  Albedo;
    float3  Normal;
    float3  ViewspacePos;
};

struct LocalMaterialValues
{
    float4      Albedo;
    float3      Normal;
    float3x3    TangentSpace;
    bool        IsFrontFace;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GenericSceneVertexTransformed GenericColoredVS( const in GenericSceneVertex input )
{
    GenericSceneVertexTransformed ret;

    ret.Color                   = input.Color   ;
    ret.Texcoord0               = input.Texcoord;

    ret.ViewspacePos            = mul( g_Global.View, float4( input.Position.xyz, 1.0 ) );
    ret.ViewspaceNormal.xyz     = normalize( mul( g_Global.View, float4(input.Normal.xyz, 0.0) ).xyz );
    ret.ViewspaceTangent.xyz    = normalize( mul( g_Global.View, float4(input.Tangent.xyz, 0.0) ).xyz );
    ret.ViewspaceBitangent.xyz  = normalize( cross( ret.ViewspaceNormal.xyz, ret.ViewspaceTangent.xyz) );

    // distance to camera
    ret.ViewspacePos.w          = length( ret.ViewspacePos.xyz );

    ret.Position                = mul( g_Global.Proj, float4( ret.ViewspacePos.xyz, 1.0 ) );

    ret.ViewspaceNormal.w       = 0.0;
    ret.ViewspaceTangent.w      = 0.0;
    ret.ViewspaceBitangent.w    = 0.0;

    return ret;
}


// ///////////////////////////////////////////////////////////////////////////////////////////////////
// // Global constant buffers
// 
// cbuffer B0 : register(b0)        { GlobalSharedConstants    g_globalShared; };
// cbuffer B1 : register(b1)        { CameraSharedConstants    g_cameraShared; };




// ///////////////////////////////////////////////////////////////////////////////////////////////////
// // Global utility functions (some might be using constants above, some not)
// 
//float NDCToViewDepth( float screenDepth )
//{
//   float depthLinearizeMul = g_cameraShared.DepthUnpackConsts.x;
//   float depthLinearizeAdd = g_cameraShared.DepthUnpackConsts.y;
//
//   // Optimised version of "-cameraClipNear / (cameraClipFar - projDepth * (cameraClipFar - cameraClipNear)) * cameraClipFar"
//
//   // Set your depthLinearizeMul and depthLinearizeAdd to:
//   // depthLinearizeMul = ( cameraClipFar * cameraClipNear) / ( cameraClipFar - cameraClipNear );
//   // depthLinearizeAdd = cameraClipFar / ( cameraClipFar - cameraClipNear );
//
// 	return depthLinearizeMul / (depthLinearizeAdd - screenDepth);
//}

float GLSL_mod( float x, float y )
{
    return x - y * floor( x / y );
}
float2 GLSL_mod( float2 x, float2 y )
{
    return x - y * floor( x / y );
}
float3 GLSL_mod( float3 x, float3 y )
{
    return x - y * floor( x / y );
}

///////////////////////////////////////////////////////////////////////////////
// Perlin simplex noise
//
// Description : Array and textureless GLSL 2D simplex noise function.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : ijm
//     Lastmod : 20110409 (stegu)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
float3 permute(float3 x) { return GLSL_mod(((x*34.0)+1.0)*x, 289.0); }
//
float snoise(float2 v)
{
    const float4 C = float4( 0.211324865405187, 0.366025403784439, -0.577350269189626, 0.024390243902439 );
  float2 i  = floor(v + dot(v, C.yy) );
  float2 x0 = v -   i + dot(i, C.xx);
  float2 i1;
  i1 = (x0.x > x0.y) ? float2(1.0, 0.0) : float2(0.0, 1.0);
  float4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;
  i = GLSL_mod(i, 289.0);
    float3 p = permute( permute( i.y + float3( 0.0, i1.y, 1.0 ) ) + i.x + float3( 0.0, i1.x, 1.0 ) );
  float3 m = max(0.5 - float3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;
  float3 x = 2.0 * frac(p * C.www) - 1.0;
  float3 h = abs(x) - 0.5;
  float3 ox = floor(x + 0.5);
  float3 a0 = x - ox;
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
  float3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}
///////////////////////////////////////////////////////////////////////////////

// From DXT5_NM
float3 UnpackNormal( float4 packedNormal )
{
    float3 normal;
    normal.xy = packedNormal.wy * 2.0 - 1.0;
    normal.z = sqrt( 1.0 - normal.x*normal.x - normal.y * normal.y );
    return normal;
}

float3 DisplayNormal( float3 normal )
{
    return normal * 0.5 + 0.5;
}

float3 DisplayNormalSRGB( float3 normal )
{
    return pow( abs( normal * 0.5 + 0.5 ), 2.2 );
}

// good source of ideas for future improvements: http://iquilezles.org/www/articles/fog/fog.htm
float3 FogForwardApply( float3 color, float viewspaceDistance )
{
    //return frac( viewspaceDistance / 10.0 );
    float d = max(0.0, viewspaceDistance - g_Global.Lighting.FogDistanceMin);
    float fogStrength = exp( - d * g_Global.Lighting.FogDensity ); 
    return lerp( g_Global.Lighting.FogColor.rgb, color.rgb, fogStrength );
}

float3 DebugViewGenericSceneVertexTransformed( in float3 inColor, const in GenericSceneVertexTransformed input )
{
//    inColor.x = 1.0;

    return inColor;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Normals encode/decode
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float3 GBufferEncodeNormal( float3 normal )
{
    float3 encoded = normal * 0.5 + 0.5;

    return encoded;
}

float3 GBufferDecodeNormal( float3 encoded )
{
    float3 normal = encoded * 2.0 - 1.0;
    return normalize( normal );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Space conversions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// normalized device coordinates (SV_Position from PS) to viewspace depth
float NDCToViewDepth( float screenDepth )
{
    float depthLinearizeMul = g_Global.DepthUnpackConsts.x;
    float depthLinearizeAdd = g_Global.DepthUnpackConsts.y;

    // Optimised version of "-cameraClipNear / (cameraClipFar - projDepth * (cameraClipFar - cameraClipNear)) * cameraClipFar"

    // Set your depthLinearizeMul and depthLinearizeAdd to:
    // depthLinearizeMul = ( cameraClipFar * cameraClipNear) / ( cameraClipFar - cameraClipNear );
    // depthLinearizeAdd = cameraClipFar / ( cameraClipFar - cameraClipNear );

    return depthLinearizeMul / ( depthLinearizeAdd - screenDepth );
}

// from [0, width], [0, height] to [-1, 1], [-1, 1]
float2 NDCToClipSpacePositionXY( float2 SVPos )
{
    return SVPos * float2( g_Global.ViewportPixel2xSize.x, -g_Global.ViewportPixel2xSize.y ) + float2( -1.0f, 1.0f );
}

float3 NDCToViewspacePosition( float2 SVPos, float viewspaceDepth )
{
    return float3( g_Global.CameraTanHalfFOV.xy * viewspaceDepth * NDCToClipSpacePositionXY( SVPos ), viewspaceDepth );
}

float3 ClipSpaceToViewspacePosition( float2 clipPos, float viewspaceDepth )
{
    return float3( g_Global.CameraTanHalfFOV.xy * viewspaceDepth * clipPos, viewspaceDepth );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GBuffer helpers
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GBufferOutput EncodeGBuffer( LocalMaterialValues lmv )
{
    GBufferOutput ret;
    
    float3 normal   = (lmv.IsFrontFace)?(lmv.Normal):(-lmv.Normal);

    ret.Albedo = float4( lmv.Albedo.xyz, 0.0 );
    ret.Normal = float4( GBufferEncodeNormal( normal ), 0.0 );

    return ret;
}

GBufferDecodedPixel DecodeGBuffer( float2 SVPos, float viewspaceDepth, float4 albedoPixelValue, float4 normalPixelValue )
{
    GBufferDecodedPixel ret;

    ret.Albedo          = albedoPixelValue.xyz;
    ret.Normal          = GBufferDecodeNormal( normalPixelValue.xyz );

    ret.ViewspacePos    = NDCToViewspacePosition( SVPos.xy, viewspaceDepth );

    return ret;
}


#endif // VASHARED_HLSL_INCLUDED