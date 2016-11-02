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

#include "Core/vaCoreIncludes.h"

#include "Rendering/DirectX/vaDirectXShader.h"
#include "Rendering/DirectX/vaTriangleMeshDX11.h"

namespace VertexAsylum
{
    namespace
    {
        typedef vaTriangleMeshDX11<vaRenderMesh::StandardVertex>          StandardTriangleMeshDX11;
    };

    struct vaRenderMaterialCachedShadersDX11
    {
        struct Key
        {
            wstring                     WStringPart;
            string                      AStringPart;

            bool                        operator == ( const Key & cmp ) const { return ( this->WStringPart == cmp.WStringPart ) && ( this->AStringPart == cmp.AStringPart ); }
            bool                        operator >( const Key & cmp ) const { return (this->WStringPart > cmp.WStringPart) || ((this->WStringPart == cmp.WStringPart) && (this->AStringPart > cmp.AStringPart) ); }
            bool                        operator < ( const Key & cmp ) const { return (this->WStringPart < cmp.WStringPart) || ((this->WStringPart == cmp.WStringPart) && (this->AStringPart < cmp.AStringPart) ); }

            // alphatest is part of the key because it determines whether PS_DepthOnly is needed at all; all other shader parameters are contained in shaderMacros
            Key( const wstring & fileName, bool alphaTest, const string & entryVS_PosOnly, const string & entryPS_DepthOnly, const string & entryVS_Standard, string & entryPS_Forward, const string & entryPS_Deferred, const vector< pair< string, string > > & shaderMacros )
            {
                WStringPart = fileName;
                AStringPart = ((alphaTest)?("a&"):("b&")) + entryVS_PosOnly + "&" + entryPS_DepthOnly + "&" + entryVS_Standard + "&" + entryPS_Forward + "&" + entryPS_Deferred;
                for( int i = 0; i < shaderMacros.size(); i++ )
                    AStringPart += shaderMacros[i].first + "&" + shaderMacros[i].second;
            }
        };

        vaDirectXVertexShader                   VS_PosOnly;
        vaDirectXPixelShader                    PS_DepthOnly;
        vaDirectXVertexShader                   VS_Standard;
        vaDirectXPixelShader                    PS_Forward;
        vaDirectXPixelShader                    PS_Deferred;
    };

    class vaRenderMaterialDX11 : public vaRenderMaterial, VertexAsylum::vaDirectXNotifyTarget
    {
        VA_RENDERING_MODULE_MAKE_FRIENDS( );

    private:
        shared_ptr<vaRenderMaterialCachedShadersDX11> 
                                                m_shaders;

        vaDirectXConstantsBuffer< RenderMeshMaterialConstants >
                                                m_constantsBuffer;

    protected:
        vaRenderMaterialDX11( const vaConstructorParamsBase * param );
        ~vaRenderMaterialDX11( );

    private:
        // vaDirectXNotifyTarget
        virtual void                            OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain );
        virtual void                            OnDeviceDestroyed( );

    private:
        // vaRenderMaterial
        virtual void                            UploadToAPIContext( vaDrawContext & drawContext );
    };

    class vaRenderMaterialManagerDX11 : public vaRenderMaterialManager, VertexAsylum::vaDirectXNotifyTarget
    {
        VA_RENDERING_MODULE_MAKE_FRIENDS( );

    private:
        map< vaRenderMaterialCachedShadersDX11::Key, weak_ptr<vaRenderMaterialCachedShadersDX11> >
                                                m_cachedShaders;

    protected:
        vaRenderMaterialManagerDX11( const vaConstructorParamsBase * params );
        ~vaRenderMaterialManagerDX11( );

    private:
        // vaDirectXNotifyTarget
        virtual void                            OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain )  { }
        virtual void                            OnDeviceDestroyed( )                                                { }

    public:
        shared_ptr<vaRenderMaterialCachedShadersDX11> 
                                                FindOrCreateShaders( const wstring & fileName, bool alphaTest, const string & entryVS_PosOnly, const string & entryPS_DepthOnly, const string & entryVS_Standard, string & entryPS_Forward, const string & entryPS_Deferred, const vector< pair< string, string > > & shaderMacros );

    public:
    };

}

