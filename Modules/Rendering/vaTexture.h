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

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// vaTexture can be a static texture used as an asset, loaded from storage or created procedurally,
// or a dynamic use GPU-only texture for use as a render target or similar. 
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core/vaCoreIncludes.h"

#include "vaRendering.h"

#include "vaTextureTypes.h"

namespace VertexAsylum
{
    struct vaTextureConstructorParams : vaConstructorParamsBase
    {
        const vaGUID &          UID;
        vaTextureConstructorParams( const vaGUID & uid = vaCore::GUIDCreate( ) ) : UID( uid ) { }
    };

    class vaTexture : public vaAssetResource, public vaRenderingModule
    {
        VA_RENDERING_MODULE_MAKE_FRIENDS( );

    private:
        vaTextureFlags                      m_flags;

        vaTextureAccessFlags                m_accessFlags;
        vaTextureType                       m_type;
        vaTextureBindSupportFlags           m_bindSupportFlags;

        vaTextureFormat                     m_resourceFormat;
        vaTextureFormat                     m_srvFormat;
        vaTextureFormat                     m_rtvFormat;
        vaTextureFormat                     m_dsvFormat;
        vaTextureFormat                     m_uavFormat;

        int                                 m_sizeX;            // serves as desc.ByteWidth for Buffer
        int                                 m_sizeY;            // doubles as ArraySize in 1D texture
        int                                 m_sizeZ;            // doubles as ArraySize in 2D texture
        int                                 m_sampleCount;
        int                                 m_mipLevels;

        int                                 m_viewedMipSlice;   // if m_viewedOriginal is nullptr, this will always be 0
        int                                 m_viewedArraySlice; // if m_viewedOriginal is nullptr, this will always be 0
        shared_ptr< vaTexture >             m_viewedOriginal;
        int                                 m_viewedSliceSizeX;
        int                                 m_viewedSliceSizeY;
        int                                 m_viewedSliceSizeZ;
    
    protected:
        static const int                    c_fileVersion       = 1;

    protected:
        friend class vaTextureDX11;     // ugly but has to be here for every API implementation - not sure how to easily handle this otherwise
                                        vaTexture( const vaConstructorParamsBase * params  );
        void                            Initialize( vaTextureBindSupportFlags binds, vaTextureFormat resourceFormat = vaTextureFormat::Unknown, vaTextureFormat srvFormat = vaTextureFormat::Unknown, vaTextureFormat rtvFormat = vaTextureFormat::Unknown, vaTextureFormat dsvFormat = vaTextureFormat::Unknown, vaTextureFormat uavFormat = vaTextureFormat::Unknown, vaTextureFlags flags = vaTextureFlags::None, int viewedMipSlice = -1, int viewedArraySlice = -1 );
    
    public:
        virtual                         ~vaTexture( )                                                   { }


    protected:
        void                            InternalUpdateFromRenderingCounterpart( bool notAllBindViewsNeeded = false );

    public:
        static vaTexture *              Import( const wstring & storagePath, bool assumeSourceIsInSRGB, bool dontAutogenerateMIPs, vaTextureBindSupportFlags binds = vaTextureBindSupportFlags::ShaderResource );
        
        static vaTexture *              Create1D( vaTextureFormat format, int width, int mipLevels, int arraySize, vaTextureBindSupportFlags bindFlags, vaTextureAccessFlags accessFlags = vaTextureAccessFlags::None, void * initialData = NULL, vaTextureFormat srvFormat = vaTextureFormat::Unknown, vaTextureFormat rtvFormat = vaTextureFormat::Unknown, vaTextureFormat dsvFormat = vaTextureFormat::Unknown, vaTextureFormat uavFormat = vaTextureFormat::Unknown, vaTextureFlags flags = vaTextureFlags::None );
        static vaTexture *              Create2D( vaTextureFormat format, int width, int height, int mipLevels, int arraySize, int sampleCount, vaTextureBindSupportFlags bindFlags, vaTextureAccessFlags accessFlags = vaTextureAccessFlags::None, void * initialData = NULL, int initialDataPitch = 0, vaTextureFormat srvFormat = vaTextureFormat::Unknown, vaTextureFormat rtvFormat = vaTextureFormat::Unknown, vaTextureFormat dsvFormat = vaTextureFormat::Unknown, vaTextureFormat uavFormat = vaTextureFormat::Unknown, vaTextureFlags flags = vaTextureFlags::None );
        static vaTexture *              Create3D( vaTextureFormat format, int width, int height, int depth, int mipLevels, vaTextureBindSupportFlags bindFlags, vaTextureAccessFlags accessFlags = vaTextureAccessFlags::None, void * initialData = NULL, int initialDataPitch = 0, int initialDataSlicePitch = 0, vaTextureFormat srvFormat = vaTextureFormat::Unknown, vaTextureFormat rtvFormat = vaTextureFormat::Unknown, vaTextureFormat dsvFormat = vaTextureFormat::Unknown, vaTextureFormat uavFormat = vaTextureFormat::Unknown, vaTextureFlags flags = vaTextureFlags::None );

        static vaTexture *              Create2DTestCheckerboardTexture( vaTextureFormat format, vaTextureBindSupportFlags bindFlags, vaTextureAccessFlags accessFlags = vaTextureAccessFlags::None );

        static vaTexture *              CreateView( std::shared_ptr< vaTexture > texture, vaTextureBindSupportFlags bindFlags, vaTextureFormat srvFormat = vaTextureFormat::Unknown, vaTextureFormat rtvFormat = vaTextureFormat::Unknown, vaTextureFormat dsvFormat = vaTextureFormat::Unknown, vaTextureFormat uavFormat = vaTextureFormat::Unknown, int viewedMipSlice = -1, int viewedArraySlice = -1 );

        vaTextureType                   GetType( ) const                                                { return m_type; }
        vaTextureBindSupportFlags       GetBindSupportFlags( ) const                                    { return m_bindSupportFlags; }
        vaTextureFlags                  GetFlags( ) const                                               { return m_flags; }


        vaTextureFormat                 GetResourceFormat( ) const                                      { return m_resourceFormat; }
        vaTextureFormat                 GetSRVFormat( ) const                                           { return m_srvFormat;      }
        vaTextureFormat                 GetDSVFormat( ) const                                           { return m_dsvFormat;      }
        vaTextureFormat                 GetRTVFormat( ) const                                           { return m_rtvFormat;      }
        vaTextureFormat                 GetUAVFormat( ) const                                           { return m_uavFormat;      }

        int                             GetSizeX( )             const                                   { return m_sizeX;       }     // serves as desc.ByteWidth for Buffer
        int                             GetSizeY( )             const                                   { return m_sizeY;       }     // doubles as ArraySize in 1D texture
        int                             GetSizeZ( )             const                                   { return m_sizeZ;       }     // doubles as ArraySize in 2D texture
        int                             GetSampleCount( )       const                                   { return m_sampleCount; }
        int                             GetMipLevels( )         const                                   { return m_mipLevels;   }

        int                             GetViewedMipSlice( )    const                                   { return m_viewedMipSlice; }
        int                             GetViewedSliceSizeX( )  const                                   { return m_viewedSliceSizeX; }
        int                             GetViewedSliceSizeY( )  const                                   { return m_viewedSliceSizeY; }
        int                             GetViewedSliceSizeZ( )  const                                   { return m_viewedSliceSizeZ; }

        virtual void                    ClearRTV( vaRenderDeviceContext & context, const vaVector4 & clearValue )                                               = 0;
        virtual void                    ClearUAV( vaRenderDeviceContext & context, const vaVector4ui & clearValue )                                             = 0;
        virtual void                    ClearUAV( vaRenderDeviceContext & context, const vaVector4 & clearValue )                                               = 0;
        virtual void                    ClearDSV( vaRenderDeviceContext & context, bool clearDepth, float depthValue, bool clearStencil, uint8 stencilValue )   = 0;

        virtual bool                    Load( vaStream & inStream )                                                                                             = 0;
        virtual bool                    Save( vaStream & outStream )                                                                                            = 0;
    };


}