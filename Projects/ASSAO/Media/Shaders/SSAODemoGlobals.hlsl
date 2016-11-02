///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  
// Copyright (c) 2016, Intel Corporation
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation 
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of 
// the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
// SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef VA_COMPILED_AS_SHADER_CODE
#include "vaShaderCore.h"
#else
#include "Rendering/Media/Shaders/vaShaderCore.h"
#endif

#ifndef VA_COMPILED_AS_SHADER_CODE
namespace VertexAsylum
{
#endif
struct SSAODemoGlobalConstants
{
    vaMatrix4x4         World;
    vaMatrix4x4         WorldView;

    // vaVector2i          DebugTexelLocation;
    // float               DebugOverlayDepth;
    // float               DebugOverlayScale;
};


#ifndef VA_COMPILED_AS_SHADER_CODE
}
#endif

#define SSAODEMOGLOBAL_CONSTANTS_BUFFERSLOT             0

#ifdef VA_COMPILED_AS_SHADER_CODE

cbuffer SSAODemoGlobalConstantsBuffer : register( B_CONCATENATER( SSAODEMOGLOBAL_CONSTANTS_BUFFERSLOT ) )
{
    SSAODemoGlobalConstants         g_SSAODemoGlobals;
}


#endif