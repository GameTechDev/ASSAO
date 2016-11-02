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

Texture2D   g_textureAlbedo                 : register( T_CONCATENATER( RENDERMESH_TEXTURE_SLOT0 ) );
Texture2D   g_textureNormal                 : register( T_CONCATENATER( RENDERMESH_TEXTURE_SLOT1 ) );
Texture2D   g_textureSpecular               : register( T_CONCATENATER( RENDERMESH_TEXTURE_SLOT2 ) );

struct RenderMeshStandardVertexInput
{
    float4 Position             : SV_Position;
    float4 Color                : COLOR;
    float4 Normal               : NORMAL;
    float4 Tangent              : TANGENT;
    float2 Texcoord0            : TEXCOORD0;
    float2 Texcoord1            : TEXCOORD1;
};

void GetNormalAndTangentSpace( const GenericSceneVertexTransformed input, out float3 normal, out float3x3 tangentSpace )
{
#if VA_RMM_HASNORMALMAPTEXTURE
    float3 normal       = UnpackNormal( g_textureNormal.Sample( g_samplerAnisotropicWrap, input.Texcoord0.xy ) );
#else
    normal          = float3( 0.0, 0.0, 1.0 );
#endif

    tangentSpace    = float3x3(
			                    normalize( input.ViewspaceTangent.xyz   ),
			                    normalize( input.ViewspaceBitangent.xyz ),
                                normalize( input.ViewspaceNormal.xyz    ) );

    normal          = mul( normal, tangentSpace );
    normal          = normalize( normal );
}

float4 MeshColor( const GenericSceneVertexTransformed input, LocalMaterialValues lmv, float shadowTerm )
{
    const float3 diffuseLightVector     = -g_Global.Lighting.DirectionalLightViewspaceDirection.xyz;

    float3 color    = lmv.Albedo.rgb;
    float3 normal   = (lmv.IsFrontFace)?(lmv.Normal):(-lmv.Normal);

    float3 viewDir                      = normalize( input.ViewspacePos.xyz );

    // start calculating final colour
    float3 lightAccum       = color.rgb * g_Global.Lighting.AmbientLightIntensity.rgb;

    // directional light
    float nDotL             = dot( normal, diffuseLightVector );

    float3 reflected        = diffuseLightVector - 2.0*nDotL*normal;
	float rDotV             = saturate( dot(reflected, viewDir) );
    float specular          = saturate( pow( saturate( rDotV ), 8.0 ) );

    // facing towards light: front and specular
    float lightFront        = saturate( nDotL ) * shadowTerm;
    lightAccum              += lightFront * color.rgb * g_Global.Lighting.DirectionalLightIntensity.rgb;
    //lightAccum += fakeSpecularMul * specular;

    float3 finalColor       = saturate( lightAccum );
    finalColor              = saturate( finalColor );

    // input.ViewspacePos.w is distance to camera
    finalColor              = FogForwardApply( finalColor, input.ViewspacePos.w );

    return float4( finalColor, lmv.Albedo.a );
}

// #if defined(SHADOWS_GENERATE)
// 
// // generate opaque shadows pass
// 
// float4 RenderMeshVS( const in GenericSceneVertex input ) : SV_Position
// {
//     return mul( g_RenderMeshGlobal.ShadowWorldViewProj, float4( input.Position.xyz, 1) );
// }
// float4 RenderMeshPS( ) : SV_Target
// {
//     return float4( 0.0, 1.0, 0.0, 1.0 );
// }
// #if defined(SHADOWS_RECEIVE)
// #error Cannot have both shadow generate and receive defines at the same time
// #endif
// 
// #elif defined(SHADOWS_RECEIVE)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// new material system
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float4 VS_PosOnly( const in GenericSceneVertex input ) : SV_Position
{
    return mul( g_RenderMeshGlobal.ShadowWorldViewProj, float4( input.Position.xyz, 1) );
}

float4 PS_DepthOnly( ) : SV_Target
{
#if VA_RMM_ALPHATEST

#endif
    // if transparent, depth test
    return float4( 0.0, 1.0, 0.0, 1.0 );
}

GenericSceneVertexTransformed VS_Standard( const in RenderMeshStandardVertexInput input )
{
    GenericSceneVertexTransformed ret;

    ret.Color                   = input.Color;
    ret.Texcoord0               = float4( input.Texcoord0, input.Texcoord1 );

    ret.ViewspacePos            = mul( g_RenderMeshGlobal.WorldView, float4( input.Position.xyz, 1) );
    ret.ViewspaceNormal.xyz     = normalize( mul( g_RenderMeshGlobal.WorldView, float4(input.Normal.xyz, 0.0) ).xyz );
    ret.ViewspaceTangent.xyz    = normalize( mul( g_RenderMeshGlobal.WorldView, float4(input.Tangent.xyz, 0.0) ).xyz );
    ret.ViewspaceBitangent.xyz  = normalize( cross( ret.ViewspaceNormal.xyz, ret.ViewspaceTangent.xyz) * input.Tangent.w );     // Tangent.w contains handedness/uv.y direction!

    // distance to camera
    ret.ViewspacePos.w          = length( ret.ViewspacePos.xyz );

    ret.Position                = mul( g_Global.Proj, float4( ret.ViewspacePos.xyz, 1.0 ) );

    ret.ViewspaceNormal.w       = 0.0;
    ret.ViewspaceTangent.w      = 0.0;
    ret.ViewspaceBitangent.w    = 0.0;

    return ret;
}

LocalMaterialValues GetLocalMaterialValues( const in GenericSceneVertexTransformed input, const bool isFrontFace )
{
    LocalMaterialValues ret;

    ret.IsFrontFace     = isFrontFace;
    ret.Albedo          = input.Color;

    GetNormalAndTangentSpace( input, ret.Normal, ret.TangentSpace );

#if VA_RMM_HASALBEDOTEXTURE
    ret.Albedo *= g_textureAlbedo.Sample( g_samplerAnisotropicWrap, input.Texcoord0.xy );
#endif

    return ret;
}

float4 PS_Forward( const in GenericSceneVertexTransformed input, const bool isFrontFace : SV_IsFrontFace ) : SV_Target
{
    if( g_Global.WireframePass > 0.0 )
    {
        return float4( 0.5, 0.0, 0.0, 1.0 );
    }

    float shadowTerm = 1; //SimpleShadowMapSample( input.ViewspacePos.xyz );

    LocalMaterialValues lmv = GetLocalMaterialValues( input, isFrontFace );

#if VA_RMM_ALPHATEST
    if( lmv.Albedo.a < 0.5 ) // g_RenderMeshMaterialGlobal.AlphaCutoff
        discard;
#endif

    return MeshColor( input, lmv, shadowTerm );
}

GBufferOutput PS_Deferred( const in GenericSceneVertexTransformed input, const bool isFrontFace : SV_IsFrontFace )
{
    LocalMaterialValues lmv = GetLocalMaterialValues( input, isFrontFace );

#if VA_RMM_ALPHATEST
    if( lmv.Albedo.a < 0.5 ) // g_RenderMeshMaterialGlobal.AlphaCutoff
        discard;
#endif

    return EncodeGBuffer( lmv );
}
