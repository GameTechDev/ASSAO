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

//
//
//#ifndef VAMATERIALSHARED_SH_INCLUDED
//#define VAMATERIALSHARED_SH_INCLUDED
//
//#include "vaShared.hlsl"
//
//cbuffer B2 : register(b2)
//{
//   MaterialConstants    g_material;
//};
//
//Texture2D    g_materialDiffuseTexture           : register( t2 );
//Texture2D    g_materialEmissiveTexture          : register( t3 );
//Texture2D    g_materialNormalmapTexture         : register( t4 );
//Texture2D    g_materialExtraParamsTexture       : register( t5 );
//
//
//// TEMP
//#define HAS_DIFFUSE_TEXTURE 1
//
//float4   MaterialGetDiffuse( const float2 texUV )
//{
//   float4   color = float4( 1, 1, 1, 1 );
//#if HAS_DIFFUSE_TEXTURE
//   color = g_materialDiffuseTexture.Sample( g_samplerLinearWrap, texUV );
//#endif
//   return color * g_material.DiffuseBase;
//}
//
///*
//float3   MaterialGetAmbient( const float2 texUV )
//{
//   return g_currMat.AmbientColour.rgb;
//}
//
//float4   MaterialGetSpecular( const float2 texUV )
//{
//   return ( g_currMat.HasSpecularTexture ) ? ( g_currMatSpecularTex.Sample( g_samplerLinearWrap, texUV ) * g_currMat.SpecularColour ) : ( g_currMat.SpecularColour );
//}
//
//float3   MaterialGetNormal( const float3 normal, const float3 tangent, const float3 bitangent, const float2 texUV )
//{
//   //return ( g_currMat.HasNormalmapTexture ) ? ( g_currNormalmapTex.Sample( g_samplerLinearWrap, texUV ) ) : ( float3( 0, 0, 1 ) );
//   return normalize( normal );
//}
//
//float3   MaterialGetEmissive( const float2 texUV )
//{
//   return g_currMat.EmissiveColour.rgb;
//}
//*/
//
//#endif // VAMATERIALSHARED_SH_INCLUDED
//