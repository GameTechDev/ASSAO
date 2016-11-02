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

#include "vaRendering.h"

#include "vaTriangleMesh.h"
#include "vaTexture.h"

#include "Rendering/Media/Shaders/vaSharedTypes.h"

#include "IntegratedExternals/vaImguiIntegration.h"

// for PBR, reading material:
// - http://blog.selfshadow.com/publications/s2015-shading-course/
// - https://www.allegorithmic.com/pbr-guide
// - http://www.marmoset.co/toolbag/learn/pbr-practice
// - http://www.marmoset.co/toolbag/learn/pbr-theory
// - http://www.marmoset.co/toolbag/learn/pbr-conversion
// - http://docs.cryengine.com/display/SDKDOC2/Creating+Textures+for+Physical+Based+Shading
// - 

namespace VertexAsylum
{
    class vaRenderMaterialManager;

    struct vaRenderMaterialConstructorParams : vaConstructorParamsBase
    {
        vaRenderMaterialManager &   RenderMaterialManager;
        const vaGUID &          UID;
        vaRenderMaterialConstructorParams( vaRenderMaterialManager & renderMaterialManager, const vaGUID & uid ) : RenderMaterialManager( renderMaterialManager ), UID( uid ) { }
    };

    class vaRenderMaterial : public vaAssetResource, public vaRenderingModule
    {
    public:
        // these are static settings and should not be changed once per frame or more frequently

        struct MaterialSettingsV1
        {
            vaFaceCull                                      FaceCull;

            vaVector4                                       ColorMultAlbedo;
            vaVector4                                       ColorMultSpecular;

            bool                                            AlphaTest;
            bool                                            ReceiveShadows;

            MaterialSettingsV1( )
            {
                AlphaTest                   = false;
                ReceiveShadows              = true;
                FaceCull                    = vaFaceCull::Back;

                ColorMultAlbedo             = vaVector4( 1.0f, 1.0f, 1.0f, 1.0f );
                ColorMultSpecular           = vaVector4( 1.0f, 1.0f, 1.0f, 1.0f );
            }
        };

        struct MaterialSettings
        {
            vaFaceCull                                      FaceCull;

            vaVector4                                       ColorMultAlbedo;
            vaVector4                                       ColorMultSpecular;
            vaVector4                                       ColorMultEmissive;

            bool                                            AlphaTest;
            bool                                            ReceiveShadows;
            bool                                            Wireframe;

            float                                           SpecPow;
            float                                           SpecMul;
            bool                                            Transparent;

            // for future upgrades, so that file format stays the same
            float                                           Dummy3;
            float                                           Dummy4;
            float                                           Dummy5;
            float                                           Dummy6;
            float                                           Dummy7;

            MaterialSettings( )
            {
                Transparent                 = false;
                AlphaTest                   = false;
                ReceiveShadows              = true;
                FaceCull                    = vaFaceCull::Back;
                Wireframe                   = false;

                ColorMultAlbedo             = vaVector4( 1.0f, 1.0f, 1.0f, 1.0f );
                ColorMultSpecular           = vaVector4( 1.0f, 1.0f, 1.0f, 1.0f );
                ColorMultEmissive           = vaVector4( 0.0f, 0.0f, 0.0f, 0.0f );

                SpecPow                     = 1.0f;
                SpecMul                     = 1.0f;

                Dummy3                      = 0.0f;
                Dummy4                      = 0.0f;
                Dummy5                      = 0.0f;
                Dummy6                      = 0.0f;
                Dummy7                      = 0.0f;
            }

            inline bool operator == ( const MaterialSettings & mat ) const
            {
                return 0 == memcmp( this, &mat, sizeof( MaterialSettings ) );
            }

            inline bool operator != ( const MaterialSettings & mat ) const
            {
                return 0 != memcmp( this, &mat, sizeof( MaterialSettings ) );
            }

        };

    private:
        //wstring const                                   m_name;                 // unique (within renderMeshManager) name
        vaTT_Trackee< vaRenderMaterial * >              m_trackee;

    protected:
        vaRenderMaterialManager &                       m_renderMaterialManager;

        MaterialSettings                                m_settings;

        shared_ptr<vaTexture>                           m_textureAlbedo;
        shared_ptr<vaTexture>                           m_textureNormalmap;
        shared_ptr<vaTexture>                           m_textureSpecular;
        shared_ptr<vaTexture>                           m_textureEmissive;

        vaGUID                                          m_textureAlbedoUID;
        vaGUID                                          m_textureNormalmapUID;
        vaGUID                                          m_textureSpecularUID;
        vaGUID                                          m_textureEmissiveUID;

        wstring                                         m_shaderFileName;
        string                                          m_shaderEntryVS_PosOnly;
        string                                          m_shaderEntryPS_DepthOnly;
        string                                          m_shaderEntryVS_Standard;
        string                                          m_shaderEntryPS_Forward;
        string                                          m_shaderEntryPS_Deferred;

        vector< pair< string, string > >                m_shaderMacros;
        bool                                            m_shaderMacrosDirty;
        bool                                            m_shadersDirty;

    protected:
        friend class vaRenderMeshManager;
        vaRenderMaterial( const vaConstructorParamsBase * params );
    public:
        virtual ~vaRenderMaterial( )                { }

    public:
        //const wstring &                                 GetName( ) const                                                { return m_name; };

        const MaterialSettings &                        GetSettings( )                                                  { return m_settings; }
        void                                            SetSettings( const MaterialSettings & settings )                { if( m_settings != settings ) m_shaderMacrosDirty = true; m_settings = settings; }

        void                                            SetSettingsDirty( )                                             { m_shaderMacrosDirty = true; }

        vaRenderMaterialManager &                       GetManager( ) const                                             { m_renderMaterialManager; }
        int                                             GetListIndex( ) const                                           { return m_trackee.GetIndex( ); }

        const shared_ptr<vaTexture> &                   GetTextureAlbedo( ) const                                       { return m_textureAlbedo;           }
        const shared_ptr<vaTexture> &                   GetTextureNormalmap( ) const                                    { return m_textureNormalmap;        }
        const shared_ptr<vaTexture> &                   GetTextureSpecular( ) const                                     { return m_textureSpecular;        }
        const shared_ptr<vaTexture> &                   GetTextureEmissive( ) const                                     { return m_textureEmissive;        }

        void                                            SetTextureAlbedo(    const shared_ptr<vaTexture> & texture )    { m_textureAlbedo    = texture; m_shaderMacrosDirty = true; m_textureAlbedoUID      = (texture==nullptr)?(vaCore::GUIDNull()):(texture->UIDObject_GetUID()); }
        void                                            SetTextureNormalmap( const shared_ptr<vaTexture> & texture )    { m_textureNormalmap = texture; m_shaderMacrosDirty = true; m_textureNormalmapUID   = (texture==nullptr)?(vaCore::GUIDNull()):(texture->UIDObject_GetUID()); }
        void                                            SetTextureSpecular(  const shared_ptr<vaTexture> & texture )    { m_textureSpecular  = texture; m_shaderMacrosDirty = true; m_textureSpecularUID    = (texture==nullptr)?(vaCore::GUIDNull()):(texture->UIDObject_GetUID()); }
        void                                            SetTextureEmissive(  const shared_ptr<vaTexture> & texture )    { m_textureEmissive  = texture; m_shaderMacrosDirty = true; m_textureEmissiveUID    = (texture==nullptr)?(vaCore::GUIDNull()):(texture->UIDObject_GetUID()); }

        bool                                            GetNeedsPSForShadowGenerate( ) const                            { return false; }

        vaFaceCull                                      GetFaceCull( ) const                                            { return m_settings.FaceCull; }

        bool                                            Save( vaStream & outStream );
        bool                                            Load( vaStream & inStream );
        virtual void                                    ReconnectDependencies( );

        virtual void                                    UploadToAPIContext( vaDrawContext & drawContext ) = 0;

    protected:
        void                                            UpdateShaderMacros( );
    };

    class vaRenderMaterialManager : public vaRenderingModule, public vaImguiHierarchyObject, public vaSingletonBase< vaRenderMaterialManager >
    {
        VA_RENDERING_MODULE_MAKE_FRIENDS( );
    protected:

        vaTT_Tracker< vaRenderMaterial * >              m_renderMaterials;
        // map<wstring, shared_ptr<vaRenderMaterial>>  m_renderMaterialsMap;

        shared_ptr< vaRenderMaterial >                  m_defaultMaterial;
        bool                                            m_isDestructing;

        bool                                            m_texturingDisabled;

    public:
        friend class vaRenderingCore;
        vaRenderMaterialManager( );
        virtual ~vaRenderMaterialManager( );

    private:
        void                                            RenderMaterialsTrackeeAddedCallback( int newTrackeeIndex );
        void                                            RenderMaterialsTrackeeBeforeRemovedCallback( int removedTrackeeIndex, int replacedByTrackeeIndex );

    public:
        shared_ptr<vaRenderMaterial>                    GetDefaultMaterial( ) const     { return m_defaultMaterial; }

    public:
        shared_ptr<vaRenderMaterial>                    CreateRenderMaterial( const vaGUID & uid = vaCore::GUIDCreate( ) );
        vaTT_Tracker< vaRenderMaterial * > *            GetRenderMaterialTracker( ) { return &m_renderMaterials; }

        bool                                            GetTexturingDisabled( ) const   { return m_texturingDisabled; }
        void                                            SetTexturingDisabled( bool texturingDisabled );

    protected:
        virtual string                                  IHO_GetInstanceInfo( ) const { return vaStringTools::Format( "vaRenderMaterialManager (%d meshes)", m_renderMaterials.size( ) ); }
        virtual void                                    IHO_Draw( );
    };


}