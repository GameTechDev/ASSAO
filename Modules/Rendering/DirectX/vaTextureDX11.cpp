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

#include "vaTextureDX11.h"

#include "vaRenderingToolsDX11.h"

#include "Rendering/DirectX/vaRenderDeviceContextDX11.h"


using namespace VertexAsylum;

vaTexture *          vaTexture::Import( const wstring & storageFilePath, bool assumeSourceIsInSRGB, bool dontAutogenerateMIPs, vaTextureBindSupportFlags binds )
{
    assert( vaDirectXCore::GetDevice( ) != NULL ); // none of this works without a device
    if( vaDirectXCore::GetDevice( ) == NULL )
        return NULL;

    vaTexture * texture = VA_RENDERING_MODULE_CREATE_PARAMS( vaTexture, vaTextureConstructorParams( vaCore::GUIDCreate( ) ) );
    texture->Initialize( binds );

    vaTextureDX11 * dxTexture = vaSaferStaticCast<vaTextureDX11*>(texture);

    if( dxTexture->Import( storageFilePath, assumeSourceIsInSRGB, dontAutogenerateMIPs, binds ) )
    {
        //texture->SetStoragePath( storageFilePath );
        // success!
        return texture;
    }
    else
    {
        // should probably load dummy checkerbox fall back texture here 
        assert( false );
        delete texture;
        return NULL;
    }
}

static D3D11_USAGE DX11UsageFromVAAccessFlags( vaTextureAccessFlags flags )
{
    if( ( ( flags & vaTextureAccessFlags::CPURead ) != 0 ) && ( ( flags & vaTextureAccessFlags::CPUWrite ) != 0 ) )
    {
        // wrong combination - both read and write at the same time not supported by VA
        assert( false );
        return D3D11_USAGE_DEFAULT;
    }
    if( ( flags & vaTextureAccessFlags::CPURead ) != 0 )
        return D3D11_USAGE_STAGING;
    else if( ( flags & vaTextureAccessFlags::CPUWrite ) != 0 )
        return D3D11_USAGE_DYNAMIC;
    else
        return D3D11_USAGE_DEFAULT;
}


static vaTextureBindSupportFlags BindFlagsVAFromDX( UINT bindFlags )
{
    vaTextureBindSupportFlags ret = vaTextureBindSupportFlags::None;
    if( ( bindFlags & D3D11_BIND_VERTEX_BUFFER ) != 0 )
        ret |= vaTextureBindSupportFlags::VertexBuffer;
    if( ( bindFlags & D3D11_BIND_INDEX_BUFFER ) != 0 )
        ret |= vaTextureBindSupportFlags::IndexBuffer;
    if( ( bindFlags & D3D11_BIND_CONSTANT_BUFFER ) != 0 )
        ret |= vaTextureBindSupportFlags::ConstantBuffer;
    if( ( bindFlags & D3D11_BIND_SHADER_RESOURCE ) != 0 )
        ret |= vaTextureBindSupportFlags::ShaderResource;
    if( ( bindFlags & D3D11_BIND_RENDER_TARGET ) != 0 )
        ret |= vaTextureBindSupportFlags::RenderTarget;
    if( ( bindFlags & D3D11_BIND_DEPTH_STENCIL ) != 0 )
        ret |= vaTextureBindSupportFlags::DepthStencil;
    if( ( bindFlags & D3D11_BIND_UNORDERED_ACCESS ) != 0 )
        ret |= vaTextureBindSupportFlags::UnorderedAccess;
    return ret;
}

static vaTextureAccessFlags CPUAccessFlagsVAFromDX( UINT accessFlags )
{
    vaTextureAccessFlags ret = vaTextureAccessFlags::None;
    if( ( accessFlags & D3D11_CPU_ACCESS_READ ) != 0 )
        ret |= vaTextureAccessFlags::CPURead;
    if( ( accessFlags & D3D11_CPU_ACCESS_WRITE ) != 0 )
        ret |= vaTextureAccessFlags::CPUWrite;
    return ret;
}

static UINT BindFlagsDXFromVA( vaTextureBindSupportFlags bindFlags )
{
    UINT ret = 0;
    if( ( bindFlags & vaTextureBindSupportFlags::VertexBuffer ) != 0 )
        ret |= D3D11_BIND_VERTEX_BUFFER;
    if( ( bindFlags & vaTextureBindSupportFlags::IndexBuffer ) != 0 )
        ret |= D3D11_BIND_INDEX_BUFFER;
    if( ( bindFlags & vaTextureBindSupportFlags::ConstantBuffer ) != 0 )
        ret |= D3D11_BIND_CONSTANT_BUFFER;
    if( ( bindFlags & vaTextureBindSupportFlags::ShaderResource ) != 0 )
        ret |= D3D11_BIND_SHADER_RESOURCE;
    if( ( bindFlags & vaTextureBindSupportFlags::RenderTarget ) != 0 )
        ret |= D3D11_BIND_RENDER_TARGET;
    if( ( bindFlags & vaTextureBindSupportFlags::DepthStencil ) != 0 )
        ret |= D3D11_BIND_DEPTH_STENCIL;
    if( ( bindFlags & vaTextureBindSupportFlags::UnorderedAccess ) != 0 )
        ret |= D3D11_BIND_UNORDERED_ACCESS;
    return ret;
}

static UINT CPUAccessFlagsDXFromVA( vaTextureAccessFlags accessFlags )
{
    UINT ret = 0;
    if( ( accessFlags & vaTextureAccessFlags::CPURead ) != 0 )
        ret |= D3D11_CPU_ACCESS_READ;
    if( ( accessFlags & vaTextureAccessFlags::CPUWrite ) != 0 )
        ret |= D3D11_CPU_ACCESS_WRITE;
    return ret;
}

vaTexture * vaTexture::Create1D( vaTextureFormat format, int width, int mipLevels, int arraySize, vaTextureBindSupportFlags bindFlags, vaTextureAccessFlags accessFlags, void * initialData, vaTextureFormat srvFormat, vaTextureFormat rtvFormat, vaTextureFormat dsvFormat, vaTextureFormat uavFormat, vaTextureFlags flags )
{
    vaTexture * texture = VA_RENDERING_MODULE_CREATE_PARAMS( vaTexture, vaTextureConstructorParams( vaCore::GUIDCreate( ) ) );
    texture->Initialize( bindFlags, format, srvFormat, rtvFormat, dsvFormat, uavFormat, flags );

    vaTextureDX11 * dxTexture = vaSaferStaticCast<vaTextureDX11*>( texture );

    D3D11_SUBRESOURCE_DATA dxInitDataObj;
    D3D11_SUBRESOURCE_DATA * dxInitDataPtr = NULL;
    if( initialData != NULL )
    {
        dxInitDataObj.pSysMem = initialData;
        dxInitDataObj.SysMemPitch = 0;
        dxInitDataObj.SysMemSlicePitch = 0;
        dxInitDataPtr = &dxInitDataObj;
    }

    if( (accessFlags & vaTextureAccessFlags::CPURead) != 0 )
    {
        // Cannot have any shader binds if texture has CPU read access flag
        assert( bindFlags == vaTextureBindSupportFlags::None );
        if( bindFlags != vaTextureBindSupportFlags::None )
            return NULL;
    }

    // do we need an option to make this any other usage? staging will be needed likely
    D3D11_USAGE usage = DX11UsageFromVAAccessFlags( accessFlags );
    UINT miscFlags = 0;

    ID3D11Resource * resource = vaDirectXTools::CreateTexture1D( (DXGI_FORMAT)format, width, dxInitDataPtr, arraySize, mipLevels, BindFlagsDXFromVA( bindFlags ), usage, CPUAccessFlagsDXFromVA( accessFlags ), miscFlags );

    if( resource != NULL )
    {
        dxTexture->SetResource( resource );
        //texture->SetStoragePath( L"" );
        return texture;
    }
    else
    {
        delete texture;
        assert( false );
        return NULL;
    }
}

vaTexture * vaTexture::Create2D( vaTextureFormat format, int width, int height, int mipLevels, int arraySize, int sampleCount, vaTextureBindSupportFlags bindFlags, vaTextureAccessFlags accessFlags, void * initialData, int initialDataPitch, vaTextureFormat srvFormat, vaTextureFormat rtvFormat, vaTextureFormat dsvFormat, vaTextureFormat uavFormat, vaTextureFlags flags )
{
    vaTexture * texture = VA_RENDERING_MODULE_CREATE_PARAMS( vaTexture, vaTextureConstructorParams( vaCore::GUIDCreate( ) ) );
    texture->Initialize( bindFlags, format, srvFormat, rtvFormat, dsvFormat, uavFormat, flags );

    vaTextureDX11 * dxTexture = vaSaferStaticCast<vaTextureDX11*>( texture );

    D3D11_SUBRESOURCE_DATA dxInitDataObj;
    D3D11_SUBRESOURCE_DATA * dxInitDataPtr = NULL;
    if( initialData != NULL )
    {
        dxInitDataObj.pSysMem           = initialData;
        dxInitDataObj.SysMemPitch       = initialDataPitch;
        dxInitDataObj.SysMemSlicePitch  = 0;
        dxInitDataPtr = &dxInitDataObj;
    }

    // do we need an option to make this any other usage? staging will be needed likely
    D3D11_USAGE usage = DX11UsageFromVAAccessFlags( accessFlags );
    UINT miscFlags = 0;

    ID3D11Resource * resource = vaDirectXTools::CreateTexture2D( (DXGI_FORMAT)format, width, height, dxInitDataPtr, arraySize, mipLevels, BindFlagsDXFromVA(bindFlags), usage, CPUAccessFlagsDXFromVA(accessFlags), sampleCount, 0, miscFlags );
    
    if( resource != NULL )
    {
        dxTexture->SetResource( resource );
        //texture->SetStoragePath( L"" );
        return texture;
    }
    else
    {
        delete texture;
        assert( false );
        return NULL;
    }
}

vaTexture * vaTexture::Create3D( vaTextureFormat format, int width, int height, int depth, int mipLevels, vaTextureBindSupportFlags bindFlags, vaTextureAccessFlags accessFlags, void * initialData, int initialDataPitch, int initialDataSlicePitch, vaTextureFormat srvFormat, vaTextureFormat rtvFormat, vaTextureFormat dsvFormat, vaTextureFormat uavFormat, vaTextureFlags flags )
{
    vaTexture * texture = VA_RENDERING_MODULE_CREATE_PARAMS( vaTexture, vaTextureConstructorParams( vaCore::GUIDCreate( ) ) );
    texture->Initialize( bindFlags, format, srvFormat, rtvFormat, dsvFormat, uavFormat, flags );

    vaTextureDX11 * dxTexture = vaSaferStaticCast<vaTextureDX11*>( texture );

    D3D11_SUBRESOURCE_DATA dxInitDataObj;
    D3D11_SUBRESOURCE_DATA * dxInitDataPtr = NULL;
    if( initialData != NULL )
    {
        dxInitDataObj.pSysMem = initialData;
        dxInitDataObj.SysMemPitch = initialDataPitch;
        dxInitDataObj.SysMemSlicePitch = 0;
        dxInitDataPtr = &dxInitDataObj;
    }

    // do we need an option to make this any other usage? staging will be needed likely
    D3D11_USAGE usage = DX11UsageFromVAAccessFlags( accessFlags );
    UINT miscFlags = 0;

    ID3D11Resource * resource = vaDirectXTools::CreateTexture3D( (DXGI_FORMAT)format, width, height, depth, dxInitDataPtr, mipLevels, BindFlagsDXFromVA( bindFlags ), usage, CPUAccessFlagsDXFromVA( accessFlags ), miscFlags );

    if( resource != NULL )
    {
        dxTexture->SetResource( resource );
        //texture->SetStoragePath( L"" );
        return texture;
    }
    else
    {
        delete texture;
        assert( false );
        return NULL;
    }
}

vaTexture * vaTextureDX11::CreateWrap( ID3D11Resource * resource, vaTextureFormat srvFormat, vaTextureFormat rtvFormat, vaTextureFormat dsvFormat, vaTextureFormat uavFormat )
{
    D3D11_RESOURCE_DIMENSION dim;
    resource->GetType( &dim );

    UINT bindFlags       = 0;

    vaTextureFormat resourceFormat = vaTextureFormat::Unknown;

    // get bind flags and format - needed at this point
    switch( dim )
    {
        case D3D11_RESOURCE_DIMENSION_BUFFER:
        {
            ID3D11Buffer * buffer = vaDirectXTools::QueryResourceInterface<ID3D11Buffer>( resource, IID_ID3D11Buffer );        
            assert( buffer != NULL ); if( buffer == NULL ) return NULL;

            D3D11_BUFFER_DESC desc; buffer->GetDesc( &desc ); SAFE_RELEASE( buffer );
            bindFlags = desc.BindFlags;
        }  break;
        case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
        {
            ID3D11Texture1D * tex = vaDirectXTools::QueryResourceInterface<ID3D11Texture1D>( resource, IID_ID3D11Texture1D );
            assert( tex != NULL );  if( tex == NULL ) return NULL;

            D3D11_TEXTURE1D_DESC desc; tex->GetDesc( &desc ); SAFE_RELEASE( tex );
            bindFlags       = desc.BindFlags;
            resourceFormat  = (vaTextureFormat)desc.Format;
        }  break;
        case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
        {
            ID3D11Texture2D * tex = vaDirectXTools::QueryResourceInterface<ID3D11Texture2D>( resource, IID_ID3D11Texture2D );
            assert( tex != NULL );  if( tex == NULL ) return NULL;

            D3D11_TEXTURE2D_DESC desc; tex->GetDesc( &desc ); SAFE_RELEASE( tex );
            bindFlags       = desc.BindFlags;
            resourceFormat  = (vaTextureFormat)desc.Format;
        }  break;
        case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
        {
            ID3D11Texture3D * tex = vaDirectXTools::QueryResourceInterface<ID3D11Texture3D>( resource, IID_ID3D11Texture3D );
            assert( tex != NULL );  if( tex == NULL ) return NULL;

            D3D11_TEXTURE3D_DESC desc; tex->GetDesc( &desc ); SAFE_RELEASE( tex );
            bindFlags       = desc.BindFlags;
            resourceFormat  = (vaTextureFormat)desc.Format;
        } break;
        default:
        {
            assert( false ); return NULL;
        }  break;
    }

    vaTextureFlags flags = vaTextureFlags::None; // this should be deduced from something probably, but not needed at the moment

    vaTexture * newTexture = VA_RENDERING_MODULE_CREATE_PARAMS( vaTexture, vaTextureConstructorParams( vaCore::GUIDCreate( ) ) );
    newTexture->Initialize( BindFlagsVAFromDX(bindFlags), resourceFormat, srvFormat, rtvFormat, dsvFormat, uavFormat, flags );

    vaTextureDX11 * newDX11Texture = vaSaferStaticCast<vaTextureDX11*>( newTexture );

    resource->AddRef();
    newDX11Texture->SetResource( resource );

    return newTexture;
}

vaTexture * vaTexture::CreateView( std::shared_ptr< vaTexture > texture, vaTextureBindSupportFlags bindFlags, vaTextureFormat srvFormat, vaTextureFormat rtvFormat, vaTextureFormat dsvFormat, vaTextureFormat uavFormat, int mipSlice, int arraySlice )
{
    vaTextureDX11 * origDX11Texture = vaSaferStaticCast<vaTextureDX11*>( texture.get() );

    ID3D11Resource * resource = origDX11Texture->GetResource( );

    if( resource == NULL )
    {
        assert( false ); 
        return NULL;
    }

    // Can't request additional binding flags that were not supported in the original texture
    vaTextureBindSupportFlags origFlags = origDX11Texture->GetBindSupportFlags();
    assert( ((~origFlags) & bindFlags) == 0 );

    vaTexture * newTexture = VA_RENDERING_MODULE_CREATE_PARAMS( vaTexture, vaTextureConstructorParams( vaCore::GUIDCreate( ) ) );
    newTexture->Initialize( bindFlags, texture->GetResourceFormat(), srvFormat, rtvFormat, dsvFormat, uavFormat, texture->GetFlags(), mipSlice, arraySlice );
    
    // it is debatable whether this is needed since DX resources have reference counting and will stay alive, but it might be useful for DX12 or other API implementations so I'll leave it in
    newTexture->m_viewedOriginal = texture;

    vaTextureDX11 * newDX11Texture = vaSaferStaticCast<vaTextureDX11*>( newTexture );
    resource->AddRef( );
    newDX11Texture->SetResource( resource, true );
    return newTexture;
}

vaTextureDX11::vaTextureDX11( const vaConstructorParamsBase * params ) : vaTexture( params )
{ 
    m_resource  = NULL;
    m_buffer    = NULL;
    m_texture1D = NULL;
    m_texture2D = NULL;
    m_texture3D = NULL;
    m_srv       = NULL;
    m_rtv       = NULL;
    m_dsv       = NULL;
    m_uav       = NULL;

    VA_DIRECTX_NOTIFY_TARGET_CONSTRUCT_PATCH( vaTextureDX11 );
}

vaTextureDX11::~vaTextureDX11( )
{ 
    VA_DIRECTX_NOTIFY_TARGET_DESTRUCT_PATCH( vaTextureDX11 );
}

void vaTextureDX11::OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain )
{
}

void vaTextureDX11::OnDeviceDestroyed( )
{
    Destroy();
}

void vaTextureDX11::Destroy( )
{
    SAFE_RELEASE( m_resource );
    SAFE_RELEASE( m_buffer );
    SAFE_RELEASE( m_texture1D );
    SAFE_RELEASE( m_texture2D );
    SAFE_RELEASE( m_texture3D );
    SAFE_RELEASE( m_srv );
    SAFE_RELEASE( m_rtv );
    SAFE_RELEASE( m_dsv );
    SAFE_RELEASE( m_uav );
}

bool vaTextureDX11::Import( const wstring & storageFilePath, bool assumeSourceIsInSRGB, bool dontAutogenerateMIPs, vaTextureBindSupportFlags binds )
{
    Destroy( );

    std::shared_ptr<vaMemoryStream> fileContents;

    wstring usedPath;

    wstring outDir, outName, outExt;
    vaStringTools::SplitPath( storageFilePath, &outDir, &outName, &outExt );
    outExt = vaStringTools::ToLower( outExt );

    bool isDDS = outExt == L".dds";

    // try asset paths
    if( fileContents.get( ) == NULL ) 
    {
        usedPath = vaRenderingCore::GetInstance().FindAssetFilePath( storageFilePath );
        //fileContents = vaFileTools::LoadFileToMemoryStream( usedPath.c_str() );
    }

    // found? try load and return!
    if( vaFileTools::FileExists( usedPath ) )
    //if( fileContents.get( ) != NULL )
    {
        if( isDDS )
        {
            m_resource = vaDirectXTools::LoadTextureDDS( usedPath.c_str(), assumeSourceIsInSRGB, dontAutogenerateMIPs );
            //m_resource = vaDirectXTools::LoadTextureDDS( fileContents->GetBuffer( ), fileContents->GetLength( ), assumeSourceIsInSRGB, dontAutogenerateMIPs );
        }
        else
        {
            m_resource = vaDirectXTools::LoadTextureWIC( usedPath.c_str(), assumeSourceIsInSRGB, dontAutogenerateMIPs );
            //m_resource = vaDirectXTools::LoadTextureWIC( fileContents->GetBuffer( ), fileContents->GetLength( ), assumeSourceIsInSRGB, dontAutogenerateMIPs );
        }
    }

    if( m_resource == NULL )
    { 
        // not found? try embedded
        vaFileTools::EmbeddedFileData embeddedFile = vaFileTools::EmbeddedFilesFind( ( L"textures:\\" + storageFilePath ).c_str( ) );
        if( embeddedFile.HasContents( ) )
        {
            if( isDDS )
            {
                m_resource = vaDirectXTools::LoadTextureDDS( embeddedFile.MemStream->GetBuffer( ), embeddedFile.MemStream->GetLength( ), assumeSourceIsInSRGB, dontAutogenerateMIPs );
            }
            else
            {
                m_resource = vaDirectXTools::LoadTextureWIC( embeddedFile.MemStream->GetBuffer( ), embeddedFile.MemStream->GetLength( ), assumeSourceIsInSRGB, dontAutogenerateMIPs );
            }
        }
    }

    if( m_resource == NULL )
    {
        VA_WARN( L"vaTextureDX11::Import - unable to find or load '%s' texture file!", storageFilePath.c_str( ) );

        return false;
    }

    ProcessResource( );    

    return true;
}


void vaTexture::InternalUpdateFromRenderingCounterpart( bool notAllBindViewsNeeded )
{
    vaTextureDX11 * dx11Texture = vaSaferStaticCast<vaTextureDX11*>( this );


    D3D11_RESOURCE_DIMENSION dim;
    dx11Texture->GetResource()->GetType( &dim );

    D3D11_USAGE Usage           = D3D11_USAGE_DEFAULT;
    UINT        BindFlags       = 0;
    UINT        CPUAccessFlags  = 0;
    UINT        MiscFlags       = 0;

    m_type = vaTextureType::Unknown; 

    switch( dim )
    {
        case D3D11_RESOURCE_DIMENSION_UNKNOWN:
        {
            assert( false );   
         }  break;
        case D3D11_RESOURCE_DIMENSION_BUFFER:
        {
            assert( false ); // never debugged - just go through it and make sure it's true

            dx11Texture->m_buffer = vaDirectXTools::QueryResourceInterface<ID3D11Buffer>( dx11Texture->GetResource(), IID_ID3D11Buffer );        
            assert( dx11Texture->m_buffer != NULL );
            if( dx11Texture->m_buffer == NULL )
            {
                dx11Texture->Destroy();
                return;
            }
            m_type = vaTextureType::Buffer;

            D3D11_BUFFER_DESC desc;
            dx11Texture->m_buffer->GetDesc( &desc );
            
            m_sizeX             = desc.ByteWidth;
            m_sizeY             = desc.StructureByteStride;

            Usage               = desc.Usage;
            BindFlags           = desc.BindFlags;
            CPUAccessFlags      = desc.CPUAccessFlags;
            MiscFlags           = desc.MiscFlags;

        }  break;
        case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
        {
            dx11Texture->m_texture1D = vaDirectXTools::QueryResourceInterface<ID3D11Texture1D>( dx11Texture->GetResource( ), IID_ID3D11Texture1D );
            assert( dx11Texture->m_texture1D != NULL );
            if( dx11Texture->m_texture1D == NULL )
            {
                dx11Texture->Destroy( );
                return;
            }

            D3D11_TEXTURE1D_DESC desc;
            dx11Texture->m_texture1D->GetDesc( &desc );

            m_sizeX             = desc.Width;
            m_mipLevels         = desc.MipLevels;
            m_sizeY             = desc.ArraySize;
            m_sizeZ             = 1;
            if( desc.ArraySize > 1 )
                m_type = vaTextureType::Texture1DArray;
            else
                m_type = vaTextureType::Texture1D;
            if( m_resourceFormat != vaTextureFormat::Unknown )
            { assert( m_resourceFormat == (vaTextureFormat)desc.Format ); }
            m_resourceFormat = (vaTextureFormat)desc.Format;
            m_sampleCount       = 1;

            Usage               = desc.Usage;
            BindFlags           = desc.BindFlags;
            CPUAccessFlags      = desc.CPUAccessFlags;
            MiscFlags           = desc.MiscFlags;

        }  break;
        case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
        {
            dx11Texture->m_texture2D = vaDirectXTools::QueryResourceInterface<ID3D11Texture2D>( dx11Texture->GetResource( ), IID_ID3D11Texture2D );
            assert( dx11Texture->m_texture2D != NULL );
            if( dx11Texture->m_texture2D == NULL )
            {
                dx11Texture->Destroy( );
                return;
            }

            D3D11_TEXTURE2D_DESC desc;
            dx11Texture->m_texture2D->GetDesc( &desc );

            m_sizeX             = desc.Width;
            m_sizeY             = desc.Height;
            m_mipLevels         = desc.MipLevels;
            m_sizeZ             = desc.ArraySize;
            if( m_resourceFormat != vaTextureFormat::Unknown )
            { assert( m_resourceFormat == (vaTextureFormat)desc.Format ); }
            m_resourceFormat = (vaTextureFormat)desc.Format;
            m_sampleCount       = desc.SampleDesc.Count;
            //                  = desc.SampleDesc.Quality;

            if( desc.ArraySize > 1 )
            {
                if( desc.SampleDesc.Count > 1 )
                    m_type = vaTextureType::Texture2DMSArray;
                else
                    m_type = vaTextureType::Texture2DArray;
            }
            else
            {
                if( desc.SampleDesc.Count > 1 )
                    m_type = vaTextureType::Texture2DMS;
                else
                    m_type = vaTextureType::Texture2D;
            }

            Usage               = desc.Usage;
            BindFlags           = desc.BindFlags;
            CPUAccessFlags      = desc.CPUAccessFlags;
            MiscFlags           = desc.MiscFlags;
        }  break;
        case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
        {
            dx11Texture->m_texture3D = vaDirectXTools::QueryResourceInterface<ID3D11Texture3D>( dx11Texture->GetResource( ), IID_ID3D11Texture3D );
            assert( dx11Texture->m_texture3D != NULL );
            if( dx11Texture->m_texture3D == NULL )
            {
                dx11Texture->Destroy( );
                return;
            }

            D3D11_TEXTURE3D_DESC desc;
            dx11Texture->m_texture3D->GetDesc( &desc );

            m_sizeX             = desc.Width;
            m_sizeY             = desc.Height;
            m_sizeZ             = desc.Depth;
            m_mipLevels         = desc.MipLevels;
            if( m_resourceFormat != vaTextureFormat::Unknown )
            { assert( m_resourceFormat == (vaTextureFormat)desc.Format ); }
            m_resourceFormat = (vaTextureFormat)desc.Format;
            m_sampleCount       = 1;
            m_type              = vaTextureType::Texture3D;

            Usage               = desc.Usage;
            BindFlags           = desc.BindFlags;
            CPUAccessFlags      = desc.CPUAccessFlags;
            MiscFlags           = desc.MiscFlags;
        }  break;
        default:
        {
            m_type = vaTextureType::Unknown; assert( false );
        }  break;
    }

    m_viewedSliceSizeX = m_sizeX;
    m_viewedSliceSizeY = m_sizeY;
    m_viewedSliceSizeZ = m_sizeZ;
    for( int i = 0; i < m_viewedMipSlice; i++ )
    {
        m_viewedSliceSizeX = (m_viewedSliceSizeX+1) / 2;
        m_viewedSliceSizeY = (m_viewedSliceSizeY+1) / 2;
        m_viewedSliceSizeZ = (m_viewedSliceSizeZ+1) / 2;
    }
    m_viewedSliceSizeX = vaMath::Max( m_viewedSliceSizeX, 1 );
    m_viewedSliceSizeY = vaMath::Max( m_viewedSliceSizeY, 1 );
    m_viewedSliceSizeZ = vaMath::Max( m_viewedSliceSizeZ, 1 );

    Usage;

    m_accessFlags = vaTextureAccessFlags::None;
    if( (CPUAccessFlags & D3D11_CPU_ACCESS_WRITE ) != 0 )
        m_accessFlags = m_accessFlags | vaTextureAccessFlags::CPUWrite;
    if( ( CPUAccessFlags & D3D11_CPU_ACCESS_READ ) != 0 )
        m_accessFlags = m_accessFlags | vaTextureAccessFlags::CPURead;

    // make sure bind flags were set up correctly
    if( !notAllBindViewsNeeded )
    {
        if( (BindFlags & D3D11_BIND_VERTEX_BUFFER       ) != 0 )
            assert( ( m_bindSupportFlags & vaTextureBindSupportFlags::VertexBuffer ) != 0 );
        if( (BindFlags & D3D11_BIND_INDEX_BUFFER        ) != 0 ) 
            assert( ( m_bindSupportFlags & vaTextureBindSupportFlags::IndexBuffer ) != 0 );
        if( (BindFlags & D3D11_BIND_CONSTANT_BUFFER     ) != 0 ) 
            assert( ( m_bindSupportFlags & vaTextureBindSupportFlags::ConstantBuffer ) != 0 );
        if( (BindFlags & D3D11_BIND_SHADER_RESOURCE     ) != 0 ) 
            assert( ( m_bindSupportFlags & vaTextureBindSupportFlags::ShaderResource ) != 0 );
        if( (BindFlags & D3D11_BIND_RENDER_TARGET       ) != 0 ) 
            assert( ( m_bindSupportFlags & vaTextureBindSupportFlags::RenderTarget ) != 0 );
        if( (BindFlags & D3D11_BIND_DEPTH_STENCIL       ) != 0 ) 
            assert( ( m_bindSupportFlags & vaTextureBindSupportFlags::DepthStencil ) != 0 );
        if( (BindFlags & D3D11_BIND_UNORDERED_ACCESS    ) != 0 ) 
            assert( ( m_bindSupportFlags & vaTextureBindSupportFlags::UnorderedAccess ) != 0 );
    }

    MiscFlags;
}

void vaTextureDX11::SetResource( ID3D11Resource * resource, bool notAllBindViewsNeeded )
{
    Destroy();
    m_resource = resource;
    ProcessResource( notAllBindViewsNeeded );
}

void vaTextureDX11::ProcessResource( bool notAllBindViewsNeeded )
{
    InternalUpdateFromRenderingCounterpart( notAllBindViewsNeeded );

    if( ( GetBindSupportFlags( ) & vaTextureBindSupportFlags::VertexBuffer ) != 0 )
    {
        assert( false ); // not implemented yet
    }

    if( ( GetBindSupportFlags( ) & vaTextureBindSupportFlags::IndexBuffer ) != 0 )
    {
        assert( false ); // not implemented yet
    }

    if( ( GetBindSupportFlags( ) & vaTextureBindSupportFlags::ConstantBuffer ) != 0 )
    {
        assert( false ); // not implemented yet
    }

    if( ( GetBindSupportFlags( ) & vaTextureBindSupportFlags::ConstantBuffer ) != 0 )
    {
        assert( false ); // not implemented yet
    }

    if( ( GetBindSupportFlags( ) & vaTextureBindSupportFlags::ShaderResource ) != 0 )
    {
        // not the cleanest way to do this - should probably get updated and also assert on _TYPELESS
        if( GetSRVFormat() == vaTextureFormat::Unknown )
            m_srvFormat = m_resourceFormat;
        m_srv = vaDirectXTools::CreateShaderResourceView( m_resource, (DXGI_FORMAT)GetSRVFormat(), m_viewedMipSlice );
    }

    if( ( GetBindSupportFlags( ) & vaTextureBindSupportFlags::RenderTarget ) != 0 )
    {
        // not the cleanest way to do this - should probably get updated and also assert on _TYPELESS
        if( GetRTVFormat( ) == vaTextureFormat::Unknown )
            m_rtvFormat = m_resourceFormat;
        m_rtv = vaDirectXTools::CreateRenderTargetView( m_resource, (DXGI_FORMAT)GetRTVFormat(), m_viewedMipSlice, m_viewedArraySlice );
    }

    if( ( GetBindSupportFlags( ) & vaTextureBindSupportFlags::DepthStencil ) != 0 )
    {
        // non-0 mip levels not supported at the moment
        assert( m_viewedMipSlice == -1 );

        // not the cleanest way to do this - should probably get updated and also assert on _TYPELESS
        if( GetDSVFormat( ) == vaTextureFormat::Unknown )
            m_dsvFormat = m_resourceFormat;
        m_dsv = vaDirectXTools::CreateDepthStencilView( m_resource, (DXGI_FORMAT)GetDSVFormat() );
    }

    if( ( GetBindSupportFlags( ) & vaTextureBindSupportFlags::UnorderedAccess ) != 0 )
    {
        //// non-0 mip levels not supported at the moment
        //assert( m_viewedMipSlice == -1 );

        // not the cleanest way to do this - should probably get updated and also assert on _TYPELESS
        if( GetUAVFormat( ) == vaTextureFormat::Unknown )
            m_uavFormat = m_resourceFormat;
        m_uav = vaDirectXTools::CreateUnorderedAccessView( m_resource, (DXGI_FORMAT)GetUAVFormat(), m_viewedMipSlice, m_viewedArraySlice );
    }

    if( ( GetBindSupportFlags( ) & vaTextureBindSupportFlags::CreateAutoMipViews ) != 0 )
    {
        assert( false ); // not implemented yet
    }
}

void vaTextureDX11::ClearRTV( vaRenderDeviceContext & context, const vaVector4 & clearValue )
{
    assert( m_rtv != nullptr ); if( m_rtv == nullptr ) return;
    ID3D11DeviceContext * dx11Context = vaSaferStaticCast< vaRenderDeviceContextDX11 * >( &context )->GetDXImmediateContext( );
    dx11Context->ClearRenderTargetView( m_rtv, &clearValue.x );
}

void vaTextureDX11::ClearUAV( vaRenderDeviceContext & context, const vaVector4ui & clearValue )
{
    assert( m_uav != nullptr ); if( m_uav == nullptr ) return;
    ID3D11DeviceContext * dx11Context = vaSaferStaticCast< vaRenderDeviceContextDX11 * >( &context )->GetDXImmediateContext( );
    dx11Context->ClearUnorderedAccessViewUint( m_uav, &clearValue.x );
}

void vaTextureDX11::ClearUAV( vaRenderDeviceContext & context, const vaVector4 & clearValue )
{
    assert( m_uav != nullptr ); if( m_uav == nullptr ) return;
    ID3D11DeviceContext * dx11Context = vaSaferStaticCast< vaRenderDeviceContextDX11 * >( &context )->GetDXImmediateContext( );
    dx11Context->ClearUnorderedAccessViewFloat( m_uav, &clearValue.x );
}

void vaTextureDX11::ClearDSV( vaRenderDeviceContext & context, bool clearDepth, float depthValue, bool clearStencil, uint8 stencilValue )
{
    assert( m_dsv != nullptr ); if( m_dsv == nullptr ) return;
    ID3D11DeviceContext * dx11Context = vaSaferStaticCast< vaRenderDeviceContextDX11 * >( &context )->GetDXImmediateContext( );
    UINT clearFlags = 0;
    if( clearDepth )    clearFlags |= D3D11_CLEAR_DEPTH;
    if( clearStencil )  clearFlags |= D3D11_CLEAR_STENCIL;
    dx11Context->ClearDepthStencilView( m_dsv, clearFlags, depthValue, stencilValue );
}

bool vaTextureDX11::Save( vaStream & outStream )
{
    assert( m_viewedOriginal == nullptr );
    if( m_viewedOriginal != nullptr )
        return false;

    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<int32>( c_fileVersion ) );

    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<vaTextureFlags            > ( m_flags ) );

    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<vaTextureAccessFlags      >( m_accessFlags      ) );
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<vaTextureType             >( m_type             ) );
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<vaTextureBindSupportFlags >( m_bindSupportFlags ) );
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<vaTextureFormat           >( m_resourceFormat   ) );
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<vaTextureFormat           >( m_srvFormat        ) );
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<vaTextureFormat           >( m_rtvFormat        ) );
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<vaTextureFormat           >( m_dsvFormat        ) );
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<vaTextureFormat           >( m_uavFormat        ) );
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<int                       >( m_sizeX            ) );
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<int                       >( m_sizeY            ) );
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<int                       >( m_sizeZ            ) );
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<int                       >( m_sampleCount      ) );
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<int                       >( m_mipLevels        ) );

    int64 posOfSize = outStream.GetPosition( );
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<int64>( 0 ) );

    VERIFY_TRUE_RETURN_ON_FALSE( vaDirectXTools::SaveDDSTexture( outStream, m_resource ) );

    int64 calculatedSize = outStream.GetPosition( ) - posOfSize;
    outStream.Seek( posOfSize );
    VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<int64>( calculatedSize - 8 ) );
    outStream.Seek( posOfSize + calculatedSize );

    return true;
}

bool vaTextureDX11::Load( vaStream & inStream )
{
    Destroy( );

    int32 fileVersion = 0;
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<int32>( fileVersion ) );
    if( fileVersion != c_fileVersion )
    {
        VA_LOG( L"vaRenderMaterial::Load(): unsupported file version" );
        return false;
    }

    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<vaTextureFlags            >( m_flags ) );
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<vaTextureAccessFlags      >( m_accessFlags ) );
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<vaTextureType             >( m_type ) );
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<vaTextureBindSupportFlags >( m_bindSupportFlags ) );
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<vaTextureFormat           >( m_resourceFormat ) );
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<vaTextureFormat           >( m_srvFormat ) );
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<vaTextureFormat           >( m_rtvFormat ) );
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<vaTextureFormat           >( m_dsvFormat ) );
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<vaTextureFormat           >( m_uavFormat ) );
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<int                       >( m_sizeX ) );
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<int                       >( m_sizeY ) );
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<int                       >( m_sizeZ ) );
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<int                       >( m_sampleCount ) );
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<int                       >( m_mipLevels ) );

    int64 textureDataSize;
    VERIFY_TRUE_RETURN_ON_FALSE( inStream.ReadValue<int64                     >( textureDataSize ) );

    // direct reading from the stream not implemented yet
    byte * buffer = new byte[ textureDataSize ];
    if( !inStream.Read( buffer, textureDataSize ) )
    {
        assert( false );
        delete buffer;
        return false;
    }

    m_resource = vaDirectXTools::LoadTextureDDS( buffer, textureDataSize, false, false );
    delete buffer;

    if( m_resource == NULL )
    {
        VA_WARN( L"vaTextureDX11::Load - error processing file!" );
        assert( false );

        return false;
    }

    ProcessResource( );

    return true;
}
