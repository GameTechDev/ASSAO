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

#include "vaCameraBase.h"

#include "vaCameraControllers.h"

using namespace VertexAsylum;

using namespace std;

#pragma warning( disable : 4996 )


vaCameraBase::vaCameraBase( )
{
    m_YFOVMain          = true;
    m_YFOV              = 90.0f / 360.0f * VA_PIf;
    m_XFOV              = 0.0f;
    m_aspect            = 1.0f;
    m_nearPlane         = 0.2f;
    m_farPlane          = 100000.0f;
    m_viewportWidth     = 64;
    m_viewportHeight    = 64;
    m_position          = vaVector3( 0.0f, 0.0f, 0.0f );
    m_orientation       = vaQuaternion::Identity;
    //
    m_viewTrans         = vaMatrix4x4::Identity;
    m_projTrans         = vaMatrix4x4::Identity;
    //
    m_useReversedZ      = true;
    //
    UpdateSecondaryFOV( );
}
//
vaCameraBase::~vaCameraBase( )
{
}
//
#define RETURN_FALSE_IF_FALSE( x ) if( !(x) ) return false;
//
void vaCameraBase::UpdateFrom( vaCameraBase & copyFrom )
{
    m_YFOVMain          = copyFrom.m_YFOVMain      ;
    m_YFOV              = copyFrom.m_YFOV          ;
    m_XFOV              = copyFrom.m_XFOV          ;
    m_aspect            = copyFrom.m_aspect        ;
    m_nearPlane         = copyFrom.m_nearPlane     ;
    m_farPlane          = copyFrom.m_farPlane      ;
    m_viewportWidth     = copyFrom.m_viewportWidth ;
    m_viewportHeight    = copyFrom.m_viewportHeight; 
    m_position          = copyFrom.m_position      ;
    m_orientation       = copyFrom.m_orientation   ;
    UpdateSecondaryFOV( );
}
//
bool vaCameraBase::Load( vaStream & inStream )
{
    float dummyAspect;
    int dummyWidth;
    int dummyHeight;

    RETURN_FALSE_IF_FALSE( inStream.ReadValue( m_YFOVMain )       );
    RETURN_FALSE_IF_FALSE( inStream.ReadValue( m_YFOV )           );
    RETURN_FALSE_IF_FALSE( inStream.ReadValue( m_XFOV )           );
    RETURN_FALSE_IF_FALSE( inStream.ReadValue( dummyAspect )      );
    RETURN_FALSE_IF_FALSE( inStream.ReadValue( m_nearPlane )      );
    RETURN_FALSE_IF_FALSE( inStream.ReadValue( m_farPlane )       );
    RETURN_FALSE_IF_FALSE( inStream.ReadValue( dummyWidth )  );
    RETURN_FALSE_IF_FALSE( inStream.ReadValue( dummyHeight ) );
    RETURN_FALSE_IF_FALSE( inStream.ReadValue( m_position )       );
    RETURN_FALSE_IF_FALSE( inStream.ReadValue( m_orientation )    );
    UpdateSecondaryFOV( );
    return true;
}
//
bool vaCameraBase::Save( vaStream & outStream ) const
{
    RETURN_FALSE_IF_FALSE( outStream.WriteValue( m_YFOVMain        )  );
    RETURN_FALSE_IF_FALSE( outStream.WriteValue( m_YFOV            )  );
    RETURN_FALSE_IF_FALSE( outStream.WriteValue( m_XFOV            )  );
    RETURN_FALSE_IF_FALSE( outStream.WriteValue( m_aspect          )  );
    RETURN_FALSE_IF_FALSE( outStream.WriteValue( m_nearPlane       )  );
    RETURN_FALSE_IF_FALSE( outStream.WriteValue( m_farPlane        )  );
    RETURN_FALSE_IF_FALSE( outStream.WriteValue( m_viewportWidth   )  );
    RETURN_FALSE_IF_FALSE( outStream.WriteValue( m_viewportHeight  )  );
    RETURN_FALSE_IF_FALSE( outStream.WriteValue( m_position        )  );
    RETURN_FALSE_IF_FALSE( outStream.WriteValue( m_orientation     )  );
    return true;
}
//
void vaCameraBase::AttachController( const std::shared_ptr<vaCameraControllerBase> & cameraController )
{
    m_controller = cameraController;

    if( m_controller != nullptr )
        m_controller->CameraAttached( *this );
}
//
void vaCameraBase::Tick( float deltaTime, bool hasFocus )
{
    if( (m_controller != nullptr) && (deltaTime != 0.0f) )
        m_controller->CameraTick( deltaTime, *this, hasFocus );

    UpdateSecondaryFOV( );

    const vaQuaternion & orientation = GetOrientation( );
    const vaVector3 & position = GetPosition( );
    
    m_worldTrans = vaMatrix4x4::FromQuaternion( orientation );
    m_worldTrans.SetTranslation( m_position );

    m_direction = m_worldTrans.GetAxisX();  // forward

    m_viewTrans = m_worldTrans.Inverse( );

    if( m_useReversedZ )
    {
        m_projTrans = vaMatrix4x4::PerspectiveFovLH( m_YFOV, m_aspect, m_farPlane, m_nearPlane );

#if 0
		// just for testing - use UE4 approach to see if it works with the postprocess stuff
        const float ueGNearClippingPlane = 2.0f;
        m_projTrans.m[2][2] = 0.0f;                 //zf / ( zf - zn );
        m_projTrans.m[3][2] = ueGNearClippingPlane; //( zf * zn ) / ( zn - zf );
#endif
    }
    else
        m_projTrans = vaMatrix4x4::PerspectiveFovLH( m_YFOV, m_aspect, m_nearPlane, m_farPlane );

    // a hacky way to record camera flythroughs!
#if 1
    if( vaInputKeyboardBase::GetCurrent( )->IsKeyClicked( ( vaKeyboardKeys )'K' ) && vaInputKeyboardBase::GetCurrent( )->IsKeyDown( KK_CONTROL ) )
    {
        vaFileStream fileOut;
        if( fileOut.Open( vaCore::GetExecutableDirectory( ) + L"camerakeys.txt", FileCreationMode::Append ) )
        {
            string newKey = vaStringTools::Format( "m_flythroughCameraController->AddKey( vaCameraControllerFocusLocationsFlythrough::Keyframe( vaVector3( %.3ff, %.3ff, %.3ff ), vaQuaternion( %.3ff, %.3ff, %.3ff, %.3ff ), 10.0f ) );\n\n", 
                m_position.x, m_position.y, m_position.z, m_orientation.x, m_orientation.y, m_orientation.z, m_orientation.w );
            fileOut.WriteTXT( newKey );
            VA_LOG( "Logging camera key: %s", newKey.c_str() );
        }
    }
#endif
}
//
void vaCameraBase::SetViewportSize( int width, int height )
{
    m_viewportWidth = width;
    m_viewportHeight = height;
    m_aspect = width / (float)height;
}
//
void vaCameraBase::CalcFrustumPlanes( vaPlane planes[6] ) const
{
    vaMatrix4x4 cameraViewProj = m_viewTrans * m_projTrans;

    vaGeometry::CalculateFrustumPlanes( planes, cameraViewProj );
}
//
void vaCameraBase::GetZOffsettedProjMatrix( vaMatrix4x4 & mat, float zModMul, float zModAdd )
{
    UpdateSecondaryFOV( );
    mat = vaMatrix4x4::PerspectiveFovLH( m_YFOV, m_aspect, m_nearPlane * zModMul + zModAdd, m_farPlane * zModMul + zModAdd );
}
//
void vaCameraBase::SetOrientationLookAt( const vaVector3 & lookAtPos, const vaVector3 & upVector )
{
    vaMatrix4x4 lookAt = vaMatrix4x4::LookAtLH( m_position, lookAtPos, upVector );

    SetOrientation( vaQuaternion::FromRotationMatrix( lookAt ).Inverse( ) );
}

/*
void vaCameraBase::FillSelectionParams( vaSRMSelectionParams & selectionParams )
{
selectionParams.MainCameraPos = XMFLOAT4( m_position.x, m_position.y, m_position.z, 1.0f );
selectionParams.MainCameraDir = XMFLOAT4( m_lookDir.x, m_lookDir.y, m_lookDir.z, 1.0f );

XMVECTOR planes[6];
GetFrustumPlanes( planes );
for( int i = 0; i < 6; i++ )
XMStoreFloat4( &selectionParams.MainCameraFrustum[i], planes[i] );

selectionParams.MainCameraClipNear  = m_nearPlane;
selectionParams.MainCameraClipFar   = m_farPlane;
selectionParams.MainCameraViewRange = m_farPlane;
}
//
void vaCameraBase::FillRenderParams( vaSRMRenderParams & renderParams )
{
renderParams.CameraView = *GetViewMatrix();
renderParams.CameraProj = *GetProjMatrix();

renderParams.CameraPos = XMFLOAT4( m_position.x, m_position.y, m_position.z, 1.0f );
renderParams.CameraDir = XMFLOAT4( m_lookDir.x, m_lookDir.y, m_lookDir.z, 1.0f );

renderParams.CameraClipNear      = m_nearPlane;
renderParams.CameraClipFar       = m_farPlane;
renderParams.CameraFOVY          = m_FOV;
renderParams.CameraAspectRatio   = m_aspect;
}
*/
//
void vaCameraBase::UpdateSecondaryFOV( )
{
    if( m_YFOVMain )
    {
        m_XFOV = m_YFOV / m_aspect;
    }
    else
    {
        m_YFOV = m_XFOV * m_aspect;
    }
}
//
