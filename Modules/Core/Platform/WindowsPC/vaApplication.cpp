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

#include "vaApplication.h"

#include "IntegratedExternals\vaImguiIntegration.h"

#include "Rendering/vaRendering.h"

#include "Rendering/vaAssetPack.h"

#include <timeapi.h>

#include <tpcshrd.h>

using namespace VertexAsylum;

vaApplication * vaApplication::s_instance = NULL;

class vaFPSLimiter : public vaSingletonBase<vaFPSLimiter>
{
private:
    LARGE_INTEGER       m_startTimestamp;
    LARGE_INTEGER       m_frequency;

    double              m_lastTimestamp;
    double              m_prevError;

public:
    vaFPSLimiter()
    {
        QueryPerformanceFrequency( &m_frequency );
        QueryPerformanceCounter( &m_startTimestamp );

        m_lastTimestamp             = 0.0;
        m_prevError                 = 0.0;

        // Set so that ::Sleep below is accurate to within 1ms. This itself can adversely affects battery life on Windows 7 but should not have any impact on 
        // Windows 8 and above; for details please check following article with special attention to "Update, July 13, 2013" part: 
        // at https://randomascii.wordpress.com/2013/07/08/windows-timer-resolution-megawatts-wasted/ 
        timeBeginPeriod( 1 );

    }
    ~vaFPSLimiter()
    {
        timeEndPeriod( 1 );
    }

private:
    double              GetTime( )
    {
        LARGE_INTEGER   currentTime;
        QueryPerformanceCounter( &currentTime );
        
        return (double)(currentTime.QuadPart - m_startTimestamp.QuadPart) / (double)m_frequency.QuadPart;
    }

public:
    void                FramerateLimit( int fpsTarget )
    {
        double deltaTime = GetTime() - m_lastTimestamp;

        double targetDeltaTime = 1.0 / (double)fpsTarget;

        double diffFromTarget = targetDeltaTime - deltaTime + m_prevError;
        if( diffFromTarget > 0.0f )
        {
            double timeToWait = diffFromTarget;

            int timeToSleepMS = (int)(timeToWait * 1000);
            if( timeToSleepMS > 0 )
                Sleep( timeToSleepMS );
        }

        double prevTime = m_lastTimestamp;
        m_lastTimestamp = GetTime();
        double deltaError = targetDeltaTime - (m_lastTimestamp - prevTime);

        // dampen the spring-like effect, but still remain accurate to any positive/negative creep induced by our sleep mechanism
        m_prevError = deltaError * 0.9 + m_prevError * 0.1;

        // clamp error handling to 1 frame length
        if( m_prevError > targetDeltaTime )
            m_prevError = targetDeltaTime;
        if( m_prevError < -targetDeltaTime )
            m_prevError = -targetDeltaTime;
        
        // shift last time by error to compensate
        m_lastTimestamp += m_prevError;
    }
};

vaApplication::Settings::Settings( )
{
    UserOutputWindow = NULL;

    Cursor = NULL;
    Icon = NULL;
    SmallIcon = NULL;
    CmdShow = SW_SHOWDEFAULT;
}

// https://randomascii.wordpress.com/2012/07/05/when-even-crashing-doesnt-work/
static void disable_exception_swallowing( void )
{
    typedef BOOL( WINAPI *tGetPolicy )( LPDWORD lpFlags );
    typedef BOOL( WINAPI *tSetPolicy )( DWORD dwFlags );
    const DWORD EXCEPTION_SWALLOWING = 0x1;

    HMODULE kernel32 = GetModuleHandleA( "kernel32" );
    tGetPolicy pGetPolicy = (tGetPolicy)GetProcAddress( kernel32, "GetProcessUserModeExceptionPolicy" );
    tSetPolicy pSetPolicy = (tSetPolicy)GetProcAddress( kernel32, "SetProcessUserModeExceptionPolicy" );

    if( pGetPolicy && pSetPolicy )
    {
        DWORD dwFlags;
        if( pGetPolicy( &dwFlags ) )
        {
            // Turn off the filter
            pSetPolicy( dwFlags & ~EXCEPTION_SWALLOWING );
        }
    }
}

vaApplication::vaApplication( Settings & settings, std::shared_ptr<vaRenderDevice> renderDevice, const wstring & cmdLine )
    : vaApplicationBase( settings, renderDevice, cmdLine ), m_settings( settings )
{
    m_wndClassName = L"VertexAsylumApp";
    m_hWnd = NULL;

    // if( m_settings.UserOutputWindow != NULL )
    // {
    //     RECT rect;
    //     GetWindowRect( m_settings.UserOutputWindow, &rect );
    // }

    m_systemMenu = 0;

    m_preventWMSIZEResizeSwapChain = false;
    m_inResizeOrMove = false;

    m_cursorHand = 0;
    m_cursorArrow = 0;
    m_cursorNone = 0;

    disable_exception_swallowing( );

    VA_ASSERT( s_instance == NULL, L"There can be only one instance of Application!" );
    s_instance = this;

    new vaFPSLimiter( );
}

vaApplication::~vaApplication( )
{
    delete vaFPSLimiter::GetInstancePtr();

    assert( !m_running );

    VA_ASSERT( s_instance == this, L"Not the active instance?!" );
    s_instance = NULL;
}

void vaApplication::Initialize( )
{
    vaApplicationBase::Initialize();

    {
        WNDCLASSEX wcex;

        wcex.cbSize = sizeof( WNDCLASSEX );

        wcex.style = 0; //CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProcStatic;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = GetModuleHandle( NULL );
        wcex.hIcon = m_settings.Icon;
        wcex.hCursor = m_settings.Cursor;
        wcex.hbrBackground = (HBRUSH)( COLOR_WINDOW + 1 );
        wcex.lpszMenuName = NULL; //MAKEINTRESOURCE(IDC_VANILLA);
        wcex.lpszClassName = m_wndClassName.c_str( );
        wcex.hIconSm = NULL; //LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

        ATOM ret = RegisterClassEx( &wcex );
        assert( ret != 0 ); ret;

        m_hWnd = CreateWindow( m_wndClassName.c_str( ), m_settings.AppName.c_str( ), WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, GetModuleHandle( NULL ), NULL );

        RegisterTouchWindow( m_hWnd, 0 );

        vaWindows::SetMainHWND( m_hWnd );

        assert( m_hWnd != NULL );
        SetWindowClientAreaSize( m_settings.StartScreenWidth, m_settings.StartScreenHeight );

        ShowWindow( m_hWnd, SW_SHOWDEFAULT ); //m_settings.CmdShow );
        UpdateWindow( m_hWnd );

        GetWindowPlacement( m_hWnd, &m_windowPlacement );
    }

    vaLog::GetInstance( ).Add( LOG_COLORS_NEUTRAL, L"vaApplication initialized (%d, %d)", m_settings.StartScreenWidth, m_settings.StartScreenHeight );

    if( m_settings.StartFullscreen )
        ToggleFullscreen( );

    m_renderDevice->CreateSwapChain( m_currentWindowClientSizeX, m_currentWindowClientSizeY, true, m_hWnd );
    //m_renderDevice->CreateSwapChain( 3840, 2160, true, m_hWnd ); //<- don't ask, just for measurements...

    vaRenderingCore::OnAPIInitialized( );

}

void vaApplication::Tick( float deltaTime )
{
    assert( m_initialized );

    if( m_framerateLimit > 0 )
        vaFPSLimiter::GetInstance().FramerateLimit( m_framerateLimit );

    vaApplicationBase::Tick( deltaTime );
}

void vaApplication::CaptureMouse( )
{
    if( IsMouseCaptured( ) )
        return;

    vaInputMouse::GetInstance( ).SetCapture( );

    ::SetCursor( m_cursorNone );

    event_MouseCaptureChanged( );
}

void vaApplication::ReleaseMouse( )
{
    if( !IsMouseCaptured( ) )
        return;

    vaInputMouse::GetInstance( ).ReleaseCapture( );

    event_MouseCaptureChanged( );
}

void vaApplication::UpdateMouseClientWindowRect( )
{
    RECT rc;
    GetClientRect(m_hWnd, &rc); // get client coords
    ClientToScreen(m_hWnd, reinterpret_cast<POINT*>(&rc.left)); // convert top-left
    ClientToScreen(m_hWnd, reinterpret_cast<POINT*>(&rc.right)); // convert bottom-right
    vaInputMouse::GetInstance( ).SetWindowClientRect( rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top );
}

void vaApplication::OnResized( int width, int height, bool windowed )
{
    UpdateMouseClientWindowRect( );
    vaApplicationBase::OnResized( width, height, windowed );
}

#ifdef VA_IMGUI_INTEGRATION_ENABLED
void ImGui_ImplDX11_NewFrame( bool dontTouchMyCursor );
#endif

void vaApplication::Run( )
{
    assert( m_initialized );

    m_running = true;

    MSG msg;

    m_mainTimer.Start( );

    //
    int wmMessagesPerFrame = 0;

    OnAboutToRun( );

#ifdef VA_IMGUI_INTEGRATION_ENABLED
    {
        // works only for monospace fonts: ensure 3 chars are one indent
        // float charAdvanceSize = ImGui::GetWindowFont()->GetCharAdvance(' ');
        // ImGui::GetStyle( ).IndentSpacing = 3 * charAdvanceSize;
    }
#endif

    event_Started( );
    OnResized( m_currentWindowClientSizeX, m_currentWindowClientSizeY, !m_renderDevice->IsFullscreen( ) );

    // Main message loop:
    vaLog::GetInstance( ).Add( LOG_COLORS_NEUTRAL, L"vaApplication entering main loop" );
    while( !m_shouldQuit )
    {
        if( ( wmMessagesPerFrame < 10 ) && ::PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
        {
            if( msg.message == WM_QUIT || msg.message == WM_CLOSE || msg.message == WM_DESTROY )
                m_shouldQuit = true;

            if( ( msg.hwnd == m_hWnd ) && ( msg.message == WM_DESTROY ) )
            {
                m_hWnd = NULL;
            }

            ::TranslateMessage( &msg );

            bool overrideDefault = false;
            //if( ( m_settings.UserOutputWindow != NULL ) && ( msg.hwnd == m_settings.UserOutputWindow ) )
            PreWndProcOverride( msg.hwnd, msg.message, msg.wParam, msg.lParam, overrideDefault );

            if( !overrideDefault )
                ::DispatchMessage( &msg );

            wmMessagesPerFrame++;

        }
        else
        {
            if( m_toggleFullscreenNextFrame )
            {
                ToggleFullscreenInternal();
                m_toggleFullscreenNextFrame = false;
            }
            if( m_setWindowSizeNextFrame.x != 0 && m_setWindowSizeNextFrame.y != 0 )
            {
                SetWindowClientAreaSize( m_setWindowSizeNextFrame.x, m_setWindowSizeNextFrame.y );
                m_setWindowSizeNextFrame = vaVector2i( 0, 0 );
            }

            bool windowOk = UpdateUserWindowChanges( );

            wmMessagesPerFrame = 0;

            if( !windowOk )
            {
                // maybe the window was closed?
                assert( false );
                continue;
            }

            extern bool evilg_inOtherMessageLoop_PreventTick;
            if( evilg_inOtherMessageLoop_PreventTick )
                continue;

            vaProfiler::GetInstance().NewFrame( );

            m_mainTimer.Tick( );

            double totalElapsedTime = m_mainTimer.GetTimeFromStart( );
            totalElapsedTime;
            double deltaTime = m_mainTimer.GetDeltaTime( );

            UpdateFramerateStats( (float)deltaTime );
            if( m_settings.UpdateWindowTitleWithBasicFrameInfo )
            {
                static int tickCounter = 0;
                if( ( tickCounter % 30 ) == 0 )
                {
                    wstring newTitle = m_settings.AppName + L" " + m_basicFrameInfo;
                    ::SetWindowText( m_hWnd, newTitle.c_str() );
                }
                tickCounter++;
            }
            //         vaCore::Tick( (float)deltaTime );

#ifdef VA_IMGUI_INTEGRATION_ENABLED
            ImGui::GetIO( ).DeltaTime = (float)deltaTime;

            // hacky mouse handling, but good for now
            bool dontTouchMyCursor = vaInputMouseBase::GetCurrent( )->IsCaptured( );

            ImGui_ImplDX11_NewFrame( dontTouchMyCursor );
#endif

            Tick( (float)deltaTime );

            m_renderDevice->BeginFrame( (float)deltaTime );

            Render( );

            m_renderDevice->EndAndPresentFrame( (m_vsync)?(1):(0) );

            // if( !m_hasFocus )
            //     Sleep( 30 );
        }
    }
    vaLog::GetInstance( ).Add( LOG_COLORS_NEUTRAL, L"vaApplication main loop closed, exiting..." );
    event_BeforeStopped( );

     // cleanup the message queue just in case
     int safetyBreakNum = 100;
     while( ::PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) && ( safetyBreakNum > 0 ) )
     {
         ::TranslateMessage( &msg );
         ::DispatchMessage( &msg );
         safetyBreakNum--;
     }

    event_Stopped( );
    OnAboutToQuit( );

    vaRenderingCore::OnAPIAboutToBeDeinitialized( );

    m_renderDevice->OnAppAboutToQuit( );

    m_running = false;
}

void vaApplication::Render( )
{
    vaApplicationBase::Render( );
}

void vaApplication::UpdateDeviceSizeOnWindowResize( )
{
    ReleaseMouse( );
    bool windowed = !m_renderDevice->IsFullscreen( );
    event_BeforeWindowResized( );
    if( m_renderDevice->ResizeSwapChain( m_currentWindowClientSizeX, m_currentWindowClientSizeY, true ) )
    {
        OnResized( m_currentWindowClientSizeX, m_currentWindowClientSizeY, windowed );
    }
}

bool vaApplication::IsFullscreen( ) const
{
    DWORD dwStyle = GetWindowLong( m_hWnd, GWL_STYLE );
    
    if( !m_toggleFullscreenNextFrame )
        return ( dwStyle & WS_OVERLAPPEDWINDOW ) == 0;
    else
        return ( dwStyle & WS_OVERLAPPEDWINDOW ) != 0;
}


#ifdef VA_IMGUI_INTEGRATION_ENABLED
IMGUI_API LRESULT ImGui_ImplDX11_WndProcHandler(HWND, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

void vaApplication::ToggleFullscreen( )
{
    m_toggleFullscreenNextFrame = !m_toggleFullscreenNextFrame;
}

void vaApplication::ToggleFullscreenInternal( )
{
    if( !m_renderDevice->IsSwapChainCreated( ) )
        return;
    m_toggleFullscreenNextFrame = false;

    DWORD dwStyle = GetWindowLong( m_hWnd, GWL_STYLE );
    if( !IsFullscreen( ) )
    {
        MONITORINFO mi = { sizeof( mi ) };
        if( GetWindowPlacement( m_hWnd, &m_windowPlacement ) &&
            GetMonitorInfo( MonitorFromWindow( m_hWnd, MONITOR_DEFAULTTOPRIMARY ), &mi ) )
        {
            SetWindowLong( m_hWnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW );
            SetWindowPos( m_hWnd, HWND_TOP,
                mi.rcMonitor.left, mi.rcMonitor.top,
                mi.rcMonitor.right - mi.rcMonitor.left,
                mi.rcMonitor.bottom - mi.rcMonitor.top,
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED );
        }
    }
    else
    {
        SetWindowLong( m_hWnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW );
        SetWindowPlacement( m_hWnd, &m_windowPlacement );
        SetWindowPos( m_hWnd, NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED );
    }
}

void vaApplication::PreWndProcOverride( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool & overrideDefault )
{
    LRESULT ret;
    event_WinProcOverride( overrideDefault, ret, hWnd, message, wParam, lParam );
    if( overrideDefault )
        return;

    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )
    {
    case WM_PAINT:
        hdc = BeginPaint( hWnd, &ps );
        // TODO: Add any drawing code here...
        EndPaint( hWnd, &ps );
        overrideDefault = true;
        break;
    }
}

LRESULT vaApplication::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    vaInputMouse::GetInstance().WndMessage( hWnd, message, wParam, lParam );

    enum {
#define MAKE_COMMAND(c) ((c)<<4)
        FIRST_COMMAND = 1337,
        CMD_ON_TOP = MAKE_COMMAND( FIRST_COMMAND ),
        CMD_TOGGLE_GRAB = MAKE_COMMAND( FIRST_COMMAND + 1 ),
        CMD_TOGGLE_VSYNC = MAKE_COMMAND( FIRST_COMMAND + 2 ),
        CMD_TOGGLE_FS = MAKE_COMMAND( FIRST_COMMAND + 3 ),
#undef MAKE_COMMAND
    };

#ifdef VA_IMGUI_INTEGRATION_ENABLED
    if( m_showIMGUI && !IsMouseCaptured() )
    {
        if( ImGui_ImplDX11_WndProcHandler( hWnd, message, wParam, lParam ) )
        {
            return DefWindowProc( hWnd, message, wParam, lParam );
        }
    }
#endif

    int wmId, wmEvent;

    switch( message )
    {
    case WM_SETCURSOR:
        // this currently never happens since ::SetCapture disables WM_SETCURSOR but leave it in for future possibility
        if( IsMouseCaptured() )
        {
            SetCursor( m_cursorNone );
            //SetWindowLongPtr( hWnd, DWLP_MSGRESULT, TRUE );
            return TRUE;
        }
        else
            return DefWindowProc( hWnd, message, wParam, lParam );

    case WM_CREATE:
        m_cursorHand = LoadCursor( NULL, IDC_HAND );
        m_cursorArrow = LoadCursor( NULL, IDC_ARROW );
        m_systemMenu = GetSystemMenu( hWnd, FALSE );

        {
            int curWidth = 0;
            int curHeight = 0;
            curWidth    = GetSystemMetrics( SM_CXCURSOR );
            curHeight   = GetSystemMetrics( SM_CYCURSOR );

            BYTE * andMask  = new BYTE[curWidth * curHeight];
            BYTE * xorMask   = new BYTE[curWidth * curHeight];
            memset( andMask, 0xFF, curWidth * curHeight );
            memset( xorMask, 0x00, curWidth * curHeight );
            m_cursorNone = ::CreateCursor( GetModuleHandle( NULL ), curWidth/2, curHeight/2, curWidth, curHeight, andMask, xorMask );
            delete[] andMask;
            delete[] xorMask;
        }

        // InsertMenuA( m_systemMenu, 0, 0, CMD_ON_TOP, "On Top\tCtrl+A" );
        return 0;
    case WM_INITMENUPOPUP:
        if( (HMENU)wParam == m_systemMenu )
        {
            return 0;
        }
        break;
    case WM_COMMAND:
        wmId = LOWORD( wParam );
        wmEvent = HIWORD( wParam );
        //// Parse the menu selections:
        //switch (wmId)
        //{
        ////case IDM_ABOUT:
        ////   DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
        ////   break;
        //case IDM_EXIT:
        //   DestroyWindow(hWnd);
        //   break;
        //default:
        //   return DefWindowProc(hWnd, message, wParam, lParam);
        //}
        //break;
    case WM_MOVE:
        UpdateMouseClientWindowRect( );
        break;
    case WM_MOUSEWHEEL:
        vaInputMouse::GetInstance().AccumulateWheelDelta( (float)(GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA) );
        break;
    case WM_DESTROY:
        ::DestroyCursor( m_cursorNone );
    case WM_CLOSE:
        if( !m_shouldQuit )
        {
            m_shouldQuit = true;
            return DefWindowProc( hWnd, message, wParam, lParam );
        }
        break;
    case WM_ENTERSIZEMOVE:
        m_inResizeOrMove = true;
        break;
    case WM_EXITSIZEMOVE:
        if( m_inResizeOrMove )
            UpdateDeviceSizeOnWindowResize( );
        m_inResizeOrMove = false;
        break;
    case WM_ACTIVATE:
    {
        if( wParam == 0 )
            OnLostFocus( );
        else
            OnGotFocus( );
        return 0;
    }
    case WM_KILLFOCUS:
        OnLostFocus( );
        break;
    case WM_SETFOCUS:
        if( m_inResizeOrMove )
            UpdateDeviceSizeOnWindowResize( );
        m_inResizeOrMove = false;
        OnGotFocus( );
        break;
    case WM_KEYDOWN:
        if( wParam == 27 ) // ESC
        {
            m_shouldQuit = true;
            //::PostQuitMessage( 0 );
        }
        break;
    case WM_KEYUP:
        break;
    case WM_SIZE:
    {
        if( m_hWnd != NULL )
        {
            RECT rect;
            ::GetClientRect( m_hWnd, &rect );

            int newWidth    =  rect.right - rect.left;
            int newHeight   =  rect.bottom - rect.top;

            if( ( newWidth != 0 ) && ( newHeight != 0 ) )
            {
                m_currentWindowClientSizeX = newWidth;
                m_currentWindowClientSizeY = newHeight;

                if( !m_preventWMSIZEResizeSwapChain && !m_inResizeOrMove )
                    UpdateDeviceSizeOnWindowResize( );
            }
        }
        else
        {
            int dbg = 0;
            dbg++;
        }
    }
    break;
    case WM_SYSKEYDOWN:
    {        
        if( wParam == VK_RETURN && (m_settings.UserOutputWindow == NULL) )
        {
            ToggleFullscreen( );
        }
        // if( wParam == VK_TAB )
        // {
        //    // move out of fullscreen
        //    // (doesn't work - doesn't ever get called)
        //    int dbg = 0;
        //    dbg++;
        // }
        if( wParam == VK_F4 )
        {
            m_shouldQuit = true;
            //::PostQuitMessage( 0 );
        }
    }
    break;
    case WM_SYSCOMMAND:
    {
        // TODO: make the commands user-configurable?
        int command = wParam & 0xfff0;
        switch( command )
        {
        case CMD_ON_TOP:
            //toggle_always_on_top_GUI( );
            return 0;
        case SC_MAXIMIZE:
            //if( !current_mode.allow_resize )
            //{
            //    toggle_windowed_mode_GUI( );
            //    return 0;
            //}
            break;
//        case SC_CLOSE:
//            m_shouldQuit = true;
//            ::PostQuitMessage( 0 );
//            return 0;
            return 0;
        }

        // let windows do its thing
        // window_in_modal_loop = true;
        // LRESULT result = DefWindowProc( hWnd, WM_SYSCOMMAND, wParam, lParam );
        // window_in_modal_loop = false;
        // 
        // cursor_clip_pending = current_state.prefs.clip_cursor;
        return DefWindowProc( hWnd, message, wParam, lParam );
    } break;

//#define VA_DISABLE_WINDOWED_SIZE_LIMIT
#ifdef VA_DISABLE_WINDOWED_SIZE_LIMIT
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO * lpMinMaxInfo = (MINMAXINFO *)lParam;
        DefWindowProc( hWnd, message, wParam, lParam );
        lpMinMaxInfo->ptMaxSize.x = 8192;
        lpMinMaxInfo->ptMaxSize.y = 8192;
        lpMinMaxInfo->ptMaxTrackSize.x = 8192;
        lpMinMaxInfo->ptMaxTrackSize.y = 8192;
        return 0;
    } break;
#endif // #ifdef VA_DISABLE_WINDOWED_SIZE_LIMIT

// left this in for future use
//    case WM_POINTERUPDATE:  // intentional fallthrough
//    case WM_POINTERDOWN:    // intentional fallthrough
//    case WM_POINTERUP:
//    {
//        WORD pointerId = GET_POINTERID_WPARAM( wParam );
//        POINTER_INFO pointerInfo;
//
//        if( ::GetPointerInfo( pointerId, &pointerInfo ) )
//        {
//            if( pointerInfo.pointerType == PT_TOUCH )
//            {
//                POINT p = pointerInfo.ptPixelLocation;
//                ScreenToClient( hWnd, &p );
//
//                //pointerInfo.dwKeyStates
//
//                bool sendToIMGUI = false;
//                UINT newMessage = 0;
//                UINT newWParam = 0;
//                UINT newLParam = MAKELPARAM( p.x, p.y );
//
//                if( message == WM_POINTERDOWN )
//                {
//                    newMessage = WM_LBUTTONDOWN;
//                    newWParam = MK_LBUTTON;
//                    sendToIMGUI = true;
//                }
//                else if( message == WM_POINTERUP )
//                {
//                    newMessage = WM_LBUTTONUP;
//                    newWParam = 0;
//                    sendToIMGUI = true;
//                }
//                else if( message == WM_POINTERUPDATE )
//                {
//                    bool lbuttondown = IS_POINTER_FIRSTBUTTON_WPARAM( wParam );
//                    newMessage = WM_MOUSEMOVE;
//                    newWParam = ( lbuttondown ) ? ( MK_LBUTTON ) : ( 0 );
//                    sendToIMGUI = true;
//                }
//#ifdef VA_IMGUI_INTEGRATION_ENABLED
//                if( sendToIMGUI && m_showIMGUI && !IsMouseCaptured( ) )
//                {
//                    ImGui_ImplDX11_WndProcHandler( hWnd, newMessage, newWParam, newLParam );
//                    return DefWindowProc( hWnd, message, wParam, lParam );
//                }
//#endif
//            }
//        }
//    }

    default:
        return DefWindowProc( hWnd, message, wParam, lParam );
    }
    return 0;
}

void vaApplication::SetWindowClientAreaSize( int clientSizeX, int clientSizeY )
{
    VA_ASSERT( m_settings.UserOutputWindow == NULL, L"Using user window, this isn't going to work" );

    if( clientSizeX == m_currentWindowClientSizeX && clientSizeY == m_currentWindowClientSizeY )
        return;

    m_currentWindowClientSizeX = clientSizeX;
    m_currentWindowClientSizeY = clientSizeY;

    if( m_hWnd == NULL )
        return;

    LONG style = ::GetWindowLong( m_hWnd, GWL_STYLE );

    RECT rect;
    rect.left = 0; rect.top = 0;
    rect.right = clientSizeX;
    rect.bottom = clientSizeY;
    ::AdjustWindowRect( &rect, (DWORD)style, false );

    RECT wrect;
    ::GetWindowRect( m_hWnd, &wrect );
    ::MoveWindow( m_hWnd, wrect.left, wrect.top, ( rect.right - rect.left ), ( rect.bottom - rect.top ), TRUE );
}

LRESULT CALLBACK vaApplication::WndProcStatic( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    if( s_instance != NULL )
        return vaApplication::s_instance->WndProc( hWnd, message, wParam, lParam );
    else
        return DefWindowProc( hWnd, message, wParam, lParam );
}

bool vaApplication::UpdateUserWindowChanges( )
{
    if( m_settings.UserOutputWindow == NULL ) return true;
    if( m_hWnd == NULL )
        return false;
    assert( m_settings.UserOutputWindow == m_hWnd );

    RECT wrect;
    if( !::GetWindowRect( m_hWnd, &wrect ) )
    {
        return false;
    }
    int width = wrect.right - wrect.left;
    int height = wrect.bottom - wrect.top;

    if( width != m_currentWindowClientSizeX || height != m_currentWindowClientSizeY )
    {
        m_currentWindowClientSizeX = width;
        m_currentWindowClientSizeY = height;
        UpdateDeviceSizeOnWindowResize( );
    }
    return true;
}

