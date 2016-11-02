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

#include "Rendering/vaRenderingGlobals.h"

#include "Rendering/vaTexture.h"

using namespace VertexAsylum;

vaRenderingGlobals::vaRenderingGlobals( )
{ 
    assert( vaRenderingCore::IsInitialized() );


    for( int i = 0; i < c_shaderDebugOutputSyncDelay; i++ )
    {
        m_shaderDebugOutputGPUTextures[i] = std::shared_ptr<vaTexture>( vaTexture::Create1D( vaTextureFormat::R32_FLOAT, c_shaderDebugFloatOutputCount, 1, 1, vaTextureBindSupportFlags::UnorderedAccess, vaTextureAccessFlags::None ) );
        m_shaderDebugOutputCPUTextures[i] = std::unique_ptr<vaTexture>( vaTexture::Create1D( vaTextureFormat::R32_FLOAT, c_shaderDebugFloatOutputCount, 1, 1, vaTextureBindSupportFlags::None, vaTextureAccessFlags::CPURead ) );
    }

    for( int i = 0; i < c_shaderDebugFloatOutputCount; i++ )
    {
        m_shaderDebugFloats[i] = 0.0f;
    }

    m_frameIndex = 0;
}

vaRenderingGlobals::~vaRenderingGlobals( )
{
}

void vaRenderingGlobals::Tick( float deltaTime )
{
    m_frameIndex++;
}


