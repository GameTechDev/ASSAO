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

#include "Rendering/Effects/vaSimpleShadowMap.h"

using namespace VertexAsylum;

vaSimpleShadowMap::vaSimpleShadowMap( )
{ 
    m_resolution = 0;

    m_view      = vaMatrix4x4::Identity;
    m_proj      = vaMatrix4x4::Identity;
    m_viewProj  = vaMatrix4x4::Identity;

    m_texelSize = vaVector2( 0.0f, 0.0f );
}

vaSimpleShadowMap::~vaSimpleShadowMap( )
{
}

void vaSimpleShadowMap::Initialize( int resolution )
{
    SetResolution( resolution );
}

void vaSimpleShadowMap::SetResolution( int resolution )
{
    if( m_resolution != resolution )
    {
        m_resolution = resolution;

        m_shadowMap = std::shared_ptr<vaTexture>( vaTexture::Create2D( vaTextureFormat::R16_TYPELESS, resolution, resolution, 1, 1, 1, vaTextureBindSupportFlags::DepthStencil | vaTextureBindSupportFlags::ShaderResource, vaTextureAccessFlags::None, NULL, 0, vaTextureFormat::R16_UNORM, vaTextureFormat::Unknown, vaTextureFormat::D16_UNORM) );

        InternalResolutionOrTexelWorldSizeChanged( );
    }
}

void vaSimpleShadowMap::UpdateArea( const vaOrientedBoundingBox & volume )
{
    m_volume = volume;

    m_view                  = vaMatrix4x4( volume.Axis.Transpose() );
    m_view.r3.x             = -vaVector3::Dot( volume.Axis.r0, volume.Center );
    m_view.r3.y             = -vaVector3::Dot( volume.Axis.r1, volume.Center );
    m_view.r3.z             = -vaVector3::Dot( volume.Axis.r2, volume.Center );

    m_view.r3.AsVec3()      += vaVector3( 0.0f, 0.0f, 1.0f ) * volume.Extents.z * 1.0f;

    m_proj                  = vaMatrix4x4::OrthoLH( volume.Extents.x*2.0f, volume.Extents.y*2.0f, 0.0f, volume.Extents.z * 2.0f );

    m_viewProj              = m_view * m_proj;

    vaVector2 newTexelSize;
    newTexelSize.x          = volume.Extents.x * 2.0f / (float)m_resolution;
    newTexelSize.y          = volume.Extents.y * 2.0f / (float)m_resolution;

    if( !vaVector2::CloseEnough( newTexelSize, m_texelSize ) )
    {
        InternalResolutionOrTexelWorldSizeChanged( );
        m_texelSize         = newTexelSize;
    }
}
