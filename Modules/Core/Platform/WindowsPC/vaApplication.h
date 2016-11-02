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

#include "Core/vaApplicationBase.h"

#include "vaInputKeyboard.h"
#include "vaInputMouse.h"

namespace VertexAsylum
{

    class vaApplication : public vaApplicationBase
    {
    private:
        static vaApplication *              s_instance;

    public:
        struct Settings : vaApplicationBase::Settings
        {
            // if not NULL, output will be redirected to this window
            HWND            UserOutputWindow;
             
            // if OutputWindow == NULL a new window will be created with following settings:
            HCURSOR         Cursor;
            HICON           Icon;
            HICON           SmallIcon;
            int             CmdShow;

            Settings( );
        };

    protected:
        const Settings                      m_settings;

        wstring                             m_wndClassName;
        HWND                                m_hWnd;

        HMENU                               m_systemMenu;

        HCURSOR                             m_cursorHand;
        HCURSOR                             m_cursorArrow;
        HCURSOR                             m_cursorNone;

        WINDOWPLACEMENT                     m_windowPlacement;

        bool                                m_preventWMSIZEResizeSwapChain;
        bool                                m_inResizeOrMove;

        vaInputKeyboard                     m_keyboard;
        vaInputMouse                        m_mouse;

    public:
        vaApplication( Settings & settings, std::shared_ptr<vaRenderDevice> renderDevice, const wstring & cmdLine );
        virtual ~vaApplication( );

    public:
        virtual void                        Initialize( );
        //
    public:
        // run the main loop!
        virtual void                        Run( );
        //
    public:
        const vaSystemTimer &               GetMainTimer( ) const                   { return m_mainTimer; }
        void                                SetWindowClientAreaSize( int clientSizeX, int clientSizeY );
        //        //
        float                               GetAvgFramerate( ) const                { return m_avgFramerate; }
        float                               GetAvgFrametime( ) const                { return m_avgFrametime; }
        //
        virtual void                        CaptureMouse( );
        virtual void                        ReleaseMouse( );
        //     
        virtual bool                        IsFullscreen( ) const;
        virtual void                        SetFullscreen( bool fullscreen )        { if( fullscreen != IsFullscreen() ) ToggleFullscreen(); }
        virtual void                        ToggleFullscreen( );
        //
    protected:
        void                                UpdateMouseClientWindowRect( );
        //
    public:
        static vaApplication &              GetInstance( )                          { return *s_instance; }
        //
    protected:
        //virtual void                        OnGotFocus( );
        //virtual void                        OnLostFocus( );

        virtual void                        OnAboutToRun( )                         { }
        virtual void                        OnAboutToQuit( )                        { }

        virtual void                        Tick( float deltaTime );
        virtual void                        Render( );

        virtual void                        OnResized( int width, int height, bool windowed );

        void                                ToggleFullscreenInternal( );

    public:

        HWND                                GetMainHWND( ) const                    { return m_hWnd; }

    protected:
        virtual void                        PreWndProcOverride( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool & overrideDefault );
        virtual LRESULT                     WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

    private:
        static LRESULT CALLBACK             WndProcStatic( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

        void                                UpdateDeviceSizeOnWindowResize( );
        //
        bool                                UpdateUserWindowChanges( );

    };

}