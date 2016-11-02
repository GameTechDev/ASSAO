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

#include "vaApplicationBase.h"

#include "IntegratedExternals\vaImguiIntegration.h"

#include "Rendering\vaRendering.h"

#include "Rendering/vaAssetPack.h"

#include "vaInputKeyboard.h"
#include "vaInputMouse.h"

//#include <windows.h>

using namespace VertexAsylum;

vaApplicationBase::Settings::Settings( )
{
    AppName = L"Name me plz";
    CmdLine = L"";

    StartScreenWidth = 1280;
    StartScreenHeight = 720;
    StartFullscreen = false;

    AllowFullscreen = true;

    UpdateWindowTitleWithBasicFrameInfo = false;

    Vsync = false;

    FramerateLimit = 0;
}

vaApplicationBase::vaApplicationBase( Settings & settings, std::shared_ptr<vaRenderDevice> renderDevice, const wstring & cmdLine )
    : m_settings( settings ), m_renderDevice( renderDevice )
{
    m_initialized = false;

    m_currentWindowClientSizeX = 0;
    m_currentWindowClientSizeY = 0;

    for( int i = 0; i < _countof( m_frametimeHistory ); i++ ) m_frametimeHistory[i] = 0.0f;
    m_frametimeHistoryLast = 0;
    m_avgFramerate = 0.0f;
    m_avgFrametime = 0.0f;
    m_accumulatedDeltaFrameTime = 0.0f;

    m_shouldQuit = false;
    m_running = false;

    if( !m_settings.AllowFullscreen && m_settings.StartFullscreen )
    {
        VA_ASSERT_ALWAYS( L"Incompatible vaApplication::Settings" );
    }

    m_hasFocus = false;

    m_showIMGUI = true;

    m_blockInput    = false;

    m_cmdLineParams = vaStringTools::SplitCmdLineParams( cmdLine );

    m_toggleFullscreenNextFrame = false;
    m_setWindowSizeNextFrame = vaVector2i( 0, 0 );

    m_vsync             = m_settings.Vsync;
    m_framerateLimit    = m_settings.FramerateLimit;
}

vaApplicationBase::~vaApplicationBase( )
{
    assert( !m_running );
}

void vaApplicationBase::Quit( )
{
    assert( m_running );

    m_shouldQuit = true;
}

namespace VertexAsylum
{
#ifdef VA_IMGUI_INTEGRATION_ENABLED
    void ImSetBigClearSansRegular( ImFont * font );
    void ImSetBigClearSansBold(    ImFont * font );
#endif`
}

void vaApplicationBase::Initialize( )
{
#ifdef VA_IMGUI_INTEGRATION_ENABLED
    {
        ImGuiIO& io = ImGui::GetIO();
        
        io.Fonts->AddFontDefault();

        // this would be a good place for DPI scaling.
        float bigSizePixels = 26.0f;
        float displayOffset = -1.0f;

        ImFontConfig fontConfig;

        vaFileTools::EmbeddedFileData fontFileData;

        fontFileData = vaFileTools::EmbeddedFilesFind( wstring( L"fonts:\\ClearSans-Regular.ttf" ) );
        if( fontFileData.HasContents( ) )
        {
            void * imguiBuffer = ImGui::MemAlloc( (int)fontFileData.MemStream->GetLength() );
            memcpy( imguiBuffer, fontFileData.MemStream->GetBuffer(), (int)fontFileData.MemStream->GetLength() );

            ImFont * font = io.Fonts->AddFontFromMemoryTTF( imguiBuffer, (int)fontFileData.MemStream->GetLength(), bigSizePixels, &fontConfig );
            font->DisplayOffset.y += displayOffset;   // Render 1 pixel down
            ImSetBigClearSansRegular( font );
        }

        fontFileData = vaFileTools::EmbeddedFilesFind( wstring( L"fonts:\\ClearSans-Bold.ttf" ) );
        if( fontFileData.HasContents( ) )
        {
            void * imguiBuffer = ImGui::MemAlloc( (int)fontFileData.MemStream->GetLength() );
            memcpy( imguiBuffer, fontFileData.MemStream->GetBuffer(), (int)fontFileData.MemStream->GetLength() );

            ImFont * font = io.Fonts->AddFontFromMemoryTTF( imguiBuffer, (int)fontFileData.MemStream->GetLength(), bigSizePixels, &fontConfig );
            font->DisplayOffset.y += displayOffset;   // Render 1 pixel down
            ImSetBigClearSansBold( font );
        }

    }
#endif
    m_initialized = true;
}

void vaApplicationBase::Tick( float deltaTime )
{
    if( m_blockInput && IsMouseCaptured() )
        this->ReleaseMouse();

    if( m_hasFocus )
    {
        vaInputMouse::GetInstance( ).Tick( deltaTime );
        vaInputKeyboard::GetInstance( ).Tick( deltaTime );
    }
    else
    {
        vaInputMouse::GetInstance( ).ResetAll( );
        vaInputKeyboard::GetInstance( ).ResetAll( );
    }

    // must be a better way to do this
    if( m_blockInput )
    {
        vaInputMouse::GetInstance( ).ResetAll();
        vaInputKeyboard::GetInstance( ).ResetAll();
    }

    if( !m_blockInput && HasFocus() && vaInputMouse::GetInstance( ).IsKeyClicked( MK_Right ) )
    {
        if( IsMouseCaptured( ) )
            this->ReleaseMouse( );
        else
            this->CaptureMouse( );
    }

    // this should probably go somewhere out but.. when need be.
    if( vaInputKeyboardBase::GetCurrent()->IsKeyClicked( KK_F1 ) )
        m_showIMGUI = !m_showIMGUI;

    event_Tick( deltaTime );

    vaInputMouse::GetInstance( ).ResetWheelDelta( );
}

bool vaApplicationBase::IsMouseCaptured( ) const
{
    return vaInputMouse::GetInstance( ).IsCaptured( );
}

void vaApplicationBase::OnResized( int width, int height, bool windowed )
{
    assert( width == m_currentWindowClientSizeX );
    assert( height == m_currentWindowClientSizeY );

    event_WindowResized( width, height, windowed );
}

void vaApplicationBase::Render( )
{
    assert( m_initialized );

    VA_SCOPE_CPUGPU_TIMER( Frame, *m_renderDevice->GetMainContext( ) );

    event_Render();
}

void vaApplicationBase::UpdateFramerateStats( float deltaTime )
{
    m_frametimeHistoryLast = ( m_frametimeHistoryLast + 1 ) % _countof( m_frametimeHistory );

    // add this frame's time to the accumulated frame time
    m_accumulatedDeltaFrameTime += deltaTime;

    // remove oldest frame time from the accumulated frame time
    m_accumulatedDeltaFrameTime -= m_frametimeHistory[m_frametimeHistoryLast];

    m_frametimeHistory[m_frametimeHistoryLast] = deltaTime;

    m_avgFrametime = ( m_accumulatedDeltaFrameTime / (float)_countof( m_frametimeHistory ) );
    m_avgFramerate = 1.0f / m_avgFrametime;

    float avgFramerate = GetAvgFramerate( );
    float avgFrametimeMs = GetAvgFrametime( ) * 1000.0f;
    m_basicFrameInfo = vaStringTools::Format( L"%.2fms/frame avg (%.2fFPS, %dx%d)", avgFrametimeMs, avgFramerate, m_currentWindowClientSizeX, m_currentWindowClientSizeY );
#ifdef _DEBUG
    m_basicFrameInfo += L" DEBUG";
#endif
}

void vaApplicationBase::OnGotFocus( )
{
    vaInputKeyboard::GetInstance( ).ResetAll( );
    vaInputMouse::GetInstance( ).ResetAll( );

    m_hasFocus = true;
}

void vaApplicationBase::OnLostFocus( )
{
    vaInputKeyboard::GetInstance( ).ResetAll( );
    vaInputMouse::GetInstance( ).ResetAll( );
    
    m_hasFocus = false;
}

void vaApplicationBase::HelperUIDraw( )
{
#ifdef VA_IMGUI_INTEGRATION_ENABLED

    HelperUIFlags uiFlags = m_helperUISettings.Flags;

    if( HasFocus( ) && !vaInputMouseBase::GetCurrent( )->IsCaptured( ) )
    {
        vaInputKeyboardBase & keyboard = *vaInputKeyboardBase::GetCurrent( );
        if( keyboard.IsKeyClicked( KK_OEM_3 ) )
        {
            m_helperUISettings.ConsoleLogOpen = !m_helperUISettings.ConsoleLogOpen;
        }
    }

    bool rightWindow = ( ((int)HelperUIFlags::ShowStatsGraph | (int)HelperUIFlags::ShowResolutionOptions |  (int)HelperUIFlags::ShowGPUProfiling | (int)HelperUIFlags::ShowCPUProfiling | (int)HelperUIFlags::ShowAssetsInfo ) & (int)uiFlags ) != 0;
    // info stuff on the right
    if( rightWindow )
    {
        int sizeX = 450;
        int sizeY = 550;
        ImGui::SetNextWindowPos( ImVec2( (float)( m_currentWindowClientSizeX - sizeX - 6.0f ), 6.0f ), ImGuiSetCond_Always );     // Normally user code doesn't need/want to call it because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
        ImGui::SetNextWindowSize( ImVec2( (float)sizeX, (float)sizeY ), ImGuiSetCond_Once );
        ImGui::SetNextWindowCollapsed( true, ImGuiSetCond_FirstUseEver );
        if( ImGui::Begin( "Application", 0, ImVec2(0.f, 0.f), 0.7f, 0 ) )
        {
            ImVec4 fpsInfoColor = ImVec4( 1.0f, 1.0f, 0.0f, 1.0f );

            // Adopting David Bregman's FPS info style
            //ImGui::TextColored( fpsInfoColor, frameInfo.c_str( ) );

            string frameInfo = vaStringTools::SimpleNarrow( GetBasicFrameInfoText() );

            // graph (there is some CPU/drawing cost to this)
            if( (int)HelperUIFlags::ShowStatsGraph & (int)uiFlags )
            {
                float frameTimeMax = 0.0f;
                float frameTimeMin = FLT_MAX;
                float frameTimesMS[_countof( m_frametimeHistory )];
                float frameTimeAvg = 0.0f;
                for( int i = 0; i < _countof( m_frametimeHistory ); i++ )
                {
                    frameTimesMS[i] = m_frametimeHistory[(i+m_frametimeHistoryLast+1) % _countof( m_frametimeHistory )] * 1000.0f;
                    frameTimeMax = vaMath::Max( frameTimeMax, frameTimesMS[i] );
                    frameTimeMin = vaMath::Min( frameTimeMin, frameTimesMS[i] );
                    frameTimeAvg += frameTimesMS[i];
                }
                frameTimeAvg /= (float)_countof( m_frametimeHistory );

                static float avgFrametimeGraphMax = 1.0f;
                avgFrametimeGraphMax = vaMath::Lerp( avgFrametimeGraphMax, frameTimeMax * 1.5f, 0.05f );
                avgFrametimeGraphMax = vaMath::Min( 1000.0f, vaMath::Max( avgFrametimeGraphMax, frameTimeMax * 1.1f ) );

                float graphWidth = sizeX - 110.0f;

                ImGui::PushStyleColor( ImGuiCol_Text, fpsInfoColor );
                ImGui::PushStyleColor( ImGuiCol_PlotLines, ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
                ImGui::PlotLines( "", frameTimesMS, _countof(frameTimesMS), 0, frameInfo.c_str(), 0.0f, avgFrametimeGraphMax, ImVec2( graphWidth, 100.0f ) );
                ImGui::PopStyleColor( 2 );

                ImGui::SameLine( );
                
                ImGui::BeginGroup( );
                {
                    ImGui::Text( "\nmin: %.2f", frameTimeMin );
                    ImGui::Text( "\nmax: %.2f", frameTimeMax );
                    ImGui::Text( "\navg: %.2f", frameTimeAvg );
                }
                ImGui::EndGroup( );
            }

            if( (int)HelperUIFlags::ShowResolutionOptions & (int)uiFlags )
            {
                ImGui::Separator( );

                {
                    bool fullscreen = IsFullscreen( );

                    int ws[2] = { m_currentWindowClientSizeX, m_currentWindowClientSizeY };
                    if( ImGui::InputInt2( "Window size", ws, ImGuiInputTextFlags_EnterReturnsTrue | ( ( IsFullscreen( ) ) ? ( ImGuiInputTextFlags_ReadOnly ) : ( 0 ) ) ) )
                    {
                        if( ( ws[0] != m_currentWindowClientSizeX ) || ( ws[1] != m_currentWindowClientSizeY ) )
                        {
                            m_setWindowSizeNextFrame = vaVector2i( ws[0], ws[1] );
                        }
                    }

                    bool wasFullscreen = fullscreen;
                    ImGui::Checkbox( "Fullscreen (borderless windowed)", &fullscreen );
                    if( wasFullscreen != fullscreen )
                        ToggleFullscreen( );
                    ImGui::Checkbox( "Vsync", &m_vsync );
                }
            }

            if( (int)HelperUIFlags::ShowGPUProfiling & (int)uiFlags )
            {
                ImGui::Separator( );

                if( ImGui::CollapsingHeader( "GPU Profiling", ImGuiTreeNodeFlags_Framed | ((m_helperUISettings.GPUProfilerDefaultOpen)?(ImGuiTreeNodeFlags_DefaultOpen):(0)) ) )
                {
                    vaProfiler::GetInstance( ).DisplayImGui( false );
                }
            }
            if( (int)HelperUIFlags::ShowCPUProfiling & (int)uiFlags )
            {
                ImGui::Separator( );

                if( ImGui::CollapsingHeader( "CPU Profiling", ImGuiTreeNodeFlags_Framed | ((m_helperUISettings.CPUProfilerDefaultOpen)?(ImGuiTreeNodeFlags_DefaultOpen):(0)) ) )
                {
                    vaProfiler::GetInstance( ).DisplayImGui( true );
                }
            }

            if( (int)HelperUIFlags::ShowAssetsInfo & (int)uiFlags )
            {
                ImGui::Separator( );

                vaImguiHierarchyObject::DrawCollapsable( vaAssetPackManager::GetInstance(), true, false, false );
            }
        }
        ImGui::End( );
    }

    // console & log at the bottom
    if( (int)HelperUIFlags::ShowConsoleWindow & (int)uiFlags )
    {
        vaCriticalSectionScopeLock lock( vaLog::GetInstance( ).CriticalSection( ) );
        const float secondsToShow = 6.0f;
        const int linesToShowMax = 10;

        int showFrom = vaLog::GetInstance( ).FindNewest( secondsToShow );

        if( m_helperUISettings.ConsoleLogOpen )
            showFrom = (int)vaLog::GetInstance( ).Entries( ).size( ) - linesToShowMax - 1; // vaMath::Max( 0, (int)vaLog::GetInstance( ).Entries( ).size( ) - linesToShowMax - 1 );

        int showCount = vaMath::Min( linesToShowMax, (int)vaLog::GetInstance( ).Entries( ).size( ) - showFrom );
        showFrom = (int)vaLog::GetInstance( ).Entries( ).size( ) - showCount;

        
        //////////////////////////////////////////////////////////////////////////
        // not implemented yet
        int keyboardUpDownOffset = 0;
        if( m_helperUISettings.ConsoleLogOpen && HasFocus( ) && !vaInputMouseBase::GetCurrent( )->IsCaptured( ) )
        {
            vaInputKeyboardBase & keyboard = *vaInputKeyboardBase::GetCurrent( );

            if( keyboard.IsKeyClicked( KK_NEXT ) )
                keyboardUpDownOffset += linesToShowMax;
            if( keyboard.IsKeyClicked( KK_PRIOR ) )
                keyboardUpDownOffset -= linesToShowMax;
            if( keyboard.IsKeyClicked( KK_UP ) )
                keyboardUpDownOffset -= 1;
            if( keyboard.IsKeyClicked( KK_DOWN ) )
                keyboardUpDownOffset += 1;
        }
        // not implemented yet
        //////////////////////////////////////////////////////////////////////////


        //if( showCount > 0 )
        {
            int sizeX               = m_currentWindowClientSizeX - 10;
            int sizeY               = (int)(ImGui::GetTextLineHeightWithSpacing() * (float)showCount + 10);
            ImGui::SetNextWindowPos( ImVec2( (float)( m_currentWindowClientSizeX/2 - sizeX/2), (float)( m_currentWindowClientSizeY - sizeY) - 6 ), ImGuiSetCond_Always );     // Normally user code doesn't need/want to call it because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
            ImGui::SetNextWindowSize( ImVec2( (float)sizeX, (float)sizeY ), ImGuiSetCond_Always );
            ImGui::SetNextWindowCollapsed( false, ImGuiSetCond_Always );
            bool opened = true;
            ImGuiWindowFlags winFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoInputs;
            winFlags |= ImGuiWindowFlags_NoScrollbar; //?

            float winAlpha = (m_helperUISettings.ConsoleLogOpen)?(0.92f):(0.55f);
            if( showCount == 0 )
                winAlpha = 0.0f;
            ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( 0.0f, 0.0f, 0.0f, 1.0f ) );

            if( ImGui::Begin( "Console", &opened, ImVec2(0.f, 0.f), winAlpha, winFlags ) )
            {
                if( showCount > 0 )
                {
                    // for( int i = 0; i < linesToShowMax - showCount; i++ )
                    //     ImGui::Text( "" );

                    ImGui::Columns( 2 );
                    float scrollbarSize = 24.0f;
                    ImGui::SetColumnOffset( 1, (float)sizeX - scrollbarSize );

                    float timerSeparatorX = 80.0f;

                    for( int i = showFrom; i < (int)vaLog::GetInstance().Entries().size(); i++ )
                    {
                        if( i < 0  )
                        {
                            ImGui::Text("");
                            continue;
                        }
                        vaLog::Entry entry = vaLog::GetInstance().Entries()[i];
                        float lineCursorPosY = ImGui::GetCursorPosY( );

                        char buff[64];
                        #pragma warning ( suppress: 4996 )
                        strftime( buff, sizeof(buff), "%H:%M:%S: ", localtime( &entry.LocalTime ) );
                        ImGui::TextColored( ImVec4( 0.3f, 0.3f, 0.2f, 1.0f ), buff );

                        ImGui::SetCursorPosX( timerSeparatorX );
                        ImGui::SetCursorPosY( lineCursorPosY );
                        ImGui::TextColored( ImFromVA( entry.Color ), vaStringTools::SimpleNarrow( entry.Text ).c_str( ) );
                    }

                    ImGui::NextColumn();

                    int smax = vaMath::Max( 1, (int)vaLog::GetInstance().Entries().size() - showFrom );
                    int smin = 0;
                    int currentPos = smax - smax; // inverted? todo, this is weird
                    ImGui::VSliderInt( "", ImVec2( scrollbarSize-16, (float)sizeY - 16 ), &currentPos, smin, smax, " " );
                }
            }
            ImGui::End( );

            ImGui::PopStyleColor( 1 );
        }
    }

#endif
}

