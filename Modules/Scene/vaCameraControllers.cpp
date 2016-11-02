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

#include "vaCameraControllers.h"

#include "vaCameraBase.h"

using namespace VertexAsylum;


vaCameraControllerFreeFlight::vaCameraControllerFreeFlight( )
    // orient the camera so that X is forward, Z is up, Y is right
    : m_baseOrientation( vaMatrix4x4::RotationZ( VA_PIf * 0.5f ) * vaMatrix4x4::RotationY( VA_PIf * 0.5f ) )
{
//   m_hasFocus              = true; // temporary
   m_accumMouseDeltaX      = 0.0f;
   m_accumMouseDeltaY      = 0.0f;
   m_accumMove             = vaVector3( 0.0f, 0.0f, 0.0f );
   m_rotationSpeed         = 0.5f;
   m_movementSpeed         = 20.0f;
   m_inputSmoothingLerpK   = 200.0f;
   m_yaw                   = 0.0f;
   m_pitch                 = 0.0f; // look towards x
   m_roll                  = 0.0f; // y is right
   m_movementSpeedAccelerationModifier = 0.0f;
   m_moveWhileNotCaptured   = true;
}

vaCameraControllerFreeFlight::~vaCameraControllerFreeFlight( )
{

}

void vaCameraControllerFreeFlight::CameraAttached( vaCameraBase & camera )
{ 
    vaMatrix4x4 debasedOrientation = m_baseOrientation.Inverse() * vaMatrix4x4::FromQuaternion( camera.GetOrientation() );
    debasedOrientation.DecomposeRotationYawPitchRoll( m_yaw, m_pitch, m_roll );

    m_roll = 0;
}

void vaCameraControllerFreeFlight::CameraTick( float deltaTime, vaCameraBase & camera, bool hasFocus )
{
    if( ( vaInputMouseBase::GetCurrent( ) == NULL ) || ( vaInputKeyboardBase::GetCurrent( ) == NULL ) )
        return;

   vaVector3       objectPos = camera.GetPosition();
   vaQuaternion    objectOri = camera.GetOrientation();

   vaInputMouseBase & mouse       = *vaInputMouseBase::GetCurrent();
   vaInputKeyboardBase & keyboard = *vaInputKeyboardBase::GetCurrent( );

   if( hasFocus )
   {
      float smoothingLerpK = vaMath::TimeIndependentLerpF( deltaTime, m_inputSmoothingLerpK );
      float speedBoost = (keyboard.IsKeyDown( KK_SHIFT ))?(12.0f):(1.0f);
      speedBoost *= keyboard.IsKeyDown( KK_CONTROL )?(0.08f):(1.0f);

      //
      ///////////////////////////////////////////////////////////////////////////
      // Update camera range/speed changes
      if( keyboard.IsKeyDown( KK_SUBTRACT ) )      m_movementSpeed *= 0.95f;
      if( keyboard.IsKeyDown( KK_ADD ) )           m_movementSpeed *= 1.05f;
      m_movementSpeed = vaMath::Clamp( m_movementSpeed, 1.0f, 5000.0f );
      //if( vaIsKeyClicked( kcViewRangeDown ) )     m_viewRange *= 0.99f;
      //if( vaIsKeyClicked( kcViewRangeUp ) )       m_viewRange *= 1.01f;
      //m_viewRange = vaMath::Clamp( m_viewRange, 1000.0f, 1000000.0f );
      //
      ///////////////////////////////////////////////////////////////////////////
      // Update camera rotation
      vaVector2 cdelta = (vaVector2)mouse.GetCursorDelta( ) * m_rotationSpeed;
      //
      // smoothing
      {
         m_accumMouseDeltaX += cdelta.x;
         m_accumMouseDeltaY += cdelta.y;
         cdelta.x = smoothingLerpK * m_accumMouseDeltaX;
         cdelta.y = smoothingLerpK * m_accumMouseDeltaY;
         m_accumMouseDeltaX = (1 - smoothingLerpK) * m_accumMouseDeltaX;
         m_accumMouseDeltaY = (1 - smoothingLerpK) * m_accumMouseDeltaY;
      }
      //
      // Rotate
      if( mouse.IsCaptured( ) )
      {
          if( mouse.IsKeyDown( MK_Middle ) )
              m_roll    -= cdelta.x * 0.005f;
          else
            m_yaw		+= cdelta.x * 0.005f;

         m_pitch	   += cdelta.y * 0.003f;

         m_yaw		   = vaMath::AngleWrap( m_yaw );
         m_pitch	   = vaMath::Clamp( m_pitch, -(float)VA_PIf/2 + 1e-1f, +(float)VA_PIf/2 - 1e-1f );
         m_roll        = vaMath::AngleWrap( m_roll );
      }
      //
      vaMatrix4x4 cameraWorld = vaMatrix4x4::RotationYawPitchRoll( m_yaw, m_pitch, m_roll );
      //
      // Move
      if( mouse.IsCaptured() || m_moveWhileNotCaptured )
      {
         ///////////////////////////////////////////////////////////////////////////
         // Update camera movement
         bool hasInput = keyboard.IsKeyDown( (vaKeyboardKeys)'W' ) || keyboard.IsKeyDown( (vaKeyboardKeys)'S' ) || keyboard.IsKeyDown( (vaKeyboardKeys)'A' ) || 
                        keyboard.IsKeyDown( (vaKeyboardKeys)'D' ) || keyboard.IsKeyDown( (vaKeyboardKeys)'Q' ) || keyboard.IsKeyDown( (vaKeyboardKeys)'E' );

         m_movementSpeedAccelerationModifier = (hasInput)?(vaMath::Min(m_movementSpeedAccelerationModifier + deltaTime * 0.5f, 1.0f)):(0.0f);
         float moveSpeed = m_movementSpeed * deltaTime * ( 0.3f + 0.7f * m_movementSpeedAccelerationModifier ) * speedBoost;

         vaVector3       forward( cameraWorld.GetAxisX() );
         vaVector3       right( cameraWorld.GetAxisY() );
         vaVector3       up( cameraWorld.GetAxisZ() );

         vaVector3 accumMove = m_accumMove;

         if( keyboard.IsKeyDown( (vaKeyboardKeys)'W' ) || keyboard.IsKeyDown( KK_UP )    )     accumMove += forward * moveSpeed;
         if( keyboard.IsKeyDown( (vaKeyboardKeys)'S' ) || keyboard.IsKeyDown( KK_DOWN )  )     accumMove -= forward * moveSpeed;
         if( keyboard.IsKeyDown( (vaKeyboardKeys)'D' ) || keyboard.IsKeyDown( KK_RIGHT ) )     accumMove += right * moveSpeed;
         if( keyboard.IsKeyDown( (vaKeyboardKeys)'A' ) || keyboard.IsKeyDown( KK_LEFT )  )     accumMove -= right * moveSpeed;
         if( keyboard.IsKeyDown( (vaKeyboardKeys)'Q' ) )     accumMove -= up * moveSpeed;
         if( keyboard.IsKeyDown( (vaKeyboardKeys)'E' ) )     accumMove += up * moveSpeed;

         objectPos += accumMove * smoothingLerpK;
         m_accumMove = accumMove * (1-smoothingLerpK);
      }

      objectOri = vaQuaternion::FromRotationMatrix( m_baseOrientation * cameraWorld );
   }
   camera.SetPosition( objectPos );
   camera.SetOrientation( objectOri );
}


vaCameraControllerFocusLocationsFlythrough::vaCameraControllerFocusLocationsFlythrough( )
{
    //   m_hasFocus              = true; // temporary
    m_currentKeyIndex           = -1;
    m_currentKeyTimeRemaining   = 0.0f;
    m_userParam0                = 0.0f;
    m_userParam1                = 0.0f;
    m_fixedUp                   = false;
    m_fixedUpVec                = vaVector3( 0.0f, 0.0f, 1.0f );
}

vaCameraControllerFocusLocationsFlythrough::~vaCameraControllerFocusLocationsFlythrough( )
{

}

void vaCameraControllerFocusLocationsFlythrough::CameraAttached( vaCameraBase & camera )
{
}

void vaCameraControllerFocusLocationsFlythrough::CameraTick( float deltaTime, vaCameraBase & camera, bool hasFocus )
{
    if( m_keys.size( ) == 0 )
        return;

    m_currentKeyTimeRemaining -= deltaTime;

    while( m_currentKeyTimeRemaining < 0 )
    {
        m_currentKeyIndex = ( m_currentKeyIndex + 1 ) % m_keys.size( );
        m_currentKeyTimeRemaining += m_keys[m_currentKeyIndex].ShowTime;
    }

    Keyframe & currentKey = m_keys[m_currentKeyIndex];
    Keyframe & nextKey = m_keys[( m_currentKeyIndex + 1 ) % m_keys.size( )];

    float lerpK = vaMath::Smoothstep( 1.0f - m_currentKeyTimeRemaining / currentKey.ShowTime );

    vaVector3 pos = currentKey.Position * ( 1.0f - lerpK ) + nextKey.Position * lerpK;
    m_userParam0 = currentKey.UserParam0 * ( 1.0f - lerpK ) + nextKey.UserParam0 * lerpK;
    m_userParam1 = currentKey.UserParam1 * ( 1.0f - lerpK ) + nextKey.UserParam1 * lerpK;
    vaQuaternion rot = vaQuaternion::Slerp( currentKey.Orientation, nextKey.Orientation, lerpK );

    if( m_fixedUp )
    {
        vaVector3 currentUp = rot.GetAxisY();

        vaVector3 rotAxis   = vaVector3::Cross( currentUp, m_fixedUpVec );
        float rotAngle      = vaVector3::AngleBetweenVectors( currentUp, m_fixedUpVec );

        rot *= vaQuaternion::RotationAxis( rotAxis, rotAngle );
    }

    float lf = vaMath::TimeIndependentLerpF( deltaTime, 5.0f / (currentKey.ShowTime+2.0f) );

    pos = vaMath::Lerp( camera.GetPosition(), pos, lf );
    rot = vaQuaternion::Slerp( camera.GetOrientation(), rot, lf );

    camera.SetPosition( pos );
    camera.SetOrientation( rot );
}