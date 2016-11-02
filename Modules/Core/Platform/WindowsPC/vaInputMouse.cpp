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

#include "vaInputMouse.h"
#include "vaApplication.h"

using namespace VertexAsylum;

//static HCURSOR s_defaultCursorHandle = NULL;
//static HCURSOR s_prevCursorHandle    = NULL;

vaInputMouse::vaInputMouse( )
{
    m_captured = false;

//    CURSORINFO cinfo;
//    memset( &cinfo, 0, sizeof( cinfo ) );
//    cinfo.cbSize = sizeof( cinfo );
//    ::GetCursorInfo( &cinfo );
//    s_defaultCursorHandle = cinfo.hCursor;

    ResetAll( );

    vaInputMouseBase::SetCurrent( this );
}

vaInputMouse::~vaInputMouse( )
{
    assert( vaInputMouseBase::GetCurrent( ) == this );
    vaInputMouseBase::SetCurrent( NULL );
}

// vaSystemManagerSingletonBase<vaInputMouse>
void vaInputMouse::Tick( float deltaTime )
{
    for( int i = 0; i < MK_MaxValue; i++ )
    {
        //bool isDown = ( GetAsyncKeyState( i ) & 0x8000 ) != 0;
        bool isDown = m_platformInputKeys[i];

        g_KeyUps[i] = g_Keys[i] && !isDown;
        g_KeyDowns[i] = !g_Keys[i] && isDown;

        g_Keys[i] = isDown;
    }

    CURSORINFO cinfo; cinfo.cbSize = sizeof( cinfo );
    BOOL ret = ::GetCursorInfo( &cinfo );
    if( !ret )
    {
        ResetAll( );
        return;
    }
    assert( ret ); ret;
    m_prevPos = m_currPos;
    m_currPos = vaVector2i( cinfo.ptScreenPos.x, cinfo.ptScreenPos.y );
    m_deltaPos = m_currPos - m_prevPos;

    m_timeFromLastMove += deltaTime;
    if( m_prevPos != m_currPos )
        m_timeFromLastMove = 0.0f;

    if( m_captured )
    {
        // return to centre
        ::SetCursorPos( m_capturedWinCenterPos.x, m_capturedWinCenterPos.y );
        m_prevPos = m_currPos = m_capturedWinCenterPos;
    }

    if( m_firstPass )
    {
        m_firstPass = false;
        Tick( deltaTime );
    }
}

void vaInputMouse::ResetAll( )
{
    if( m_captured )
        ReleaseCapture();

    m_firstPass = true;

    m_prevPos = vaVector2i( 0, 0 );
    m_currPos = vaVector2i( 0, 0 );
    m_deltaPos = vaVector2i( 0, 0 );

    m_captured = false;
    m_capturedPos = vaVector2i( 0, 0 );
    m_capturedWinCenterPos = vaVector2i( 0, 0 );

    m_wheelDelta = 0;

//    m_platformInputPos = vaVector2i( 0, 0 );

    for( int i = 0; i < MK_MaxValue; i++ )
    {
        m_platformInputKeys[i] = false;
        g_Keys[i] = false;
        g_KeyUps[i] = false;
        g_KeyDowns[i] = false;
    }
    ReleaseCapture( );

    m_timeFromLastMove = false;
}

void vaInputMouse::SetCapture( )
{
    if( m_captured )
        return;

    m_captured = true;

//    int cursorDisplayCounter = ::ShowCursor( FALSE );
//    if( cursorDisplayCounter >= 0 )
//        ::ShowCursor( FALSE );

//    HCURSOR prevCursor = ::SetCursor( NULL );
//    if( prevCursor != NULL )
//        s_prevCursorHandle = prevCursor;

    POINT oldPos;
    ::GetCursorPos( &oldPos );
    m_capturedPos = vaVector2i( oldPos.x, oldPos.y );

    RECT dwr;
    ::GetWindowRect( vaApplication::GetInstance( ).GetMainHWND( ), &dwr );
    vaVector2i winCentre;
    m_capturedWinCenterPos.x = ( dwr.left + dwr.right ) / 2;
    m_capturedWinCenterPos.y = ( dwr.top + dwr.bottom ) / 2;

    ::SetCapture( vaApplication::GetInstance( ).GetMainHWND( ) );
    ::SetCursorPos( m_capturedWinCenterPos.x, m_capturedWinCenterPos.y );
    m_prevPos = m_currPos = m_capturedWinCenterPos;

}

void vaInputMouse::ReleaseCapture( )
{
    if( !m_captured )
        return;

    m_captured = false;

    ::SetCursorPos( m_capturedPos.x, m_capturedPos.y );
    m_prevPos = m_currPos = m_capturedPos;
    ::ReleaseCapture( );
}

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
#endif

void vaInputMouse::WndMessage( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    // coming from touch, ignore
    if( ( GetMessageExtraInfo( ) & 0x82 ) == 0x82 )
        return;

    if( (message >= WM_MOUSEFIRST) && (message <= WM_MOUSELAST) )
    {
        POINT pt;
        pt.x = GET_X_LPARAM( lParam );
        pt.y = GET_Y_LPARAM( lParam );

        if( hWnd != NULL )
            ::ClientToScreen( hWnd, &pt );

        // m_platformInputPos = vaVector2i( pt.x, pt.y );

        m_platformInputKeys[MK_Left]      = ( wParam & MK_LBUTTON ) != 0;
        m_platformInputKeys[MK_Right]     = ( wParam & MK_RBUTTON ) != 0;
        m_platformInputKeys[MK_Middle]    = ( wParam & MK_MBUTTON ) != 0;
        m_platformInputKeys[MK_XButton1]  = ( wParam & MK_XBUTTON1 ) != 0;
        m_platformInputKeys[MK_XButton2]  = ( wParam & MK_XBUTTON2 ) != 0;
    }
}

