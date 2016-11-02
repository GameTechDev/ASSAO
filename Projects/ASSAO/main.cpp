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
// 2016-09-07: filip.strugar@intel.com: first commit
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Rendering/DirectX/vaRenderDeviceDX11.h"

#include "ASSAODemo.h"

using namespace VertexAsylum;

static std::shared_ptr<vaRenderDevice>      g_renderDevice;
static std::shared_ptr<vaApplication>       g_application;

void RegisterSSAODemoDX11( );
void InitializeProjectAPIParts( )
{
    RegisterSSAODemoDX11( );
}

int APIENTRY _tWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow )
{
    vaCoreInitDeinit core;
    vaRenderingCoreInitDeinit rederingCore;

    {
        InitializeProjectAPIParts();

        vaApplication::Settings settings;
        settings.AppName = L"Adaptive Screen Space Ambient Occlusion";
        //   settings.Instance          = hInstance;
        settings.Cursor = LoadCursor( NULL, IDC_ARROW );
        settings.Icon = 0; //LoadIcon( hInstance, MAKEINTRESOURCE(IDI_VANILLA) );
        settings.SmallIcon = NULL;
        settings.CmdLine = lpCmdLine;
        settings.CmdShow = nCmdShow;
        settings.StartScreenWidth = 1600;
        settings.StartScreenHeight = 900;
        settings.StartFullscreen = false;
        settings.UpdateWindowTitleWithBasicFrameInfo = true;

        //////////////////////////////////////////////////////////////////////////
        // DirectX specific
        std::shared_ptr<vaRenderDevice> renderDevice = std::shared_ptr<vaRenderDevice>( new vaRenderDeviceDX11( nullptr ) );

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// TODO: REMOVE THIS COMMENTED OUT #ifdef _DEBUG FOR THE FINAL VERSION
//#ifdef _DEBUG
        // path to Modules/Windows/Media/Shaders isn't required as the data is embedded, but this compiles the correct file directly for debugging purposes
        vaDirectXShaderManager::GetInstance( ).RegisterShaderSearchPath( vaCore::GetExecutableDirectory( ) + L"../../Modules/Rendering/Media/Shaders" );
//#endif
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        // this is for the project-related shaders
        vaDirectXShaderManager::GetInstance( ).RegisterShaderSearchPath( vaCore::GetExecutableDirectory( ) + L"Media/Shaders" );
        vaRenderingCore::GetInstance( ).RegisterAssetSearchPath( vaCore::GetExecutableDirectory( ) + L"Media/Textures" );
        vaRenderingCore::GetInstance( ).RegisterAssetSearchPath( vaCore::GetExecutableDirectory( ) + L"Media/Meshes" );
        vaDirectXShaderManager::GetInstance( ).RegisterShaderSearchPath( vaCore::GetExecutableDirectory( ) + L"Media/Shaders" );
        // DirectX specific
        //////////////////////////////////////////////////////////////////////////

        std::shared_ptr<vaApplication> application = std::shared_ptr<vaApplication>( new vaApplication( settings, renderDevice, lpCmdLine ) );
        application->Initialize( );

        std::shared_ptr<ASSAODemo> ssaoDemo = std::shared_ptr<ASSAODemo>( VA_RENDERING_MODULE_CREATE( ASSAODemo ) );
        ssaoDemo->Initialize( renderDevice, application );

        application->event_Tick.Connect( ssaoDemo.get( ), &ASSAODemo::OnTick );
        application->event_Render.Connect( ssaoDemo.get( ), &ASSAODemo::OnRender );
        application->event_Started.Connect( ssaoDemo.get( ), &ASSAODemo::OnStarted );
        application->event_BeforeStopped.Connect( ssaoDemo.get( ), &ASSAODemo::OnBeforeStopped );
        application->event_Stopped.Connect( ssaoDemo.get( ), &ASSAODemo::OnStopped );
        application->event_WindowResized.Connect( ssaoDemo.get( ), &ASSAODemo::OnResized );
        application->event_WinProcOverride.Connect( ssaoDemo.get( ), &ASSAODemo::OnWndProcOverride );

        application->Run( );

        // destroy sample object first
        ssaoDemo = NULL;
        // destroy device object first
        renderDevice = NULL;
        // then destroy the app
        application = NULL;
    }
}

