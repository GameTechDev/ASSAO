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

#include "Core/vaCoreIncludes.h"

#include "Rendering/vaRenderDevice.h"

namespace VertexAsylum
{

    // this signal stuff is pretty old, not sure if it should be updated 
    class vaApplicationSignals
    {
    public:
        vaSignal0< void >                   event_Started;
        vaSignal0< void >                   event_Stopped;
        vaSignal0< void >                   event_BeforeStopped;
        vaSignal0< void >                   event_MouseCaptureChanged;

        vaSignal0< void >                   event_BeforeWindowResized;              // width, height, fullscreen
        vaSignal3< int, int, bool >         event_WindowResized;                    // width, height, fullscreen

        vaSignal1< float >                  event_Tick;
        vaSignal0< void >                   event_Render;

        //static LRESULT CALLBACK WndProcStatic( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
        typedef LRESULT & LRESULTREF;
        typedef bool & boolRef;
        vaSignal6< boolRef, LRESULTREF, HWND, UINT, WPARAM, LPARAM >
                                            event_WinProcOverride;
    };

    class vaApplicationBase : public vaApplicationSignals
    {
    public:
        struct Settings
        {
            wstring         AppName;
            wstring         CmdLine;

            // // if not NULL, output will be redirected to this window
            // HWND            UserOutputWindow;
            // 
            // // if OutputWindow == NULL a new window will be created with following settings:
            // HCURSOR         Cursor;
            // HICON           Icon;
            // HICON           SmallIcon;
            //int             CmdShow;
            int             StartScreenWidth;
            int             StartScreenHeight;
            bool            StartFullscreen;

            bool            AllowFullscreen;

            bool            UpdateWindowTitleWithBasicFrameInfo;

            bool            Vsync;

            int             FramerateLimit;

            Settings( );
        };

        enum class HelperUIFlags
        {
            None                    = 0,
            ShowStatsGraph          = ( 1 << 1 ),
            ShowResolutionOptions   = ( 1 << 2 ),
            ShowGPUProfiling        = ( 1 << 3 ),
            ShowCPUProfiling        = ( 1 << 4 ),
            ShowAssetsInfo          = ( 1 << 5 ),
            ShowConsoleWindow       = ( 1 << 6 ),
       
        };

        struct HelperUISettings
        {
            HelperUIFlags           Flags;
            bool                    GPUProfilerDefaultOpen;
            bool                    CPUProfilerDefaultOpen;
            bool                    ConsoleLogOpen;
            int                     ConsoleVSrollPos;   // functionality not implemented yet

            HelperUISettings( )
            {
                Flags = HelperUIFlags(
                (int)HelperUIFlags::ShowStatsGraph           | 
                (int)HelperUIFlags::ShowResolutionOptions    | 
                (int)HelperUIFlags::ShowGPUProfiling         |
                (int)HelperUIFlags::ShowCPUProfiling         |
                (int)HelperUIFlags::ShowAssetsInfo           |
                (int)HelperUIFlags::ShowConsoleWindow );
                
                GPUProfilerDefaultOpen  = false;
                CPUProfilerDefaultOpen  = false;
                ConsoleLogOpen          = false;
                ConsoleVSrollPos     = 0;
            }
        };

    protected:
        const Settings                      m_settings;
        HelperUISettings                    m_helperUISettings;

        bool                                m_initialized;

        std::shared_ptr<vaRenderDevice>     m_renderDevice;

        vaSystemTimer                       m_mainTimer;
        //
        int                                 m_currentWindowClientSizeX;
        int                                 m_currentWindowClientSizeY;
        //
        static const int                    c_framerateHistoryCount = 96;
        float                               m_frametimeHistory[c_framerateHistoryCount];
        int                                 m_frametimeHistoryLast;
        float                               m_avgFramerate;
        float                               m_avgFrametime;
        float                               m_accumulatedDeltaFrameTime;
        //
        bool                                m_running;
        bool                                m_shouldQuit;

        bool                                m_hasFocus;

        bool                                m_showIMGUI;

        bool                                m_blockInput;

        std::vector<vaCmdLineParam>         m_cmdLineParams;

        wstring                             m_basicFrameInfo;

        // used for delayed switch to fullscreen or window size change
        bool                                m_toggleFullscreenNextFrame;
        vaVector2i                          m_setWindowSizeNextFrame;

        bool                                m_vsync;
        
        int                                 m_framerateLimit;

    protected:
        vaApplicationBase( Settings & settings, std::shared_ptr<vaRenderDevice> renderDevice, const wstring & cmdLine );
        virtual ~vaApplicationBase( );

    public:
        virtual void                        Initialize( )  ;
        //
    protected:
        void                                UpdateFramerateStats( float deltaTime );
        //
    public:
        // run the main loop!
        virtual void                        Run( )                                  = 0;
        // quit the app (will finish current frame)
        void                                Quit( );
        //
    public:
        const vaSystemTimer &               GetMainTimer( ) const                   { return m_mainTimer; }
        void                                SetWindowClientAreaSize( int clientSizeX, int clientSizeY );
        //        //
        float                               GetAvgFramerate( ) const                { return m_avgFramerate; }
        float                               GetAvgFrametime( ) const                { return m_avgFrametime; }
        //
        virtual void                        CaptureMouse( )                         = 0;
        virtual void                        ReleaseMouse( )                         = 0;
        virtual bool                        IsMouseCaptured( ) const;
        //
        const Settings &                    GetSettings( ) const                    { return m_settings; }
        HelperUISettings &                  HelperUISettings( )                     { return m_helperUISettings; }
        //
        virtual bool                        IsFullscreen( ) const                   = 0;
        virtual void                        ToggleFullscreen( )                     = 0;

        bool                                HasFocus( ) const                       { return m_hasFocus; }
        
        bool                                IsIMGUIVisible( ) const                 { return m_showIMGUI; }
        void                                SetIMGUIVisible( bool visible )         { m_showIMGUI = visible; }

        bool                                IsInputBlocked( ) const                 { return m_blockInput; }
        void                                SetBlockInput( bool blockInput )        { m_blockInput = blockInput; }

        const std::vector<vaCmdLineParam> & GetCommandLineParameters( ) const       { return m_cmdLineParams; }

        // Use HelperUISettings for controlling the way it looks
        void                                HelperUIDraw( );

        const wstring &                     GetBasicFrameInfoText( )                { return m_basicFrameInfo; }

        std::shared_ptr<vaRenderDevice>     GetRenderDevice( )                      { return m_renderDevice; }

        bool                                GetVsync( ) const                       { return m_vsync; }
        void                                SetVsync( bool vsync )                  { m_vsync = vsync; }

        int                                 GetFramerateLimit( ) const              { return m_framerateLimit; }
        void                                SetFramerateLimit( int fpsLimit )       { m_framerateLimit = fpsLimit; }

        bool                                GetConsoleOpen( ) const                 { return m_helperUISettings.ConsoleLogOpen; }
        void                                SetConsoleOpen( bool consoleOpen )      { m_helperUISettings.ConsoleLogOpen = consoleOpen; }

    protected:
        virtual void                        OnGotFocus( );
        virtual void                        OnLostFocus( );

        virtual void                        OnAboutToRun( )                         { }
        virtual void                        OnAboutToQuit( )                        { }

        virtual void                        Tick( float deltaTime );
        virtual void                        Render( );

        virtual void                        OnResized( int width, int height, bool windowed );

    private:
        //
        bool                                UpdateUserWindowChanges( );

    };

}