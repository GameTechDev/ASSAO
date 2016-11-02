
#include "ExternalSSAOWrapper.h"

#include "Rendering/DirectX/vaRenderDeviceDX11.h"
#include "Rendering/DirectX/vaRenderDeviceContextDX11.h"
#include "Rendering/DirectX/vaRenderingToolsDX11.h"
#include "Rendering/DirectX/vaTextureDX11.h"

#ifdef SSAO_ALLOW_INTERNAL_SHADER_DEBUGGING
#include "Rendering/DirectX/vaRenderingGlobalsDX11.h"
#endif

//#define USE_HBAOPLUS

#ifdef USE_HBAOPLUS

#include "GFSDK_SSAO.h"

#pragma comment(lib, "GFSDK_SSAO.win64.lib")

#endif


namespace VertexAsylum
{
    class ExternalSSAOWrapperDX11 : public ExternalSSAOWrapper, VertexAsylum::vaDirectXNotifyTarget
    {
        VA_RENDERING_MODULE_MAKE_FRIENDS( );

    private:
        //ID3D11Buffer *                          m_fullscreenVB;
        
#ifdef USE_HBAOPLUS
        GFSDK_SSAO_CustomHeap                   m_customHeap;
        GFSDK_SSAO_Status                       m_status;
        GFSDK_SSAO_Context_D3D11 *              m_context;
        mutable GFSDK_SSAO_Parameters_D3D11     m_params;
#endif 

    public:
        ExternalSSAOWrapperDX11( const vaConstructorParamsBase * params );
        ~ExternalSSAOWrapperDX11( );

    private:
        // vaDirectXNotifyTarget
        virtual void                            OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain );
        virtual void                            OnDeviceDestroyed( );

        void                                    Draw( vaDrawContext & drawContext, ID3D11ShaderResourceView * depthTexture, ID3D11ShaderResourceView * normalmapTexture, bool blend, const vaVector4i & scissorRect );

        virtual void                            Draw( vaDrawContext & drawContext, vaTexture & depthTexture, vaTexture * normalmapTexture, bool blend, const vaVector4i & scissorRect = vaVector4i( 0, 0, 0, 0 ) );


    private:
#ifdef USE_HBAOPLUS
        virtual string                          IHO_GetInstanceInfo( ) const { return "HBAO+"; }
#else
        virtual string                          IHO_GetInstanceInfo( ) const { return "External AO wrapper (placeholder for adding other SSAOs for comparison)"; }
#endif
        virtual void                            IHO_Draw( );


    public:
    };

}

using namespace VertexAsylum;


ExternalSSAOWrapperDX11::ExternalSSAOWrapperDX11( const vaConstructorParamsBase * params )
{
#ifdef USE_HBAOPLUS
#undef new
    m_customHeap.new_ = ::operator new;
    m_customHeap.delete_ = ::operator delete;

    m_params.DepthThreshold.MaxViewDepth = 300.0f;
    m_params.Radius         = 1.2f;
    m_params.DetailAO       = 0.4f;
    m_params.CoarseAO       = 1.5f;
    m_params.Blur.Radius    = GFSDK_SSAO_BLUR_RADIUS_4;
#endif

    VA_DIRECTX_NOTIFY_TARGET_CONSTRUCT_PATCH( ExternalSSAOWrapperDX11 );
}

ExternalSSAOWrapperDX11::~ExternalSSAOWrapperDX11( )
{
    VA_DIRECTX_NOTIFY_TARGET_DESTRUCT_PATCH( ExternalSSAOWrapperDX11 );
}

void ExternalSSAOWrapperDX11::OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain )
{
#ifdef USE_HBAOPLUS

    GFSDK_SSAO_Status status = GFSDK_SSAO_CreateContext_D3D11( device, &m_context, &m_customHeap );
    assert( status == GFSDK_SSAO_OK ); // HBAO+ requires feature level 11_0 or above
#endif
}

void ExternalSSAOWrapperDX11::OnDeviceDestroyed( )
{
#ifdef USE_HBAOPLUS
    m_context->Release();
//    SAFE_RELEASE( m_fullscreenVB );
//    m_constantsBuffer.Destroy( );
#endif
}

void ExternalSSAOWrapperDX11::IHO_Draw( )
{
#ifdef VA_IMGUI_INTEGRATION_ENABLED

#ifdef USE_HBAOPLUS

    if( m_context == nullptr )
    {
        ImGui::Text( "Context not created" );
        return;
    }

    ImGui::PushItemWidth( 160.0f );

    GFSDK_SSAO_Version version = m_context->GetVersion();

    ImGui::Text( "NVIDIA HBAO+, version %d.%d.%d.%d", version.Major, version.Minor, version.Branch, version.Revision );

    ImGui::Text( "GFSDK_SSAO_ params:" );

    ImGui::InputFloat( "Radius", (float*)&m_params.Radius, 0.05f, 0.0f, 2 );
    m_params.Radius = vaMath::Clamp( (float)m_params.Radius, 0.0f, 100.0f );

    ImGui::InputFloat( "Bias", (float*)&m_params.Bias, 0.05f, 0.0f, 2 );
    m_params.Bias = vaMath::Clamp( (float)m_params.Bias, 0.0f, 1.0f );

    ImGui::InputFloat( "DetailAO", (float*)&m_params.DetailAO, 0.05f, 0.0f, 2 );
    m_params.DetailAO = vaMath::Clamp( (float)m_params.DetailAO, 0.0f, 2.0f );

    ImGui::InputFloat( "CoarseAO", (float*)&m_params.CoarseAO, 0.05f, 0.0f, 2 );
    m_params.CoarseAO = vaMath::Clamp( (float)m_params.CoarseAO, 0.0f, 2.0f );

    ImGui::InputFloat( "PowerExponent", (float*)&m_params.PowerExponent, 0.05f, 0.0f, 2 );
    m_params.PowerExponent = vaMath::Clamp( (float)m_params.PowerExponent, 0.0f, 2.0f );

    ImGui::Combo("DepthStorage", (int*)&m_params.DepthStorage, "FP32_VIEW_DEPTHS\0FP16_VIEW_DEPTHS\0\0");

    ImGui::Combo( "DepthClampMode", (int*)&m_params.DepthClampMode, "CLAMP_TO_EDGE\0CLAMP_TO_BORDER\0\0" );

    ImGui::InputInt( "DepthThreshold.Enable", (int*)&m_params.DepthThreshold.Enable, 1 );
    m_params.DepthThreshold.Enable = vaMath::Clamp( (int)m_params.DepthThreshold.Enable, 0, 1 );
    ImGui::InputFloat( "DepthThreshold.MaxViewDepth", (float*)&m_params.DepthThreshold.MaxViewDepth, 0.05f, 0.0f, 2 );
    m_params.DepthThreshold.MaxViewDepth = vaMath::Clamp( (float)m_params.DepthThreshold.MaxViewDepth, 0.0f, 10000.0f );
    ImGui::InputFloat( "DepthThreshold.Sharpness", (float*)&m_params.DepthThreshold.Sharpness, 0.05f, 0.0f, 2 );
    m_params.DepthThreshold.Sharpness = vaMath::Clamp( (float)m_params.DepthThreshold.Sharpness, 0.0f, 10000.0f );

    ImGui::InputInt( "Blur.Enable", (int*)&m_params.Blur.Enable, 1 );
    m_params.Blur.Enable = vaMath::Clamp( (int)m_params.Blur.Enable, 0, 1 );
    ImGui::Combo( "Blur.Radius", (int*)&m_params.Blur.Radius, "BLUR_RADIUS_2\0BLUR_RADIUS_4\0BLUR_RADIUS_8\0\0" );    
    ImGui::InputFloat( "Blur.Sharpness", (float*)&m_params.Blur.Sharpness, 0.2f, 0.0f, 2 );
    m_params.Blur.Sharpness = vaMath::Clamp( (float)m_params.Blur.Sharpness, 0.0f, 16.0f );

    //ImGui::InputFloat( "DepthThreshold.Sharpness", (float*)&m_params.Blur.SharpnessProfile, 0.05f, 0.0f, 2 );

    ImGui::PopItemWidth( );
#else

    ImGui::Text( "Disabled" );
#endif

#endif
}

void ExternalSSAOWrapperDX11::Draw( vaDrawContext & drawContext, ID3D11ShaderResourceView * depthTexture, ID3D11ShaderResourceView* normalmapTexture, bool blend, const vaVector4i & scissorRect )
{
#ifdef USE_HBAOPLUS
    // not sure what to do with scissorRect

    vaRenderDeviceContextDX11 * apiContext = drawContext.APIContext.SafeCast<vaRenderDeviceContextDX11*>( );
    ID3D11DeviceContext * dx11Context = apiContext->GetDXImmediateContext( );

    GFSDK_SSAO_InputData_D3D11 Input;
    Input.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;
    Input.DepthData.pFullResDepthTextureSRV = depthTexture;
    Input.DepthData.ProjectionMatrix.Data = GFSDK_SSAO_Float4x4( &drawContext.Camera.GetProjMatrix( )._11 );
    Input.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
    Input.DepthData.MetersToViewSpaceUnits = 1.0f;

    Input.NormalData.pFullResNormalTextureSRV = normalmapTexture;
    Input.NormalData.Enable = normalmapTexture != nullptr;
    Input.NormalData.DecodeBias = -1.0;
    Input.NormalData.DecodeScale = 2.0f;
    Input.NormalData.WorldToViewMatrix.Data = GFSDK_SSAO_Float4x4( &vaMatrix4x4::Identity._11 ); //drawContext.Camera.GetViewMatrix() );
    Input.NormalData.WorldToViewMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;

    GFSDK_SSAO_Parameters_D3D11 Params = m_params;
    Params.Output.BlendMode = ( blend ) ? ( GFSDK_SSAO_MULTIPLY_RGB ) : ( GFSDK_SSAO_OVERWRITE_RGB );

    // backup currently set RTs
    vaRenderDeviceContext::OutputsState rtState = apiContext->GetOutputs( );
    vaTexture & rtTexture = *rtState.RenderTargets[0];

    GFSDK_SSAO_Status status = m_context->RenderAO( dx11Context, &Input, &Params, rtTexture.SafeCast<vaTextureDX11*>( )->GetRTV( ) );
    assert( status == GFSDK_SSAO_OK );

    // restore previous RTs
    apiContext->SetOutputs( rtState );
#endif
}

void ExternalSSAOWrapperDX11::Draw( vaDrawContext & drawContext, vaTexture & depthTexture, vaTexture * normalmapTexture, bool blend, const vaVector4i & scissorRect )
{
    if( normalmapTexture != nullptr )
        Draw( drawContext, depthTexture.SafeCast<vaTextureDX11*>( )->GetSRV( ), normalmapTexture->SafeCast<vaTextureDX11*>( )->GetSRV( ), blend, scissorRect );
    else
        Draw( drawContext, depthTexture.SafeCast<vaTextureDX11*>( )->GetSRV( ), nullptr, blend, scissorRect );
}

void RegisterExternalSSAOWrapperDX11( )
{
    VA_RENDERING_MODULE_REGISTER( ExternalSSAOWrapper, ExternalSSAOWrapperDX11 );
}
