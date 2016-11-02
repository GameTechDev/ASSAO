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

#include "vaRenderDeviceContextDX11.h"
#include "vaTextureDX11.h"

#include "vaRenderingToolsDX11.h"

using namespace VertexAsylum;

namespace 
{
    struct CommonSimpleVertex
    {
        float   Position[4];
        float   UV[2];

        CommonSimpleVertex( ) {};
        CommonSimpleVertex( float px, float py, float pz, float pw, float uvx, float uvy ) { Position[0] = px; Position[1] = py; Position[2] = pz; Position[3] = pw; UV[0] = uvx; UV[1] = uvy; }
    };
}

// // used to make Gather using UV slightly off the border (so we get the 0,0 1,0 0,1 1,1 even if there's a minor calc error, without adding the half pixel offset)
// static const float  c_minorUVOffset = 0.00006f;  // less than 0.5/8192

vaRenderDeviceContextDX11::vaRenderDeviceContextDX11( const vaConstructorParamsBase * params )
{ 
    m_fullscreenVB              = NULL;
    m_fullscreenVBDynamic       = NULL;

    VA_DIRECTX_NOTIFY_TARGET_CONSTRUCT_PATCH( vaRenderDeviceContextDX11 );
}

vaRenderDeviceContextDX11::~vaRenderDeviceContextDX11( )
{ 
    VA_DIRECTX_NOTIFY_TARGET_DESTRUCT_PATCH( vaRenderDeviceContextDX11 );
}

void vaRenderDeviceContextDX11::Initialize( ID3D11DeviceContext * deviceContext )
{
    m_deviceImmediateContext = deviceContext;
    m_deviceImmediateContext->AddRef();
}

void vaRenderDeviceContextDX11::OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain )
{
    // Fullscreen pass
    {
        HRESULT hr;

        CD3D11_BUFFER_DESC desc( 3 * sizeof( CommonSimpleVertex ), D3D11_BIND_VERTEX_BUFFER );

        // using one big triangle
        CommonSimpleVertex fsVertices[3];
        fsVertices[0] = CommonSimpleVertex( -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f );
        fsVertices[1] = CommonSimpleVertex( 3.0f, 1.0f, 0.0f, 1.0f, 2.0f, 0.0f );
        fsVertices[2] = CommonSimpleVertex( -1.0f, -3.0f, 0.0f, 1.0f, 0.0f, 2.0f );

        D3D11_SUBRESOURCE_DATA initSubresData;
        initSubresData.pSysMem = fsVertices;
        initSubresData.SysMemPitch = 0;
        initSubresData.SysMemSlicePitch = 0;
        hr = device->CreateBuffer( &desc, &initSubresData, &m_fullscreenVB );

        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        hr = device->CreateBuffer( &desc, &initSubresData, &m_fullscreenVBDynamic );

        D3D11_INPUT_ELEMENT_DESC inputElements[] =
        {
            { "SV_Position", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        ID3DBlob *pVSBlob = NULL;
        const char * pVSString = "void main( inout const float4 xPos : SV_Position, inout float2 UV : TEXCOORD0 ) { }";

        m_fullscreenVS.CreateShaderAndILFromBuffer( pVSString, "vs_5_0", "main", NULL, inputElements, _countof(inputElements) );
    }

}

void vaRenderDeviceContextDX11::OnDeviceDestroyed( )
{

    SAFE_RELEASE( m_fullscreenVB );
    SAFE_RELEASE( m_fullscreenVBDynamic );

    Destroy();
}

void vaRenderDeviceContextDX11::Destroy( )
{
    SAFE_RELEASE( m_deviceImmediateContext );
}

vaRenderDeviceContext * vaRenderDeviceContextDX11::Create( ID3D11DeviceContext * deviceContext )
{
    vaRenderDeviceContext * canvas = VA_RENDERING_MODULE_CREATE( vaRenderDeviceContext );

    vaSaferStaticCast<vaRenderDeviceContextDX11*>( canvas )->Initialize( deviceContext );

    return canvas;
}

void vaRenderDeviceContextDX11::UpdateViewport( )
{
    const vaViewport & vavp = GetViewport();

    D3D11_VIEWPORT viewport;
    viewport.TopLeftX   = (float)vavp.X;
    viewport.TopLeftY   = (float)vavp.Y;
    viewport.Width      = (float)vavp.Width;
    viewport.Height     = (float)vavp.Height;
    viewport.MinDepth   = vavp.MinDepth;
    viewport.MaxDepth   = vavp.MaxDepth;

    m_deviceImmediateContext->RSSetViewports( 1, &viewport );

    vaVector4i scissorRect;
    bool scissorRectEnabled;
    GetScissorRect( scissorRect, scissorRectEnabled );
    if( scissorRectEnabled )
    {
        D3D11_RECT rect;
        rect.left   = scissorRect.x;
        rect.top    = scissorRect.y;
        rect.right  = scissorRect.z;
        rect.bottom = scissorRect.w;
        m_deviceImmediateContext->RSSetScissorRects( 1, &rect );
    }
    else
    {
        // set the scissor to viewport size, for rasterizer states that have it enabled
        D3D11_RECT rect;
        rect.left   = vavp.X;
        rect.top    = vavp.Y;
        rect.right  = vavp.Width;
        rect.bottom = vavp.Height;
        m_deviceImmediateContext->RSSetScissorRects( 1, &rect );
    }
}

void vaRenderDeviceContextDX11::UpdateRenderTargetsDepthStencilUAVs( )
{
    const std::shared_ptr<vaTexture> & renderTarget = GetRenderTarget();
    const std::shared_ptr<vaTexture> & depthStencil = GetDepthStencil();

    ID3D11RenderTargetView *    RTVs[ c_maxRTs ];
    ID3D11UnorderedAccessView * UAVs[ c_maxUAVs];
    ID3D11DepthStencilView *    DSV = NULL;
    for( size_t i = 0; i < c_maxRTs; i++ )  
        RTVs[i] = ( m_outputsState.RenderTargets[i] != nullptr ) ? ( vaSaferStaticCast<vaTextureDX11*>( m_outputsState.RenderTargets[i].get() )->GetRTV( ) ) : ( nullptr );
    for( size_t i = 0; i < c_maxUAVs; i++ )
        UAVs[i] = ( m_outputsState.UAVs[i] != nullptr ) ? ( vaSaferStaticCast<vaTextureDX11*>( m_outputsState.UAVs[i].get( ) )->GetUAV( ) ) : ( nullptr );

    if( depthStencil != NULL )
        DSV = vaSaferStaticCast<vaTextureDX11*>( depthStencil.get() )->GetDSV( );

    m_deviceImmediateContext->OMSetRenderTargetsAndUnorderedAccessViews( m_outputsState.RenderTargetCount, RTVs, DSV, m_outputsState.UAVsStartSlot, m_outputsState.UAVCount, UAVs, m_outputsState.UAVInitialCounts );
}

void vaRenderDeviceContextDX11::FullscreenPassDraw( ID3D11DeviceContext * context, ID3D11PixelShader * pixelShader, ID3D11BlendState * blendState, ID3D11DepthStencilState * depthStencilState, UINT stencilRef, ID3D11VertexShader * vertexShader, float ZDistance )
{
    if( vertexShader == NULL )
        vertexShader = m_fullscreenVS.GetShader();

    // Topology
    context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

    // Vertex buffer
    const u_int stride = sizeof( CommonSimpleVertex );
    UINT offsetInBytes = 0;
    if( ZDistance != 0.0f )
    {
        // update the vertex buffer dynamically - we should do _NO_OVERWRITE but these fullscreen passes don't happen often enough for it to matter
        HRESULT hr;

        D3D11_MAPPED_SUBRESOURCE subResMap;
        hr = context->Map( m_fullscreenVBDynamic, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResMap );

        CommonSimpleVertex  * fsVertices = (CommonSimpleVertex *)subResMap.pData;
        fsVertices[0] = CommonSimpleVertex( -1.0f, 1.0f, ZDistance, 1.0f, 0.0f, 0.0f );
        fsVertices[1] = CommonSimpleVertex( 3.0f, 1.0f, ZDistance, 1.0f, 2.0f, 0.0f );
        fsVertices[2] = CommonSimpleVertex( -1.0f, -3.0f, ZDistance, 1.0f, 0.0f, 2.0f );

        context->Unmap( m_fullscreenVBDynamic, 0 );

        context->IASetVertexBuffers( 0, 1, &m_fullscreenVBDynamic, &stride, &offsetInBytes );
    }
    else
    {
        context->IASetVertexBuffers( 0, 1, &m_fullscreenVB, &stride, &offsetInBytes );
    }

    // Shaders and input layout
    
    context->IASetInputLayout( m_fullscreenVS.GetInputLayout() );
    context->VSSetShader( vertexShader, NULL, 0 );

    context->PSSetShader( pixelShader, NULL, 0 );

    float blendFactor[4] = { 0, 0, 0, 0 };
    context->OMSetBlendState( blendState, blendFactor, 0xFFFFFFFF );

    context->OMSetDepthStencilState( depthStencilState, stencilRef );

    context->RSSetState( vaDirectXTools::GetRS_CullNone_Fill() );

    context->Draw( 3, 0 );
}


void RegisterCanvasDX11( )
{
    VA_RENDERING_MODULE_REGISTER( vaRenderDeviceContext, vaRenderDeviceContextDX11 );
}
