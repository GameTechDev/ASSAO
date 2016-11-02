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

#pragma once

#include "Rendering/vaRenderingIncludes.h"

#include "Rendering/DirectX/vaDirectXIncludes.h"

namespace VertexAsylum
{

    class vaVertexInputLayoutsDX11
    {
    public:
        class CD3D11_INPUT_ELEMENT_DESC : public D3D11_INPUT_ELEMENT_DESC
        {
        public:
            CD3D11_INPUT_ELEMENT_DESC( LPCSTR semanticName, UINT semanticIndex, DXGI_FORMAT format, UINT inputSlot, UINT alignedByteOffset, D3D11_INPUT_CLASSIFICATION inputSlotClass, UINT instanceDataStepRate )
            {
                SemanticName = semanticName;
                SemanticIndex = semanticIndex;
                Format = format;
                InputSlot = inputSlot;
                AlignedByteOffset = alignedByteOffset;
                InputSlotClass = inputSlotClass;
                InstanceDataStepRate = instanceDataStepRate;
            }
        };

    public:
        //static std::vector<D3D11_INPUT_ELEMENT_DESC> PositionColorNormalTangentTexcoord1Vertex_WInsteadOfColor_Decl( )
        //{
        //    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;
        //
        //    inputElements.push_back( CD3D11_INPUT_ELEMENT_DESC( "SV_Position", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 ) );
        //    inputElements.push_back( CD3D11_INPUT_ELEMENT_DESC( "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 ) );
        //    inputElements.push_back( CD3D11_INPUT_ELEMENT_DESC( "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 ) );
        //    inputElements.push_back( CD3D11_INPUT_ELEMENT_DESC( "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 ) );
        //
        //    return inputElements;
        //}

        static std::vector<D3D11_INPUT_ELEMENT_DESC> PositionColorNormalTangentTexcoord1VertexDecl( )
        {
            std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;

            inputElements.push_back( CD3D11_INPUT_ELEMENT_DESC( "SV_Position",  0, DXGI_FORMAT_R32G32B32_FLOAT,     0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 ) );
            inputElements.push_back( CD3D11_INPUT_ELEMENT_DESC( "COLOR",        0, DXGI_FORMAT_R8G8B8A8_UNORM,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 ) );
            inputElements.push_back( CD3D11_INPUT_ELEMENT_DESC( "NORMAL",       0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 ) );
            inputElements.push_back( CD3D11_INPUT_ELEMENT_DESC( "TANGENT",      0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 ) );
            inputElements.push_back( CD3D11_INPUT_ELEMENT_DESC( "TEXCOORD",     0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 ) );

            return inputElements;
        }

        static std::vector<D3D11_INPUT_ELEMENT_DESC> BillboardSpriteVertexDecl( )
        {
            std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;

            inputElements.push_back( CD3D11_INPUT_ELEMENT_DESC( "SV_Position",  0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 ) );
            inputElements.push_back( CD3D11_INPUT_ELEMENT_DESC( "TEXCOORD",     0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 ) );
            inputElements.push_back( CD3D11_INPUT_ELEMENT_DESC( "COLOR",        0, DXGI_FORMAT_R8G8B8A8_UNORM,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 ) );

            return inputElements;
        }
    };

    class vaTextureLoaderDX11
    {
    public:

        // static ID3D11Resource *                 Load( const wstring & texturePath, bool assumeSourceIsInSRGB );
        // static ID3D11ShaderResourceView *       LoadAsSRV( const wstring & texturePath, bool assumeSourceIsInSRGB );
    };

}