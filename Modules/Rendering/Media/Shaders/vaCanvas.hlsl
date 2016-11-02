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

struct VSOutput2D
{                                       
   float4 xPos            : SV_Position;
   float4 xColor          : COLOR;
   float4 xUV             : TEXCOORD0;  
   float2 xOrigScreenPos  : TEXCOORD1;  
};                                      

VSOutput2D VS_Canvas2D( const float4 xInPos : SV_Position, const float4 xInColor : COLOR, const float4 xUV : TEXCOORD0, const float2 xScreenPos : TEXCOORD1 )
{
   VSOutput2D output;
   output.xPos           = xInPos;
   output.xColor         = xInColor;
   output.xUV            = xUV;
   output.xOrigScreenPos = xScreenPos;
   return output;
}

float4 PS_Canvas2D( const VSOutput2D xInput ) : SV_Target
{
   return xInput.xColor;
}

struct VSOutput3D
{                                                                               
  float4 xPos   : SV_Position;                                                
  float4 xColor : TEXCOORD0;                                                    
};

VSOutput3D VS_Canvas3D( const float4 xInPos : SV_Position, const float4 xInColor : COLOR )
{                                                                               
   VSOutput3D output;                                                                
   output.xPos = xInPos;                                                         
   output.xColor = xInColor;                                                     
   return output;                                                                
}                                                                               

float4 PS_Canvas3D( const VSOutput3D xInput ) : SV_Target
{                                               
   return xInput.xColor; // + frac( float4( 0.001f * (float)g_globalShared.FrameCounter, 0, 0, 0 ) );
}