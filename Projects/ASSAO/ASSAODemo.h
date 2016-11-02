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
// 2016-09-07: filip.strugar@intel.com: first commit
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core/vaCoreIncludes.h"

#include "vaApplication.h"

#include "Scene/vaSceneIncludes.h"

#include "Rendering/vaRenderingIncludes.h"

#include "IntegratedExternals/vaImguiIntegration.h"

#include "Rendering/Effects/vaASSAO.h"

#include "ASSAOWrapper.h"
#include "ExternalSSAOWrapper.h"

#include "Rendering/vaAssetPack.h"

namespace VertexAsylum
{
    class ASSAODemo : public vaRenderingModule, public std::enable_shared_from_this<ASSAODemo>
    {
    public:

    public:

        enum class SceneSelectionType : int32
        {
            Sponza,
            SponzaAndDragons,
            Sibenik,
            SibenikAndDragons,
            LostEmpire,
            
            MaxCount
        };

        struct SSAODemoSettings
        {
            SceneSelectionType                      SceneChoice;
            bool                                    UseDeferred;
            bool                                    ShowWireframe;
            bool                                    EnableSSAO;
            int                                     SSAOSelectedVersionIndex;
            bool                                    DebugShowOpaqueSSAO;
            bool                                    DisableTexturing;
            bool                                    ExpandDrawResolution;       // to handle SSAO artifacts around screen borders
            float                                   CameraYFov;
            bool                                    UseSimpleUI;

            SSAODemoSettings( )
            {
                SceneChoice                 = SceneSelectionType::SponzaAndDragons;
                ShowWireframe               = false;
                EnableSSAO                  = true;
                SSAOSelectedVersionIndex    = 1;
                UseDeferred                 = true;
                DebugShowOpaqueSSAO         = false;
                DisableTexturing            = false;
                ExpandDrawResolution        = false;
                CameraYFov                  = 90.0f / 360.0f * VA_PIf;
                UseSimpleUI                  = true;
            }

        };

        struct GPUProfilingResults
        {
            float                                   TotalTime;
            float                                   RegularShadowmapCreate;
            float                                   SceneOpaque;
            float                                   NonShadowedTransparencies;
            float                                   ShadowedTransparencies;
            float                                   MiscDebugCanvas;
            float                                   MiscImgui;
        };

    private:
        shared_ptr<vaCameraBase>                m_camera;
        shared_ptr<vaCameraControllerFreeFlight>
                                                m_cameraFreeFlightController;
        std::shared_ptr<vaCameraControllerFocusLocationsFlythrough>     
                                                m_flythroughCameraControllerSponza;
        std::shared_ptr<vaCameraControllerFocusLocationsFlythrough>     
                                                m_flythroughCameraControllerSibenik;
        std::shared_ptr<vaCameraControllerFocusLocationsFlythrough>     
                                                m_flythroughCameraControllerLostEmpire;

        shared_ptr<vaRenderDevice>              m_renderDevice;
        shared_ptr<vaApplication>               m_application;

        unique_ptr<vaSky>                       m_sky;
        unique_ptr<vaRenderingGlobals>          m_renderingGlobals;

        shared_ptr<vaSimpleShadowMap>           m_simpleShadowMap;


        vaRenderMeshDrawList                    m_meshDrawList;

        unique_ptr<vaGBuffer>                   m_GBuffer;
        unique_ptr<vaLighting>                  m_lighting;
        shared_ptr<vaPostProcess>               m_postProcess;
        unique_ptr<vaPostProcessTonemap>        m_postProcessTonemap;
        vector<shared_ptr<vaRenderMesh>>        m_sceneMeshes;
        vector<vaMatrix4x4>                     m_sceneMeshesTransforms;

        shared_ptr<vaASSAO>                 	m_SSAOEffect_DevelopmentVersion;
        unique_ptr<ASSAOWrapper>            	m_SSAOEffect;
        unique_ptr<ExternalSSAOWrapper>         m_SSAOEffect_External;

        SceneSelectionType                      m_loadedSceneChoice;

        float                                   m_shaderDebugData[ SHADERGLOBAL_DEBUG_FLOAT_OUTPUT_COUNT ];

        shared_ptr<vaAssetPack>                 m_assetsDragon;
        shared_ptr<vaAssetPack>                 m_assetsSibenik;
        shared_ptr<vaAssetPack>                 m_assetsSponza;
        shared_ptr<vaAssetPack>                 m_assetsLostEmpire;


        bool                                    m_triggerCompareDevNonDev;
        shared_ptr<vaTexture>                   m_comparerReferenceTexture;
        shared_ptr<vaTexture>                   m_comparerCurrentTexture;
        
        vector<vaVector4>                       m_displaySampleDisk;

        
        bool                                    m_flythroughCameraEnabled;

        int                                     m_expandedSceneBorder;
        vaVector2i                              m_expandedSceneResolution;

        uint64                                  m_frameIndex;

        wstring                                 m_screenshotCapturePath;

    protected:
        SSAODemoSettings                        m_settings;

    protected:
        ASSAODemo( );

    public:
        ~ASSAODemo( );

        void                                    Initialize( const std::shared_ptr<vaRenderDevice> & renderDevice, const std::shared_ptr<vaApplication> & application );


    public:
        const std::shared_ptr<vaCameraBase> &   GetCamera( ) const          { return m_camera; }
        vaSky &                                 GetSky( )                   { return *m_sky; }

    public:
        // events/callbacks:
        void                                    OnStarted( );
        void                                    OnStopped( );
        void                                    OnBeforeStopped( );
        void                                    OnResized( int width, int height, bool windowed );
        void                                    OnTick( float deltaTime );
        void                                    OnRender( );
        void                                    OnWndProcOverride( bool & handled, LRESULT & ret, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

        vaDebugCanvas2DBase *                   GetCanvas2D( )                  { return m_renderDevice->GetCanvas2D( ); }
        vaDebugCanvas3DBase *                   GetCanvas3D( )                  { return m_renderDevice->GetCanvas3D( ); }

        SSAODemoSettings &                      GetSettings( )                  { return m_settings; }

    protected:
        void                                    SetupScene( );

        virtual void                            DrawDebugOverlay( vaDrawContext & drawContext ) = 0;

        void                                    EnsureLoaded( vaAssetPack & pack );
        void                                    InsertAllToSceneMeshesList( vaAssetPack & pack, const vaMatrix4x4 & transform = vaMatrix4x4::Identity );

        void                                    UpdateTextures( int width, int height );

    public:

        void                                    LoadCamera( int index = -1 );
        void                                    SaveCamera( int index = -1 );

        // just for debug displaying purposes
        void                                    SetDisplaySampleDisk( const vector<vaVector4> & displaySampleDisk = vector<vaVector4>() )                   { m_displaySampleDisk = displaySampleDisk; }

        void                                    CaptureScreenshotNextFrame( const wstring & path )                                                          { m_screenshotCapturePath = vaFileTools::GetAbsolutePath( path ); }

        std::shared_ptr<vaCameraControllerBase> GetCameraController( );


    private:
        //void                                    RandomizeCurrentPoissonDisk( int count = SSAO_MAX_TAPS );
    };

}