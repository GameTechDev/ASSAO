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

SamplerState        g_samplerSlot0                  : register( S_CONCATENATER( POSTPROCESS_SAMPLERS_SLOT0 ) );
SamplerState        g_samplerSlot1                  : register( S_CONCATENATER( POSTPROCESS_SAMPLERS_SLOT1 ) );

RWTexture2D<uint>   g_AVSMDeferredCounterUAV        : register( U_CONCATENATER( POSTPROCESS_COMPARISONRESULTS_UAV_SLOT ) );

Texture2D           g_sourceTexure0                 : register( T_CONCATENATER( POSTPROCESS_TEXTURE_SLOT0 ) );
Texture2D           g_sourceTexure1                 : register( T_CONCATENATER( POSTPROCESS_TEXTURE_SLOT1 ) );

void PSCompareTextures( const in float4 Position : SV_Position, const in float2 Texcoord : TEXCOORD0 )
{
    int2 pixelCoord         = (int2)Position.xy;

    float4 col0 = g_sourceTexure0.Load( int3( pixelCoord, 0 ) );
    float4 col1 = g_sourceTexure1.Load( int3( pixelCoord, 0 ) );


    // Mean Squared Error
    float3 diff3 = col0.rgb - col1.rgb;
    diff3 = diff3 * diff3;

    // Convert to luma (instead of this, I need to upgrade to calculate MSE for each channel separately, but this works for now)
    float diff = dot( diff3, float3( 0.2989, 0.5866, 0.1145 ) );

    uint diffInt = (uint)(diff*8191.0 + 0.5);

    uint sizeX, sizeY;
    g_sourceTexure0.GetDimensions( sizeX, sizeY );
    uint linearAddr = (uint)pixelCoord.x + (uint)pixelCoord.y * sizeX;
    int UAVAddr = linearAddr % (uint)POSTPROCESS_COMPARISONRESULTS_SIZE;

    InterlockedAdd( g_AVSMDeferredCounterUAV[ int2(UAVAddr, 0) ], diffInt );
    //InterlockedAdd( g_AVSMDeferredCounterUAV[ int2(UAVAddr, 0) ], 1 ); // <- use to test that all pixels were covered
}

float4 PSDrawTexturedQuad( const in float4 Position : SV_Position, const in float2 Texcoord : TEXCOORD0 ) : SV_Target
{
    int2 pixelCoord         = (int2)Position.xy;
    float4 col = g_sourceTexure0.Load( int3( pixelCoord, 0 ) );
    return col;
}

void VSStretchRect( out float4 xPos : SV_Position, in float2 UV : TEXCOORD0, out float2 outUV : TEXCOORD0 ) 
{ 
    xPos.x = (1.0 - UV.x) * g_PostProcessConsts.Param1.x + UV.x * g_PostProcessConsts.Param1.z;
    xPos.y = (1.0 - UV.y) * g_PostProcessConsts.Param1.y + UV.y * g_PostProcessConsts.Param1.w;
    xPos.z = 0;
    xPos.w = 1;

    outUV.x = (1.0 - UV.x) * g_PostProcessConsts.Param2.x + UV.x * g_PostProcessConsts.Param2.z;
    outUV.y = (1.0 - UV.y) * g_PostProcessConsts.Param2.y + UV.y * g_PostProcessConsts.Param2.w;
}

float4 PSStretchRect( const in float4 Position : SV_Position, const in float2 Texcoord : TEXCOORD0 ) : SV_Target
{
    float4 col = g_sourceTexure0.Sample( g_samplerSlot0, Texcoord );

    return float4( col.rgb, 1.0 );

    //return float4( Texcoord.x, Texcoord.y, 0, 1 );
}