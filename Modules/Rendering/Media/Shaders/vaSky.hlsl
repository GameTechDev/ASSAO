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


struct VSOutput
{
   float4 Position      : SV_Position;
   float4 PositionProj  : TEXCOORD0;
};


VSOutput SimpleSkyboxVS( const float4 inPos : SV_Position )
{
   VSOutput ret;

   ret.Position = inPos;
   ret.PositionProj = inPos; //-mul( ProjToWorld, xOut.m_xPosition ).xyz;	

   return ret;
}

float4 SimpleSkyboxPS( const VSOutput input ) : SV_Target
{
   float4 skyDirection = mul( g_SimpleSkyGlobal.ProjToWorld, input.PositionProj ).xyzw;
   skyDirection *= 1 / skyDirection.wwww;
   skyDirection.xyz = normalize( skyDirection.xyz );

   ////////////////////////////////////////////////////////////////////////////////////////////////
   // this is a quick ad-hoc noise algorithm used to dither the gradient, to mask the 
   // banding on these awful monitors
   // FS_2015: lol - this comment is from around 2005 - LCD dispays those days were apparently really bad :)
   float noise = frac( dot( sin( skyDirection.xyz * float3( 533, 599, 411 ) ) * 10, float3( 1.0, 1.0, 1.0 ) ) );
   float noiseAdd = (noise - 0.5) * 0.1;
   // noiseAdd = 0.0; // to disable noise, just uncomment this
   //
   float noiseMul = 1 - noiseAdd;
   ////////////////////////////////////////////////////////////////////////////////////////////////
   
   // Horizon

   float horizonK = 1 - dot( skyDirection.xyz, float3( 0, 0, 1 ) );
   horizonK = saturate( pow( abs( horizonK ), g_SimpleSkyGlobal.SkyColorLowPow ) * g_SimpleSkyGlobal.SkyColorLowMul );
   horizonK *= noiseMul;

   float4 finalColor = lerp( g_SimpleSkyGlobal.SkyColorHigh, g_SimpleSkyGlobal.SkyColorLow, horizonK );

   // Sun

   float dirToSun = saturate( dot( skyDirection.xyz, g_SimpleSkyGlobal.SunDir.xyz ) / 2.0 + 0.5 );

   float sunPrimary = clamp( pow( dirToSun, g_SimpleSkyGlobal.SunColorPrimaryPow ) * g_SimpleSkyGlobal.SunColorPrimaryMul, 0.0f, 1.0f );
   sunPrimary *= noiseMul;

   finalColor += g_SimpleSkyGlobal.SunColorPrimary * sunPrimary;

   float sunSecondary = clamp( pow( dirToSun, g_SimpleSkyGlobal.SunColorSecondaryPow ) * g_SimpleSkyGlobal.SunColorSecondaryMul, 0.0f, 1.0f );
   sunSecondary *= noiseMul;

   finalColor += g_SimpleSkyGlobal.SunColorSecondary * sunSecondary;
   
   return float4( finalColor.xyz, 1 );
}
