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

#include "vaInputKeyboard.h"

namespace VertexAsylum
{
    // this needs refactoring, cleaning up and renaming to vaInputMouseWin32 and moving all the platform independent stuff to vaInputMouseBase
    class vaInputMouse : public vaSingletonBase < vaInputMouse >, public vaInputMouseBase
    {
    public:
        static const int        c_InitPriority = vaInputKeyboard::c_InitPriority;
        static const int        c_TickPriority = vaInputKeyboard::c_TickPriority + 1;

    protected:

        // this is where the platform dependent input goes; it's then processed in Tick
        bool                    m_platformInputKeys[MK_MaxValue];
        //vaVector2i              m_platformInputPos;                 // in screen space

        // these so much need renaming
        bool                    g_Keys[MK_MaxValue];
        bool                    g_KeyUps[MK_MaxValue];
        bool                    g_KeyDowns[MK_MaxValue];

        bool                    m_firstPass;

        bool                    m_captured;
        vaVector2i              m_capturedPos;
        vaVector2i              m_capturedWinCenterPos;

        vaVector2i              m_prevPos;
        vaVector2i              m_currPos;
        vaVector2i              m_deltaPos;

        float                   m_wheelDelta;
        
        vaVector2i              m_windClientPos;
        vaVector2i              m_windClientSize;

        float                   m_timeFromLastMove;

    private:
        friend class vaWindowsDirectX;
        friend class vaApplicationBase;
        friend class vaApplication;
        vaInputMouse( );
        ~vaInputMouse( );

    protected:
        void                        Tick( float deltaTime );

    public:
        virtual bool                IsKeyDown( vaMouseKeys key ) const          { return g_Keys[key]; }
        virtual bool                IsKeyClicked( vaMouseKeys key ) const       { return g_KeyDowns[key]; }
        virtual bool                IsKeyReleased( vaMouseKeys key ) const      { return g_KeyUps[key]; }
        //
        virtual vaVector2i          GetCursorPos( ) const                       { return m_currPos; }
        virtual vaVector2i          GetCursorClientPos( ) const                 { return m_currPos - m_windClientPos; }
        virtual vaVector2           GetCursorClientNormalizedPos( ) const       { return vaVector2::ComponentDiv( vaVector2( m_currPos - m_windClientPos ), vaVector2( m_windClientSize ) ); }
        virtual vaVector2i          GetCursorDelta( ) const                     { return m_deltaPos; }
        //
        virtual float               GetWheelDelta( ) const                      { return m_wheelDelta; }
        //
        virtual float               TimeFromLastMove( ) const                   { return m_timeFromLastMove; }
        //
        virtual bool                IsCaptured( ) const                         { return m_captured; }
        //
    protected:
        void                        ResetAll( );
        //
        void                        SetCapture( );
        void                        ReleaseCapture( );
        //
        void                        AccumulateWheelDelta( float wheelDelta )                    { m_wheelDelta += wheelDelta; }
        void                        ResetWheelDelta( )                                          { m_wheelDelta = 0.0f; }
        void                        SetWindowClientRect( int x, int y, int width, int height )  { m_windClientPos.x = x; m_windClientPos.y = y; m_windClientSize.x = width; m_windClientSize.y = height; }
        //
        //
    public:
        void                        WndMessage( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
    };

}