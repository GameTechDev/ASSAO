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

#include "vaTexture.h"

using namespace VertexAsylum;

vaTexture::vaTexture( const vaConstructorParamsBase * params ) : 
    vaAssetResource( vaSaferStaticCast< const vaTextureConstructorParams *, const vaConstructorParamsBase *>( params )->UID )
{ 
    m_bindSupportFlags      = vaTextureBindSupportFlags::None;
    m_resourceFormat        = vaTextureFormat::Unknown;
    m_srvFormat             = vaTextureFormat::Unknown;
    m_rtvFormat             = vaTextureFormat::Unknown;
    m_dsvFormat             = vaTextureFormat::Unknown;
    m_uavFormat             = vaTextureFormat::Unknown;

    m_accessFlags           = vaTextureAccessFlags::None;
    m_type                  = vaTextureType::Unknown;
    
    m_sizeX                 = 0;
    m_sizeY                 = 0;
    m_sizeZ                 = 0;
    m_sampleCount           = 0;
    m_mipLevels             = 0;

    m_viewedSliceSizeX      = 0;
    m_viewedSliceSizeY      = 0;
    m_viewedSliceSizeZ      = 0;

    m_viewedMipSlice        = -1;
    m_viewedArraySlice      = -1;

    assert( vaRenderingCore::IsInitialized( ) ); 
}

void vaTexture::Initialize( vaTextureBindSupportFlags binds, vaTextureFormat resourceFormat, vaTextureFormat srvFormat, vaTextureFormat rtvFormat, vaTextureFormat dsvFormat, vaTextureFormat uavFormat, vaTextureFlags flags, int viewedMipSlice, int viewedArraySlice )
{
    m_bindSupportFlags  = binds;
    m_resourceFormat    = resourceFormat;
    m_srvFormat         = srvFormat;
    m_rtvFormat         = rtvFormat;
    m_dsvFormat         = dsvFormat;
    m_uavFormat         = uavFormat;
    m_flags             = flags;
    m_viewedMipSlice    = viewedMipSlice;
    m_viewedArraySlice  = viewedArraySlice;

    // no point having format if no bind support - bind flag maybe forgotten?
    if( m_srvFormat != vaTextureFormat::Unknown )
    {
        assert( ( m_bindSupportFlags & vaTextureBindSupportFlags::ShaderResource ) != 0 );
    }
    if( m_rtvFormat != vaTextureFormat::Unknown )
    {
        assert( ( m_bindSupportFlags & vaTextureBindSupportFlags::RenderTarget ) != 0 );
    }
    if( m_dsvFormat != vaTextureFormat::Unknown )
    {
        assert( ( m_bindSupportFlags & vaTextureBindSupportFlags::DepthStencil ) != 0 );
    }
    if( m_uavFormat != vaTextureFormat::Unknown )
    {
        assert( ( m_bindSupportFlags & vaTextureBindSupportFlags::UnorderedAccess ) != 0 );
    }
}

vaTexture * vaTexture::Create2DTestCheckerboardTexture( vaTextureFormat format, vaTextureBindSupportFlags bindFlags, vaTextureAccessFlags accessFlags )
{
    assert( format == vaTextureFormat::R8G8B8A8_UNORM_SRGB );
    if( format != vaTextureFormat::R8G8B8A8_UNORM_SRGB )
        return NULL;

    const uint32 dim = 32;
    uint32 buffer[dim*dim];
    int bufferPitch = sizeof( *buffer ) * 32;
    for( int y = 0; y < dim; y++ )
    {
        for( int x = 0; x < dim; x++ )
        {
            uint32 & pixel = buffer[dim*y + x];
            if( x < dim / 2 )
            {
                if( y < dim / 2 )
                {
                    if( ( x + y ) % 2 == 0 )
                        pixel = vaVector4::ToRGBA( vaVector4( 1.0f, 1.0f, 1.0f, 1.0f ) );
                    else
                        pixel = vaVector4::ToRGBA( vaVector4( 0.0f, 0.0f, 0.0f, 1.0f ) );
                }
                else
                {
                    if( ( x + y ) % 2 == 0 )
                        pixel = vaVector4::ToRGBA( vaVector4( 1.0f, 1.0f, 1.0f, 1.0f ) );
                    else
                        pixel = vaVector4::ToRGBA( vaVector4( 0.0f, 1.0f, 0.0f, 0.0f ) );
                }
            }
            else
            {
                if( y < dim / 2 )
                {
                    if( ( x + y ) % 2 == 0 )
                        pixel = vaVector4::ToRGBA( vaVector4( 1.0f, 1.0f, 1.0f, 0.0f ) );
                    else
                        pixel = vaVector4::ToRGBA( vaVector4( 1.0f, 0.0f, 0.0f, 1.0f ) );
                }
                else
                {
                    if( ( ( x * 8 / dim ) % 2 ) == ( ( y * 8 / dim ) % 2 ) )
                        pixel = vaVector4::ToRGBA( vaVector4( 1.0f, 1.0f, 1.0f, 1.0f ) );
                    else
                        pixel = vaVector4::ToRGBA( vaVector4( 0.0f, 0.0f, 0.0f, 1.0f ) );
                }
            }
        }
    }

    return vaTexture::Create2D( vaTextureFormat::R8G8B8A8_UNORM_SRGB, dim, dim, 1, 1, 1, vaTextureBindSupportFlags::ShaderResource, vaTextureAccessFlags::None, (void*)buffer, bufferPitch );
}



