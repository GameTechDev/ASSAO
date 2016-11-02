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

// vaRenderMesh and vaRenderMeshManager are a generic render mesh implementation

namespace VertexAsylum
{
    enum class vaAssetType : int32
    {
        Texture,
        RenderMesh,
        RenderMaterial,
    };

    class vaTexture;
    class vaRenderMesh;
    class vaRenderMaterial;
    class vaAssetPack;

    // this needs to be converted to a class, along with type names and other stuff (it started as a simple struct)
    struct vaAsset : public vaImguiHierarchyObject
    {
    protected:
        struct UIContext
        {
            char        RenamingPopupNewNameBuff[128];
        };
    UIContext                                               m_uiContext;

    private:
        friend class vaAssetPack;
        shared_ptr<vaAssetResource>                         m_resourceBasePtr;
        string                                              m_name;                     // warning, never change this except by using Rename
        vaAssetPack &                                       m_parentPack;
        int                                                 m_parentPackStorageIndex;   // referring to vaAssetPack::m_assetList

    protected:
        vaAsset( vaAssetPack & pack, const vaAssetType type, const string & name, const shared_ptr<vaAssetResource> & resourceBasePtr ) : m_parentPack( pack ), Type( type ), m_name( name ), m_resourceBasePtr( resourceBasePtr ), m_parentPackStorageIndex( -1 )
                                                            { m_resourceBasePtr->SetParentAsset( this ); }
        virtual ~vaAsset( )                                 { m_resourceBasePtr->SetParentAsset( nullptr ); }

    public:
        const vaAssetType                                   Type;
//        const wstring                                       StoragePath;

        virtual const shared_ptr<vaTexture> *               GetTexture( )                       { return nullptr; }
        virtual const shared_ptr<vaRenderMesh> *            GetMesh( )                          { return nullptr; }
        virtual const shared_ptr<vaRenderMaterial> *        GetMaterial( )                      { return nullptr; }

        const string &                                      Name( ) const                       { return m_name; }

        bool                                                Rename( const string & newName );

        virtual bool                                        Save( vaStream & outStream )    = 0;

        void                                                ReconnectDependencies( )            { m_resourceBasePtr->ReconnectDependencies(); }

    protected:
        virtual string                                      IHO_GetInstanceInfo( ) const        { return vaStringTools::Format( "%s", m_name.c_str() ); }
        virtual void                                        IHO_Draw( );
    };

    struct vaAssetTexture : public vaAsset
    {
    private:
        friend class vaAssetPack;
        vaAssetTexture( vaAssetPack & pack, const shared_ptr<vaTexture> & texture, const string & name );
    
    public:

        shared_ptr<vaTexture>                           Resource;

        virtual const shared_ptr<vaTexture> *           GetTexture( )                        { return &Resource; }

        virtual bool                                    Save( vaStream & outStream );
        static vaAssetTexture *                         CreateAndLoad( vaAssetPack & pack, const string & name, vaStream & inStream );

        static shared_ptr<vaAssetTexture>               SafeCast( const shared_ptr<vaAsset> & asset ) { assert( asset->Type == vaAssetType::RenderMesh ); return std::dynamic_pointer_cast<vaAssetTexture, vaAsset>( asset ); }

    private:
        virtual string                                  IHO_GetInstanceInfo( ) const        { return vaStringTools::Format( "texture: %s ", Name().c_str() ); }
    };

    struct vaAssetRenderMesh : public vaAsset
    {
    private:
        friend class vaAssetPack;
        vaAssetRenderMesh( vaAssetPack & pack, const shared_ptr<vaRenderMesh> & mesh, const string & name );

    public:
        shared_ptr<vaRenderMesh>                        Resource;

        virtual const shared_ptr<vaRenderMesh> *        GetMesh( )                           { return &Resource; }

        virtual bool                                    Save( vaStream & outStream );
        static vaAssetRenderMesh *                      CreateAndLoad( vaAssetPack & pack, const string & name, vaStream & inStream );

        static shared_ptr<vaAssetRenderMesh>            SafeCast( const shared_ptr<vaAsset> & asset ) { assert( asset->Type == vaAssetType::RenderMesh ); return std::dynamic_pointer_cast<vaAssetRenderMesh, vaAsset>( asset ); }

    private:
        virtual string                                  IHO_GetInstanceInfo( ) const        { return vaStringTools::Format( "mesh: %s ", Name().c_str() ); }
    };

    struct vaAssetRenderMaterial : public vaAsset
    {
    private:
        friend class vaAssetPack;
        vaAssetRenderMaterial( vaAssetPack & pack, const shared_ptr<vaRenderMaterial> & material, const string & name );

    public:
        shared_ptr<vaRenderMaterial>                    Resource;

        virtual const shared_ptr<vaRenderMaterial> *    GetMaterial( )                       { return &Resource; }

        virtual bool                                    Save( vaStream & outStream );
        static vaAssetRenderMaterial *                  CreateAndLoad( vaAssetPack & pack, const string & name, vaStream & inStream );

        static shared_ptr<vaAssetRenderMaterial>        SafeCast( const shared_ptr<vaAsset> & asset ) { assert( asset->Type == vaAssetType::RenderMaterial ); return std::dynamic_pointer_cast<vaAssetRenderMaterial, vaAsset>( asset ); }

    private:
        virtual string                                  IHO_GetInstanceInfo( ) const        { return vaStringTools::Format( "material: %s ", Name().c_str() ); }
        virtual void                                    IHO_Draw( );
    };

    class vaAssetPack : public vaImguiHierarchyObject //, public std::enable_shared_from_this<vaAssetPack>
    {
        struct UIContext
        {
            char        RenamingPopupNewNameBuff[128];

            wstring     ImportingPopupSelectedFile;
            vaVector3   ImportingPopupBaseTranslation;
            vaVector3   ImportingPopupBaseScaling;
            bool        ImportingPopupRotateX90;            // a.k.a. Z is up
            bool        ImportingForceGenerateNormals; 
            bool        ImportingGenerateNormalsIfNeeded; 
            bool        ImportingGenerateSmoothNormals; 
            //float       ImportingGenerateSmoothNormalsSmoothingAngle; 
            //bool        ImportingRegenerateTangents;

            UIContext( )
            {
            }
        };

        friend class vaAssetPackManager;
        vaTT_Trackee< vaAssetPack * >                       m_trackee;  // for tracking by vaAssetPackManager

    protected:
        string                                              m_name;
        std::map< string, shared_ptr<vaAsset> >             m_assetMap;
        
        vector< shared_ptr<vaAsset> >                       m_assetList;

        UIContext                                           m_uiContext;

    protected:

    public:
        vaAssetPack( const string & name );
        virtual ~vaAssetPack( );

    public:
        string                                              FindSuitableAssetName( const string & nameSuggestion );

        shared_ptr<vaAsset>                                 Find( const string & _name );
        //shared_ptr<vaAsset>                                 FindByStoragePath( const wstring & _storagePath );
        void                                                Remove( const shared_ptr<vaAsset> & asset );
        void                                                RemoveAll( );
        // save current contents
        bool                                                Save( vaStream & outStream );
        // load contents (current contents are not deleted)
        bool                                                Load( vaStream & inStream );

    public:
        shared_ptr<vaAssetTexture>                          Add( const shared_ptr<vaTexture> & texture, const string & name );
        shared_ptr<vaAssetRenderMesh>                       Add( const shared_ptr<vaRenderMesh> & mesh, const string & name );
        shared_ptr<vaAssetRenderMaterial>                   Add( const shared_ptr<vaRenderMaterial> & material, const string & name );

        const string &                                      Name( ) const                               { return m_name; }

        bool                                                Rename( vaAsset & asset, const string & newName );

        size_t                                              Count( ) const                          { assert( m_assetList.size() == m_assetMap.size() ); return m_assetList.size(); }

        const shared_ptr<vaAsset> &                         operator [] ( size_t index )            { return m_assetList[index]; }

    protected:
        virtual string                                      IHO_GetInstanceInfo( ) const                { return vaStringTools::Format("Asset Pack '%s'", m_name.c_str() ); }
        virtual void                                        IHO_Draw( );

    private:
        void                                                InsertAndTrackMe( shared_ptr<vaAsset> newAsset );

    private:
        void                                                OnRenderingAPIAboutToShutdown( )            { RemoveAll(); }
    };

    class vaAssetPackManager : public vaImguiHierarchyObject, public vaSingletonBase< vaAssetPackManager >
    {
        unique_ptr<vaAssetPack>                             m_defaultPack;

    protected:
        friend class vaAssetPack;
        vaTT_Tracker< vaAssetPack * >                       m_assetPackTracker; // to track all vaAssetPacks

    public:
                                                            vaAssetPackManager( );
                                                            ~vaAssetPackManager( );

    public:
        vaAssetPack &                                       DefaultPack( )                              { return *m_defaultPack; }

    protected:
        // Many assets have DirectX/etc. resource locks so make sure we're not holding any references 
        friend class vaDirectXCore; // <- these should be reorganized so that this is not called from anything that is API-specific
        void                                                OnRenderingAPIAboutToShutdown( );

    protected:
        virtual string                                      IHO_GetInstanceInfo( ) const { return "Asset Pack Manager"; }
        virtual void                                        IHO_Draw( );
    };


    inline shared_ptr<vaAsset> vaAssetPack::Find( const string & _name ) 
    { 
        auto it = m_assetMap.find( vaStringTools::ToLower( _name ) );                        
        if( it == m_assetMap.end( ) ) 
            return nullptr; 
        else 
            return it->second; 
    }
    //inline shared_ptr<vaAsset> vaAssetPack::FindByStoragePath( const wstring & _storagePath ) 
    //{ 
    //    auto it = m_assetMapByStoragePath.find( vaStringTools::ToLower( _storagePath ) );    
    //    if( it == m_assetMapByStoragePath.end( ) ) 
    //        return nullptr; 
    //    else 
    //        return it->second; 
    //}

}