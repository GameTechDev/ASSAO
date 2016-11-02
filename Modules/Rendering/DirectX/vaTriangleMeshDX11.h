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

#include "Rendering/DirectX/vaDirectXCore.h"
#include "Rendering/DirectX/vaDirectXTools.h"

#include "Rendering/vaRenderingIncludes.h"

namespace VertexAsylum
{

    // THIS IS NOT TO BE USED FOR A DYNAMIC MESH!
    template< class VertexType >
    class vaTriangleMeshDX11 : public vaTriangleMesh< VertexType >, private VertexAsylum::vaDirectXNotifyTarget
    {
        VA_RENDERING_MODULE_MAKE_FRIENDS( );

    private:
        bool                                m_dirty;

    private:
        vaDirectXIndexBuffer< uint32 >      m_indexBuffer;
        vaDirectXVertexBuffer< VertexType > m_vertexBuffer;

    protected:
        vaTriangleMeshDX11( const vaConstructorParamsBase * params )
        {
            VA_DIRECTX_NOTIFY_TARGET_CONSTRUCT_PATCH( vaTriangleMeshDX11 );
            m_dirty = true;
        }

        virtual ~vaTriangleMeshDX11( )
        {
            VA_DIRECTX_NOTIFY_TARGET_DESTRUCT_PATCH( vaTriangleMeshDX11 );
        }

    public:
        void                                            UpdateAndSetToD3DContext( ID3D11DeviceContext * context );
        const vaDirectXIndexBuffer< uint32 >      &     GetIndexBuffer( ) const     { return m_indexBuffer; }
        const vaDirectXVertexBuffer< VertexType > &     GetVertexBuffer( ) const    { return m_vertexBuffer; }

        uint32                                          GetIndexCount( ) const      { return (uint32)Indices.size( ); }
        uint32                                          GetVertexCount( ) const     { return (uint32)Vertices.size( ); }

    private:
        // vaDirectXNotifyTarget
        virtual void                                    OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain )
        {
            m_dirty = true;
        }

        virtual void                                    OnDeviceDestroyed( )
        {
            m_indexBuffer.Destroy( );
            m_vertexBuffer.Destroy( );
            m_dirty = true;
        }

    private:
        virtual void                                    SetDataDirty( )
        {
            m_indexBuffer.Destroy( );
            m_vertexBuffer.Destroy( );
            m_dirty = true;
        }

    };

    typedef vaTriangleMeshDX11<vaPositionColorNormalTangentTexcoord1Vertex> vaTriangleMeshDX11_PositionColorNormalTangentTexcoord1Vertex;

    template<class VertexType>
    inline void vaTriangleMeshDX11<VertexType>::UpdateAndSetToD3DContext( ID3D11DeviceContext * context )
    {
        if( m_dirty )
        {
            m_indexBuffer.Destroy( );
            m_vertexBuffer.Destroy( );

            if( Indices.size( ) > 0 )
            {
                m_indexBuffer.Create( (int)Indices.size( ), &Indices[0] );
                m_vertexBuffer.Create( (int)Vertices.size( ), &Vertices[0] );
            }
            m_dirty = false;
        }

        m_indexBuffer.SetToD3DContext( context );
        m_vertexBuffer.SetToD3DContext( context );
    }

}