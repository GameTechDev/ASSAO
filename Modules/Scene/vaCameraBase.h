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
    class vaCameraControllerBase;

    class vaCameraBase
    {
    protected:
        // attached controller
        std::shared_ptr < vaCameraControllerBase >
            m_controller;
        //                      
        // Primary values
        float                           m_YFOV;
        float                           m_XFOV;
        bool                            m_YFOVMain;
        float                           m_aspect;
        float                           m_nearPlane;
        float                           m_farPlane;
        int                             m_viewportWidth;
        int                             m_viewportHeight;
        bool                            m_useReversedZ;
        //

        // in world space
        vaVector3                       m_position;
        vaQuaternion                    m_orientation;
        //                            
        // Secondary values (updated by Tick() )
        vaMatrix4x4                     m_worldTrans;
        vaMatrix4x4                     m_viewTrans;
        vaMatrix4x4                     m_projTrans;
        vaVector3                       m_direction;
        //
    public:
        vaCameraBase( );
        virtual ~vaCameraBase( );
        //
    public:
        float                           GetYFOV( ) const                                     { return m_YFOV; }
        float                           GetXFOV( ) const                                     { return m_XFOV; }
        float                           GetAspect( ) const                                   { return m_aspect; }
        //                              
        void                            SetYFOV( float yFov )                               { m_YFOV = yFov; m_YFOVMain = true;    UpdateSecondaryFOV( ); }
        void                            SetXFOV( float xFov )                               { m_XFOV = xFov; m_YFOVMain = false;   UpdateSecondaryFOV( ); }
        //
        void                            SetNearPlane( float nearPlane )                     { m_nearPlane = nearPlane; }
        void                            SetFarPlane( float farPlane )                       { m_farPlane = farPlane; }
        //
        float                           GetNearPlane( ) const                                { return m_nearPlane; }
        float                           GetFarPlane( ) const                                 { return m_farPlane; }
        //
        void                            SetPosition( const vaVector3 & newPos )             { m_position = newPos; }
        void                            SetOrientation( const vaQuaternion & newOri )       { m_orientation = newOri; }
        void                            SetOrientationLookAt( const vaVector3 & lookAtPos, const vaVector3 & upVector = vaVector3( 0.0f, 0.0f, 1.0f ) );
        //
        const vaVector3 &               GetPosition( ) const                                 { return m_position; }
        const vaQuaternion &            GetOrientation( ) const                              { return m_orientation; }
        const vaVector3 &               GetDirection( ) const                                { return m_direction; }
        //                              
        void                            CalcFrustumPlanes( vaPlane planes[6] ) const;
        //                   
        const vaMatrix4x4 &             GetWorldMatrix( ) const                             { return m_worldTrans; }
        const vaMatrix4x4 &             GetViewMatrix( ) const                              { return m_viewTrans; }
        const vaMatrix4x4 &             GetProjMatrix( ) const                              { return m_projTrans; }
        // same as GetWorldMatrix !!
        const vaMatrix4x4 &             GetInvViewMatrix( ) const                           { return m_worldTrans; }
        //                      
        void                            GetZOffsettedProjMatrix( vaMatrix4x4 & outMat, float zModMul = 1.0f, float zModAdd = 0.0f );
        //                         
        void                            SetUseReversedZ( bool useReversedZ )                { m_useReversedZ = useReversedZ; }
        bool                            GetUseReversedZ( ) const                            { return m_useReversedZ; }
        //
        //
        /*
        void                            FillSelectionParams( VertexAsylum::vaSRMSelectionParams & selectionParams );
        void                            FillRenderParams( VertexAsylum::vaSRMRenderParams & renderParams );
        */
        //
        virtual void                    SetViewportSize( int width, int height );
        //
        int                             GetViewportWidth( ) const                    { return m_viewportWidth; }
        int                             GetViewportHeight( ) const                   { return m_viewportHeight; }
        //
        void                            UpdateFrom( vaCameraBase & copyFrom );
        //
        bool                            Load( vaStream & inStream );
        bool                            Save( vaStream & outStream ) const;
        //
        const std::shared_ptr<vaCameraControllerBase> &
                                        GetAttachedController( )                    { return m_controller; }
        void                            AttachController( const std::shared_ptr<vaCameraControllerBase> & cameraController );
        //
        void                            Tick( float deltaTime, bool hasFocus );


    protected:
        void                            UpdateSecondaryFOV( );
        //
        virtual void                    SetAspect( float aspect )                    { m_aspect = aspect; }
        //*/
    };

}
