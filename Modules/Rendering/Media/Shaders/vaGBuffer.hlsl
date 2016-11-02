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

Texture2D           g_textureSlot0              : register(t0);

float DepthToViewspaceLinearPS( const in float4 Position : SV_Position, const in float2 Texcoord : TEXCOORD0 ) : SV_Target
{
    float v = g_textureSlot0.Load( int3( int2(Position.xy), 0 ) ).x;
    return NDCToViewDepth( v );
}

float4 DebugDrawDepthPS( const in float4 Position : SV_Position, const in float2 Texcoord : TEXCOORD0 ) : SV_Target
{
    float v = g_textureSlot0.Load( int3( int2(Position.xy), 0 ) ).x;
    return float4( frac(v), frac(v*10.0), frac(v*100.0), 1.0 );
}

float4 DebugDrawDepthViewspaceLinearPS( const in float4 Position : SV_Position, const in float2 Texcoord : TEXCOORD0 ) : SV_Target
{
    float v = g_textureSlot0.Load( int3( int2(Position.xy), 0 ) ).x;
    //return float4( v.xxx/100.0, 1.0 );
    return float4( frac(v), frac(v/10.0), frac(v/100.0), 1.0 );
}

float4 DebugDrawNormalMapPS( const in float4 Position : SV_Position, const in float2 Texcoord : TEXCOORD0 ) : SV_Target
{
    float3 v = g_textureSlot0.Load( int3( int2(Position.xy), 0 ) ).xyz;
    
    v = GBufferDecodeNormal( v );

    return float4( DisplayNormalSRGB(v), 1.0 );
}

float4 DebugDrawAlbedoPS( const in float4 Position : SV_Position, const in float2 Texcoord : TEXCOORD0 ) : SV_Target
{
    float3 v = g_textureSlot0.Load( int3( int2(Position.xy), 0 ) ).xyz;
    return float4( v, 1.0 );
}

float4 DebugDrawRadiancePS( const in float4 Position : SV_Position, const in float2 Texcoord : TEXCOORD0 ) : SV_Target
{
    float3 v = g_textureSlot0.Load( int3( int2(Position.xy), 0 ) ).xyz;
    return float4( v, 1.0 );
}
