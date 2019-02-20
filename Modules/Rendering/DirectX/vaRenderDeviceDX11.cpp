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

#include "vaRenderDeviceDX11.h"

#include "Rendering/DirectX/vaTextureDX11.h"
#include "Rendering/DirectX/vaRenderDeviceContextDX11.h"

#include "Rendering/DirectX/vaDirectXTools.h"
#include "Rendering/DirectX/vaDirectXShader.h"
#include "Rendering/DirectX/vaDirectXFont.h"

#include "Rendering/DirectX/vaGPUTimerDX11.h"

#include "IntegratedExternals/vaImguiIntegration.h"

#ifdef VA_IMGUI_INTEGRATION_ENABLED
#include "IntegratedExternals/imgui\DirectX11/imgui_impl_dx11.h"
#endif


using namespace VertexAsylum;

const DXGI_FORMAT                            c_DefaultBackbufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
const int                                    c_DefaultBackbufferCount = 3;
const uint32                                 c_DefaultSwapChainFlags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

typedef HRESULT( WINAPI * LPCREATEDXGIFACTORY )( REFIID, void ** );
typedef HRESULT( WINAPI * LPD3D11CREATEDEVICE )( IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT32, D3D_FEATURE_LEVEL*, UINT, UINT32, ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext** );

static HMODULE                               s_hModDXGI = NULL;
static LPCREATEDXGIFACTORY                   s_DynamicCreateDXGIFactory = NULL;
static HMODULE                               s_hModD3D11 = NULL;
static LPD3D11CREATEDEVICE                   s_DynamicD3D11CreateDevice = NULL;

//vaRenderDeviceDX11 * vaRenderDeviceDX11::s_mainDevice = NULL;

vaRenderDeviceDX11::vaRenderDeviceDX11( const vaConstructorParamsBase * params ) : vaRenderDevice( ), m_canvas2D( )
{
    m_device = NULL;
    m_deviceImmediateContext = NULL;
    m_DXGIFactory = NULL;
    m_swapChain = NULL;
    m_mainOutput = NULL;
    m_mainRenderTargetView = NULL;
    m_mainDepthStencil = NULL;
    m_mainDepthStencilView = NULL;
    m_mainDepthSRV = NULL;

    m_application = NULL;

    m_createdFeatureLevel = D3D_FEATURE_LEVEL_11_0;

    memset( &m_backbufferTextureDesc, 0, sizeof( m_backbufferTextureDesc ) );

    //assert( s_mainDevice == NULL );
    //s_mainDevice = this;

    m_renderFrameCounter = 0;

    bool initialized = Initialize();
    assert( initialized );

    vaDirectXFont::InitializeFontGlobals( );

    //    m_canvas2D = new vaDebugCanvas2DDX11( m_mainViewport );
}

vaRenderDeviceDX11::~vaRenderDeviceDX11( void )
{
    CleanupAPIDependencies( );

    //assert( s_mainDevice == this );
    //s_mainDevice = NULL;
    vaDirectXFont::DeinitializeFontGlobals( );

    Deinitialize( );
    //    if( m_canvas2D != NULL )
    //        delete m_canvas2D;
    //    m_canvas2D = NULL;
}

static void EnsureDirectXAPILoaded( )
{
    if( s_hModDXGI == NULL )
    {
        s_hModDXGI = LoadLibrary( L"dxgi.dll" );
        if( s_hModDXGI == NULL )
            VA_ERROR( L"Unable to load dxgi.dll; Vista SP2, Win7 or above required" );
    }

    if( s_DynamicCreateDXGIFactory == NULL && s_hModDXGI != NULL )
    {
        s_DynamicCreateDXGIFactory = (LPCREATEDXGIFACTORY)GetProcAddress( s_hModDXGI, "CreateDXGIFactory1" );
        if( s_hModDXGI == NULL )
            VA_ERROR( L"Unable to create CreateDXGIFactory1 proc; Vista SP2, Win7 or above required" );
    }

    if( s_hModD3D11 == NULL )
    {
        s_hModD3D11 = LoadLibrary( L"d3d11.dll" );
        if( s_hModD3D11 == NULL )
            VA_ERROR( L"Unable to load d3d11.dll; please install the latest DirectX." );
    }

    if( s_DynamicD3D11CreateDevice == NULL && s_hModD3D11 != NULL )
    {
        s_DynamicD3D11CreateDevice = (LPD3D11CREATEDEVICE)GetProcAddress( s_hModD3D11, "D3D11CreateDevice" );
        if( s_DynamicD3D11CreateDevice == NULL )
            VA_ERROR( L"D3D11CreateDevice proc not found" );
    }
}

// void vaApplication::RegisterShaderSearchPath( const wchar_t * path, bool pushBack )
// {
//     vaDirectXShaderManager::GetInstance( ).RegisterShaderSearchPath( path, pushBack );
// }

bool vaRenderDeviceDX11::Initialize( )
{
    EnsureDirectXAPILoaded( );

    HRESULT hr = S_OK;

    // create DXGI factory
    {
        hr = s_DynamicCreateDXGIFactory( __uuidof( IDXGIFactory1 ), (LPVOID*)&m_DXGIFactory );
        if( FAILED( hr ) )
            VA_ERROR( L"Unable to create DXGIFactory1; Vista SP2, Win7 or above required" );
    }

    vaCOMSmartPtr<IDXGIAdapter1> adapter;

    D3D_DRIVER_TYPE ddt = D3D_DRIVER_TYPE_HARDWARE;
    //D3D_DRIVER_TYPE ddt = D3D_DRIVER_TYPE_WARP;
    //D3D_DRIVER_TYPE ddt = D3D_DRIVER_TYPE_REFERENCE;

    // create IDXGIAdapter1
    {
        int  adapterOrdinal = 0;

        if( ddt == D3D_DRIVER_TYPE_HARDWARE )
        {
            IDXGIAdapter1* pRet;
            hr = m_DXGIFactory->EnumAdapters1( adapterOrdinal, &pRet );
            if( FAILED( hr ) )
                VA_ERROR( L"Error trying to EnumAdapters1" );
            adapter = pRet;
            ddt = D3D_DRIVER_TYPE_UNKNOWN;
        }
        else if( ddt == D3D_DRIVER_TYPE_WARP )
        {
            assert( false );
        }
        else if( ddt == D3D_DRIVER_TYPE_REFERENCE )
        {
            assert( false );
        }
    }

    // create ID3D11Device and ID3D11DeviceContext
   {
       UINT32 flags = 0;

#ifdef _DEBUG 
       flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

       D3D_FEATURE_LEVEL requestedFeatureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

       ID3D11Device *         device = nullptr;
       ID3D11DeviceContext *  deviceImmediateContext = nullptr;

       //s_DynamicD3D11CreateDevice
       hr = s_DynamicD3D11CreateDevice( adapter.Get( ), ddt, NULL, flags, requestedFeatureLevels, _countof( requestedFeatureLevels ), D3D11_SDK_VERSION, &device, &m_createdFeatureLevel, &deviceImmediateContext );

       if( ((flags & D3D11_CREATE_DEVICE_DEBUG) != 0) && FAILED( hr ) )
       {
           VA_WARN( L"Error trying to create D3D11 device, might be because of the debug flag and missing Windows 10 SDK, retrying..." );
           flags &= ~D3D11_CREATE_DEVICE_DEBUG;
           hr = s_DynamicD3D11CreateDevice( adapter.Get( ), ddt, NULL, flags, requestedFeatureLevels, _countof( requestedFeatureLevels ), D3D11_SDK_VERSION, &device, &m_createdFeatureLevel, &deviceImmediateContext );
       }

       if( FAILED( hr ) )
       {
           VA_ERROR( L"Error trying to create D3D11 device" );
           return false;
       }

       m_device = device;
       m_deviceImmediateContext = deviceImmediateContext;

       /*
       if( !SUCCEEDED( device->QueryInterface( IID_ID3D11DeviceContext2, (void**)&m_device ) ) )
       {
           assert( false ); // sorry, this is the min supported
           m_device = nullptr;
       }

       if( !SUCCEEDED( deviceImmediateContext->QueryInterface( IID_ID3D11DeviceContext2, (void**)&m_deviceImmediateContext ) ) )
       {
           assert( false ); // sorry, this is the min supported
           m_deviceImmediateContext = nullptr;
       }
       */

       if( (m_device == nullptr) || (m_deviceImmediateContext == nullptr) )
       {
           VA_ERROR( L"Unable to create DirectX11 device." )
           return false;
       }
   }

   // enumerate outputs
   {
       int outputCount;
       for( outputCount = 0;; ++outputCount )
       {
           IDXGIOutput * pOutput;
           if( FAILED( adapter->EnumOutputs( outputCount, &pOutput ) ) )
               break;
           SAFE_RELEASE( pOutput );
       }

       IDXGIOutput** ppOutputArray = new IDXGIOutput*[outputCount];
       VA_ASSERT( ppOutputArray != NULL, L"Out of memory?" );

       for( int output = 0; output < outputCount; ++output )
       {
           adapter->EnumOutputs( output, &ppOutputArray[output] );
       }

       // select first
       m_mainOutput = ppOutputArray[0];
       m_mainOutput->AddRef( );

       // release the rest
       for( int output = 0; output < outputCount; ++output )
           ppOutputArray[output]->Release( );

       delete[] ppOutputArray;
   }

   DXGI_ADAPTER_DESC1 adapterDesc1;
   adapter->GetDesc1( &adapterDesc1 );

   vaDirectXCore::GetInstance( ).EarlySetDevice( adapterDesc1, m_device );

   // main canvas
   {
       m_mainDeviceContext = std::shared_ptr< vaRenderDeviceContext >( vaRenderDeviceContextDX11::Create( m_deviceImmediateContext ) );
   }

   return true;
}

void vaRenderDeviceDX11::Deinitialize( )
{
    if( m_device != NULL )
    {
#ifdef VA_IMGUI_INTEGRATION_ENABLED
        ImGui_ImplDX11_Shutdown( );
#endif

        vaGPUTimerDX11::OnDeviceAboutToBeDestroyed( );

        ReleaseSwapChainRelatedObjects( );

        // have to release these first!
        m_mainColor = NULL;
        m_mainDepth = NULL;
        m_mainDeviceContext = NULL;

        vaDirectXCore::GetInstance( ).PostDeviceDestroyed( );

        SAFE_RELEASE( m_mainOutput );
        SAFE_RELEASE( m_deviceImmediateContext );
        SAFE_RELEASE( m_swapChain );
        SAFE_RELEASE( m_device );
        SAFE_RELEASE( m_DXGIFactory );
    }
}

void vaRenderDeviceDX11::CreateSwapChain( int width, int height, bool windowed, HWND hwnd )
{
    DXGI_SWAP_CHAIN_DESC desc;
    memset( &desc, 0, sizeof( desc ) );

    desc.BufferDesc.Format = c_DefaultBackbufferFormat;
    desc.BufferDesc.Width = width;
    desc.BufferDesc.Height = height;
    desc.BufferDesc.RefreshRate.Numerator = 1;
    desc.BufferDesc.RefreshRate.Denominator = 60;
    desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    desc.BufferCount = c_DefaultBackbufferCount;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_SHADER_INPUT; //DXGI_USAGE_UNORDERED_ACCESS
    desc.OutputWindow = hwnd;
    desc.Flags = c_DefaultSwapChainFlags;
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    desc.Windowed = windowed;

    if( !windowed )
    {
        DXGI_MODE_DESC closestMatch;
        memset( &closestMatch, 0, sizeof( closestMatch ) );

        HRESULT hr = m_mainOutput->FindClosestMatchingMode( &desc.BufferDesc, &closestMatch, m_device );
        if( FAILED( hr ) )
        {
            VA_ERROR( L"Error trying to find closest matching display mode" );
        }
        desc.BufferDesc = closestMatch;
    }

    //IDXGISwapChain * pSwapChain = NULL;
    HRESULT hr = m_DXGIFactory->CreateSwapChain( m_device, &desc, &m_swapChain ); //&pSwapChain );
    if( FAILED( hr ) )
    {
        VA_ERROR( L"Error trying to create D3D11 swap chain" );
    }

    // stop automatic alt+enter, we'll handle it manually
   {
       hr = m_DXGIFactory->MakeWindowAssociation( hwnd, DXGI_MWA_NO_ALT_ENTER );
   }

   //if( FAILED( d3dResource->QueryInterface( __uuidof( IDXGISwapChain1 ), (void**)&m_swapChain ) ) )
   //{
   //   VA_ERROR( L"Error trying to cast into IDXGISwapChain1" );
   //}
   //SAFE_RELEASE( pSwapChain );

   //   m_swapChain->GetBuffer

   // Broadcast that the device was created!
   vaDirectXCore::GetInstance( ).PostDeviceCreated( m_device, m_swapChain );

   vaGPUTimerDX11::OnDeviceAndContextCreated( m_device );

#ifdef VA_IMGUI_INTEGRATION_ENABLED
    ImGui_ImplDX11_Init( hwnd, m_device, m_deviceImmediateContext );
#endif

    CreateSwapChainRelatedObjects( );

    vaLog::GetInstance( ).Add( LOG_COLORS_NEUTRAL, L"DirectX 11 device and swap chain created" );
}

void vaRenderDeviceDX11::CreateSwapChainRelatedObjects( )
{
    HRESULT hr;

    // Create render target view
    {
        // Get the back buffer and desc
        ID3D11Texture2D* pBackBuffer = NULL;
        hr = m_swapChain->GetBuffer( 0, __uuidof( *pBackBuffer ), (LPVOID*)&pBackBuffer );
        if( FAILED( hr ) )
        {
            VA_ERROR( L"Error trying to get back buffer texture" );
        }
        pBackBuffer->GetDesc( &m_backbufferTextureDesc );

        m_mainRenderTargetView = vaDirectXTools::CreateRenderTargetView( pBackBuffer );

        m_mainColor = std::shared_ptr< vaTexture >( vaTextureDX11::CreateWrap( pBackBuffer ) );

        SAFE_RELEASE( pBackBuffer );
    }

    // Create depth-stencil
   {
       m_mainDepthStencil = vaDirectXTools::CreateTexture2D( DXGI_FORMAT_R32G8X24_TYPELESS, m_backbufferTextureDesc.Width, m_backbufferTextureDesc.Height, NULL, 1, 0, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE );
       m_mainDepthStencilView = vaDirectXTools::CreateDepthStencilView( m_mainDepthStencil, DXGI_FORMAT_D32_FLOAT_S8X24_UINT );
       m_mainDepthSRV = vaDirectXTools::CreateShaderResourceView( m_mainDepthStencil, DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS );

       m_mainDepth = std::shared_ptr< vaTexture >( vaTextureDX11::CreateWrap( m_mainDepthStencil, vaTextureFormat::R32_FLOAT_X8X24_TYPELESS, vaTextureFormat::Unknown, vaTextureFormat::D32_FLOAT_S8X24_UINT ) );

       //delete vaTexture::CreateView( m_mainDepth, vaTextureBindSupportFlags::ShaderResource, vaTextureFormat::R32_FLOAT_X8X24_TYPELESS );
   }

   DXGI_SWAP_CHAIN_DESC scdesc;
   m_swapChain->GetDesc( &scdesc );

   DXGI_SURFACE_DESC sdesc;
   sdesc.Width = scdesc.BufferDesc.Width;
   sdesc.Height = scdesc.BufferDesc.Height;
   sdesc.SampleDesc = scdesc.SampleDesc;
   sdesc.Format = scdesc.BufferDesc.Format;

   m_mainDeviceContext->SetRenderTarget( m_mainColor, m_mainDepth, true );

   //m_mainViewport.X = 0;
   //m_mainViewport.Y = 0;
   //m_mainViewport.Width = scdesc.BufferDesc.Width;
   //m_mainViewport.Height = scdesc.BufferDesc.Height;

   vaDirectXCore::GetInstance( ).PostResizedSwapChain( sdesc );

#ifdef VA_IMGUI_INTEGRATION_ENABLED
   ImGui_ImplDX11_CreateDeviceObjects();
#endif
}

void vaRenderDeviceDX11::ReleaseSwapChainRelatedObjects( )
{
#ifdef VA_IMGUI_INTEGRATION_ENABLED
    ImGui_ImplDX11_InvalidateDeviceObjects();
#endif

    vaDirectXCore::GetInstance( ).PostReleasingSwapChain( );

    SAFE_RELEASE( m_mainRenderTargetView );
    SAFE_RELEASE( m_mainDepthStencil );
    SAFE_RELEASE( m_mainDepthStencilView );
    SAFE_RELEASE( m_mainDepthSRV );

    m_mainDeviceContext->SetRenderTarget( NULL, NULL, false );
    m_mainColor = NULL;
    m_mainDepth = NULL;
}

bool vaRenderDeviceDX11::IsFullscreen( )
{
    BOOL fullscreen = FALSE;
    if( m_swapChain != NULL )
    {
        m_swapChain->GetFullscreenState( &fullscreen, NULL );
    }

    return fullscreen != FALSE;
}

void vaRenderDeviceDX11::FindClosestFullscreenMode( int & width, int & height )
{
    assert( m_mainOutput != NULL );

    DXGI_MODE_DESC desc;

    desc.Format = c_DefaultBackbufferFormat;
    desc.Width = width;
    desc.Height = height;
    desc.RefreshRate.Numerator = 1;
    desc.RefreshRate.Denominator = 60;
    desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

    DXGI_MODE_DESC closestMatch;
    memset( &closestMatch, 0, sizeof( closestMatch ) );

    HRESULT hr = m_mainOutput->FindClosestMatchingMode( &desc, &closestMatch, m_device );
    if( FAILED( hr ) )
        VA_ERROR( L"Error trying to find closest matching display mode" );

    width = closestMatch.Width;
    height = closestMatch.Height;
}

bool vaRenderDeviceDX11::ResizeSwapChain( int width, int height, bool windowed )
{
    if( m_swapChain == NULL ) return false;
    if( (int)m_backbufferTextureDesc.Width == width && (int)m_backbufferTextureDesc.Height == height && windowed == !IsFullscreen( ) ) return false;

    if( (int)m_backbufferTextureDesc.Width != width || (int)m_backbufferTextureDesc.Height != height )
    {
        ReleaseSwapChainRelatedObjects( );

        HRESULT hr = m_swapChain->ResizeBuffers( c_DefaultBackbufferCount, width, height, DXGI_FORMAT_UNKNOWN, c_DefaultSwapChainFlags );
        if( FAILED( hr ) )
        {
            assert( false );
            //VA_ERROR( L"Error trying to m_swapChain->ResizeBuffers" );
            return false;
        }

        CreateSwapChainRelatedObjects( );
        return true;
    }

    //if( windowed != !IsFullscreen( ) )
    //{
    //    HRESULT hr = m_swapChain->SetFullscreenState( !windowed, NULL );
    //    if( FAILED( hr ) )
    //    {
    //        //VA_ERROR( L"Error trying to m_swapChain->SetFullscreenState" );
    //    }
    //
    //    IsFullscreen( );
    //    return true;
    //}
    return true;
}

void vaRenderDeviceDX11::SetMainRenderTargetToImmediateContext( )
{
    m_mainDeviceContext->SetRenderTarget( m_mainColor, m_mainDepth, true );
}

void vaRenderDeviceDX11::RecompileFileLoadedShaders( )
{
    vaDirectXShaderManager::GetInstance( ).RecompileFileLoadedShaders( );
}

void vaRenderDeviceDX11::BeginFrame( float deltaTime )
{
    m_renderFrameCounter++;

    if( (vaInputKeyboardBase::GetCurrent() != NULL) && vaInputKeyboardBase::GetCurrent()->IsKeyDown( KK_CONTROL ) )
    {
        if( vaInputKeyboardBase::GetCurrent()->IsKeyClicked( ( vaKeyboardKeys )'R' ) )
            vaDirectXShaderManager::GetInstance( ).RecompileFileLoadedShaders( );
    }

    m_directXCore.TickInternal( );

    SetMainRenderTargetToImmediateContext( );

//    // maybe not needed here?
//    vaDirectXTools::ClearColorDepthStencil( m_deviceImmediateContext, true, true, true, vaVector4( 0.0f, 0.5f, 0.0f, 0.0f ), 1.0f, 0 );

    vaGPUTimerDX11::OnFrameStart();
}

void vaRenderDeviceDX11::DrawDebugCanvas2D( )
{
    m_canvas2D.Draw( m_deviceImmediateContext );
    m_canvas2D.CleanQueued( );
}

void vaRenderDeviceDX11::DrawDebugCanvas3D( vaDrawContext & drawContext )
{
    m_canvas3D.Draw( drawContext, drawContext.Camera.GetViewMatrix() * drawContext.Camera.GetProjMatrix() );
    m_canvas3D.CleanQueued( );
}

void vaRenderDeviceDX11::EndAndPresentFrame( int vsyncInterval )
{
    vaGPUTimerDX11::OnFrameEnd( );

    UINT flags = 0;

    HRESULT hr = m_swapChain->Present( (UINT)vsyncInterval, flags );
    if( FAILED( hr ) )
    {
        assert( false );
    }
}

wstring vaRenderDeviceDX11::GetAdapterNameShort( )
{
    return vaDirectXCore::GetInstance().GetAdapterNameShort();
}

wstring vaRenderDeviceDX11::GetAdapterNameID( )
{
    return vaDirectXCore::GetInstance().GetAdapterNameID();
}