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


namespace VertexAsylum
{
    class vaCameraBase;

    class vaCameraControllerBase
    {
    protected:
        vaCameraControllerBase() {}
    
    public:
        virtual ~vaCameraControllerBase() {}

    protected:
        friend class vaCameraBase;

        virtual void                                        CameraAttached( vaCameraBase & camera )         { }
        virtual void                                        CameraTick( float deltaTime, vaCameraBase & camera, bool hasFocus )  { }
    };

    class vaCameraControllerFreeFlight : public vaCameraControllerBase
    {

    protected:
        float                   m_yaw;
        float                   m_pitch;
        float                   m_roll;

        // a reference for yaw pitch roll calculations: default is X is forward, Z is up, Y is right
        const vaMatrix4x4       m_baseOrientation;

        float                   m_accumMouseDeltaX;
        float                   m_accumMouseDeltaY;
        vaVector3               m_accumMove;
        float                   m_rotationSpeed;
        float                   m_movementSpeed;
        float                   m_inputSmoothingLerpK;

        float                   m_movementSpeedAccelerationModifier;

        bool                    m_moveWhileNotCaptured;

    private:

    public:
        vaCameraControllerFreeFlight( );
        virtual  ~vaCameraControllerFreeFlight( );

    protected:
        virtual void                                        CameraAttached( vaCameraBase & camera );
        virtual void                                        CameraTick( float deltaTime, vaCameraBase & camera, bool hasFocus );

    public:
        void                                                SetMoveWhileNotCaptured( bool moveWhileNotCaptured )    { m_moveWhileNotCaptured = moveWhileNotCaptured; }
        bool                                                GetMoveWhileNotCaptured( )                              { return m_moveWhileNotCaptured; }
    };

    class vaCameraControllerFocusLocationsFlythrough : public vaCameraControllerBase
    {
    
    public:
        struct Keyframe
        {
            vaVector3               Position;
            vaQuaternion            Orientation;
            float                   ShowTime;
            float                   UserParam0;
            float                   UserParam1;

            Keyframe( const vaVector3 & position, const vaQuaternion & orientation, float showTime, float userParam0 = 0.0f, float userParam1 = 0.0f ) : Position( position ), Orientation( orientation ), ShowTime( showTime ), UserParam0( userParam0 ), UserParam1( userParam1 ) { }
        };

    protected:
        std::vector<Keyframe>       m_keys;
        int                         m_currentKeyIndex;
        float                       m_currentKeyTimeRemaining;

        float                       m_userParam0;
        float                       m_userParam1;

        bool                        m_fixedUp;
        vaVector3                   m_fixedUpVec;

    private:

    public:
        vaCameraControllerFocusLocationsFlythrough( );
        virtual  ~vaCameraControllerFocusLocationsFlythrough( );

        void                                                AddKey( const Keyframe & key )                          { m_keys.push_back( key ); }
        void                                                ResetTime( )                                            { m_currentKeyIndex = -1; m_currentKeyTimeRemaining = 0.0f; }
        
        void                                                SetFixedUp( bool enabled, vaVector3 & upVec = vaVector3( 0.0f, 0.0f, 1.0f ) )   { m_fixedUp = enabled; m_fixedUpVec = upVec; }

    protected:
        virtual void                                        CameraAttached( vaCameraBase & camera );
        virtual void                                        CameraTick( float deltaTime, vaCameraBase & camera, bool hasFocus );
    };

}