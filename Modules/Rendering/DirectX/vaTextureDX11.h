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

    class vaTextureDX11 : public vaTexture, private VertexAsylum::vaDirectXNotifyTarget
    {
        VA_RENDERING_MODULE_MAKE_FRIENDS( );

    private:
        ID3D11Resource *                m_resource;
        ID3D11Buffer *                  m_buffer;
        ID3D11Texture1D *               m_texture1D;
        ID3D11Texture2D *               m_texture2D;
        ID3D11Texture3D *               m_texture3D;
        ID3D11ShaderResourceView *      m_srv;
        ID3D11RenderTargetView *        m_rtv;
        ID3D11DepthStencilView *        m_dsv;
        ID3D11UnorderedAccessView *     m_uav;

    protected:
        friend class vaTexture;
                                        vaTextureDX11( const vaConstructorParamsBase * params );
        virtual                         ~vaTextureDX11( )   ;

        bool                            Import( const wstring & storageFilePath, bool assumeSourceIsInSRGB, bool dontAutogenerateMIPs, vaTextureBindSupportFlags binds );
        void                            Destroy( );

    public:
        ID3D11Resource *                GetResource( )          { return m_resource; }
        ID3D11Texture1D *               GetTexture1D( )         { return m_texture1D; }
        ID3D11Texture2D *               GetTexture2D( )         { return m_texture2D; }
        ID3D11Texture3D *               GetTexture3D( )         { return m_texture3D; }
        ID3D11ShaderResourceView *      GetSRV( )               { return m_srv; }
        ID3D11RenderTargetView *        GetRTV( )               { return m_rtv; }
        ID3D11DepthStencilView *        GetDSV( )               { return m_dsv; }
        ID3D11UnorderedAccessView *     GetUAV( )               { return m_uav; }

    public:
        static vaTexture *              CreateWrap( ID3D11Resource * resource, vaTextureFormat srvFormat = vaTextureFormat::Unknown, vaTextureFormat rtvFormat = vaTextureFormat::Unknown, vaTextureFormat dsvFormat = vaTextureFormat::Unknown, vaTextureFormat uavFormat = vaTextureFormat::Unknown );

    private:
        void                            SetResource( ID3D11Resource * resource, bool notAllBindViewsNeeded = false );
        void                            ProcessResource( bool notAllBindViewsNeeded = false );

    private:
        // vaDirectXNotifyTarget
        virtual void                    OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain );
        virtual void                    OnDeviceDestroyed( );

    protected:
        // vaTexture
        virtual void                    ClearRTV( vaRenderDeviceContext & context, const vaVector4 & clearValue );
        virtual void                    ClearUAV( vaRenderDeviceContext & context, const vaVector4ui & clearValue );
        virtual void                    ClearUAV( vaRenderDeviceContext & context, const vaVector4 & clearValue );
        virtual void                    ClearDSV( vaRenderDeviceContext & context, bool clearDepth, float depthValue, bool clearStencil, uint8 stencilValue );

        virtual bool                    Load( vaStream & inStream );
        virtual bool                    Save( vaStream & outStream );
    };


}