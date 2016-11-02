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

#include "Core/vaCoreIncludes.h"

#include "Rendering/DirectX/vaDirectXCore.h"
#include "Rendering/DirectX/vaDirectXShader.h"

#include "Rendering/vaRenderMesh.h"

#include "Rendering/DirectX/vaRenderDeviceContextDX11.h"

#include "Rendering/DirectX/Effects/vaSimpleShadowMapDX11.h"

#include "Rendering/DirectX/vaTriangleMeshDX11.h"

#include "Rendering/DirectX/vaRenderMaterialDX11.h"

namespace VertexAsylum
{
    namespace
    {
        typedef vaTriangleMeshDX11<vaRenderMesh::StandardVertex>          StandardTriangleMeshDX11;
    };

    class vaRenderMeshManagerDX11 : public vaRenderMeshManager, VertexAsylum::vaDirectXNotifyTarget
    {
        VA_RENDERING_MODULE_MAKE_FRIENDS( );

    private:

        vaDirectXConstantsBuffer< RenderMeshConstants >
                                                m_constantsBuffer;

    protected:
        vaRenderMeshManagerDX11( const vaConstructorParamsBase * params );
        ~vaRenderMeshManagerDX11( );

    private:
        // vaDirectXNotifyTarget
        virtual void                            OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain );
        virtual void                            OnDeviceDestroyed( );

    private:
        // vaRenderMeshManager implementation
        virtual void                            Draw( vaDrawContext & drawContext, const vaRenderMeshDrawList & list );

    public:
    };

}

using namespace VertexAsylum;


vaRenderMeshManagerDX11::vaRenderMeshManagerDX11( const vaConstructorParamsBase * params )
{
    // m_shadersDirty = true;
    // 
    // m_shaderFileToUse                   = L"vaRenderMesh.hlsl";
    // m_pixelShaderEntry                  = "RenderMeshPS";
    // m_pixelShaderDeferredEntry          = "RenderMeshDeferredPS";
    // m_vertexShaderEntry                 = "RenderMeshVS";
    // 
    // m_staticShaderMacros.push_back( std::pair<std::string, std::string>( "INVALID", "INVALID" ) );

    VA_DIRECTX_NOTIFY_TARGET_CONSTRUCT_PATCH( vaRenderMeshManagerDX11 );
}

vaRenderMeshManagerDX11::~vaRenderMeshManagerDX11( )
{
    VA_DIRECTX_NOTIFY_TARGET_DESTRUCT_PATCH( vaRenderMeshManagerDX11 );
}

void vaRenderMeshManagerDX11::OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain )
{
    m_constantsBuffer.Create( );
}

void vaRenderMeshManagerDX11::OnDeviceDestroyed( )
{
    m_constantsBuffer.Destroy( );
}


void vaRenderMeshManagerDX11::Draw( vaDrawContext & drawContext, const vaRenderMeshDrawList & list )
{
    vaRenderDeviceContextDX11 * apiContext = drawContext.APIContext.SafeCast<vaRenderDeviceContextDX11*>( );
    ID3D11DeviceContext * dx11Context = apiContext->GetDXImmediateContext( );

    assert( drawContext.GetRenderingGlobalsUpdated( ) );    if( !drawContext.GetRenderingGlobalsUpdated( ) ) return;

    vaSimpleShadowMapDX11 * simpleShadowMapDX11 = NULL;
    if( drawContext.SimpleShadowMap != NULL )
        simpleShadowMapDX11 = vaSaferStaticCast<vaSimpleShadowMapDX11*>( drawContext.SimpleShadowMap );

    // make sure we're not overwriting anything else
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, RENDERMESH_TEXTURE_SLOT0 );
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, RENDERMESH_TEXTURE_SLOT1 );
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, RENDERMESH_TEXTURE_SLOT2 );
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, RENDERMESH_TEXTURE_SLOT3 );
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, RENDERMESH_TEXTURE_SLOT4 );
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, RENDERMESH_TEXTURE_SLOT5 );
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, ( ID3D11Buffer* ) nullptr, RENDERMESH_CONSTANTS_BUFFERSLOT );
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, ( ID3D11Buffer* ) nullptr, RENDERMESHMATERIAL_CONSTANTS_BUFFERSLOT );

    // set our main constant buffer
    m_constantsBuffer.SetToD3DContextAllShaderTypes( dx11Context, RENDERMESH_CONSTANTS_BUFFERSLOT );

    // Global API states
    dx11Context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    float blendFactor[4] = { 0, 0, 0, 0 };
    if( drawContext.PassType == vaRenderPassType::ForwardTransparent  )
    {
        dx11Context->OMSetBlendState( vaDirectXTools::GetBS_AlphaBlend( ), blendFactor, 0xFFFFFFFF );
        dx11Context->OMSetDepthStencilState( (drawContext.Camera.GetUseReversedZ())?( vaDirectXTools::GetDSS_DepthEnabledG_NoDepthWrite( ) ):( vaDirectXTools::GetDSS_DepthEnabledL_NoDepthWrite( ) ), 0 );
    }
    else if( drawContext.PassType == vaRenderPassType::ForwardDebugWireframe )
    {
        dx11Context->OMSetBlendState( vaDirectXTools::GetBS_Opaque( ), blendFactor, 0xFFFFFFFF );
        dx11Context->OMSetDepthStencilState( (drawContext.Camera.GetUseReversedZ())?( vaDirectXTools::GetDSS_DepthEnabledGE_NoDepthWrite( ) ):( vaDirectXTools::GetDSS_DepthEnabledLE_NoDepthWrite( ) ), 0 );
    }
    else // all other
    {
        dx11Context->OMSetBlendState( vaDirectXTools::GetBS_Opaque( ), blendFactor, 0xFFFFFFFF );
        dx11Context->OMSetDepthStencilState( (drawContext.Camera.GetUseReversedZ())?( vaDirectXTools::GetDSS_DepthEnabledG_DepthWrite( ) ):( vaDirectXTools::GetDSS_DepthEnabledL_DepthWrite( ) ), 0 );
    }

    // should sort by mesh, then iterate by meshes -> subparts -> draw entries

    for( int i = 0; i < list.Count(); i++ )
    {
        const vaRenderMeshDrawList::Entry & entry = list[i];
        const vaRenderMesh & mesh = *entry.Mesh.get();

        if( mesh.GetTriangleMesh( ) == nullptr )
        {
            assert( false );
            continue;
        }

        // we can only render our own meshes
        assert( static_cast<vaRenderMeshManagerDX11*>( &mesh.GetManager()) == this );

        const vector<vaRenderMesh::SubPart> &   parts = mesh.GetParts();

        StandardTriangleMeshDX11 * triMeshDX11 = mesh.GetTriangleMesh( )->SafeCast<StandardTriangleMeshDX11 *>();
        triMeshDX11->UpdateAndSetToD3DContext( dx11Context );

        // update per-instance constants
        {
            //const Tree::Settings & settings = m_tree.GetSettings( );
            RenderMeshConstants consts;

            consts.World = vaMatrix4x4::FromQuaternion( entry.Rotation );
            consts.World.SetTranslation( entry.Translation );
            consts.World = vaMatrix4x4::Scaling( entry.Scale ) * consts.World;
            consts.Color = entry.Color;

            if( drawContext.PassType != vaRenderPassType::GenerateShadowmap )
            {
                consts.WorldView = consts.World * drawContext.Camera.GetViewMatrix( );
                consts.ShadowWorldViewProj = vaMatrix4x4::Identity;
            }
            else
            {
                assert( drawContext.SimpleShadowMap != NULL );
                consts.WorldView = consts.World * drawContext.SimpleShadowMap->GetViewMatrix( );
                consts.ShadowWorldViewProj = consts.World * drawContext.SimpleShadowMap->GetViewProjMatrix( );
            }
            consts.Color = entry.Color;

            m_constantsBuffer.Update( dx11Context, consts );
        }

        // draw subparts!
        for( size_t subPartIndex = 0; subPartIndex < parts.size(); subPartIndex++ )
        {
//             if( ( (1 << (subPartIndex-1)) & entry.SubPartMask ) == 0 )
//                 continue;

            const vaRenderMesh::SubPart & subPart = parts[subPartIndex];
            assert( (subPart.IndexStart + subPart.IndexCount) <= (int)triMeshDX11->GetIndexCount() );
        
            shared_ptr<vaRenderMaterial> material = subPart.Material.lock();
            if( material == nullptr )
                material = vaRenderMaterialManager::GetInstance().SafeCast<vaRenderMaterialManagerDX11*>()->GetDefaultMaterial( );

            const vaRenderMaterial::MaterialSettings & materialSettings = material->GetSettings();

            CD3D11_RASTERIZER_DESC rasterizerDesc;
            rasterizerDesc.FillMode                 = (drawContext.PassType == vaRenderPassType::ForwardDebugWireframe)?( D3D11_FILL_WIREFRAME ):( D3D11_FILL_SOLID );
            rasterizerDesc.CullMode                 = (materialSettings.FaceCull == vaFaceCull::None)?( D3D11_CULL_NONE ): ( (materialSettings.FaceCull == vaFaceCull::Front)?( D3D11_CULL_FRONT ): ( D3D11_CULL_BACK ) );
            rasterizerDesc.FrontCounterClockwise    = mesh.GetFrontFaceWindingOrder() == vaWindingOrder::CounterClockwise;
            rasterizerDesc.DepthBias                = 0;        // if( drawContext.PassType == vaRenderPassType::GenerateShadowmap ), these will go to whatever there's in simpleShadowMapDX11 
            rasterizerDesc.DepthBiasClamp           = 0;        // if( drawContext.PassType == vaRenderPassType::GenerateShadowmap ), these will go to whatever there's in simpleShadowMapDX11 
            rasterizerDesc.SlopeScaledDepthBias     = 0;        // if( drawContext.PassType == vaRenderPassType::GenerateShadowmap ), these will go to whatever there's in simpleShadowMapDX11 
            rasterizerDesc.DepthClipEnable          = true;                                                                        
            rasterizerDesc.ScissorEnable            = false;
            rasterizerDesc.MultisampleEnable        = false;    // this comes from drawContext
            rasterizerDesc.AntialiasedLineEnable    = false;
            
            if( drawContext.PassType == vaRenderPassType::GenerateShadowmap )
            {
                assert( false );
                // update depth slope biases here
            }
            dx11Context->RSSetState( vaDirectXTools::FindOrCreateRasterizerState( rasterizerDesc ) );
            
            material->UploadToAPIContext( drawContext );

            dx11Context->DrawIndexed( subPart.IndexCount, subPart.IndexStart, 0 );
        }
    }

    // make sure nothing messed with our constant buffers and nothing uses them after
    vaDirectXTools::AssertSetToD3DContextAllShaderTypes( dx11Context, m_constantsBuffer.GetBuffer( ), RENDERMESH_CONSTANTS_BUFFERSLOT );

    // Reset states
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, RENDERMESH_TEXTURE_SLOT0 );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, RENDERMESH_TEXTURE_SLOT1 );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, RENDERMESH_TEXTURE_SLOT2 );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, RENDERMESH_TEXTURE_SLOT3 );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, RENDERMESH_TEXTURE_SLOT4 );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11ShaderResourceView* )nullptr, RENDERMESH_TEXTURE_SLOT5 );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11Buffer* ) nullptr, RENDERMESH_CONSTANTS_BUFFERSLOT );
    vaDirectXTools::SetToD3DContextAllShaderTypes( dx11Context, ( ID3D11Buffer* ) nullptr, RENDERMESHMATERIAL_CONSTANTS_BUFFERSLOT );

    ID3D11ShaderResourceView * nullTextures[4] = { NULL, NULL, NULL, NULL };
    dx11Context->VSSetShader( NULL, NULL, 0 );
    dx11Context->VSSetShaderResources( 0, _countof( nullTextures ), nullTextures );
    dx11Context->PSSetShader( NULL, NULL, 0 );
    dx11Context->PSSetShaderResources( 0, _countof( nullTextures ), nullTextures );
}



void RegisterRenderMeshDX11( )
{
    VA_RENDERING_MODULE_REGISTER( vaRenderMesh::StandardTriangleMesh, StandardTriangleMeshDX11 );
    VA_RENDERING_MODULE_REGISTER( vaRenderMeshManager, vaRenderMeshManagerDX11 );
}

