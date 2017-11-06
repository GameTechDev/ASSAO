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

#include "vaDirectXCore.h"


namespace VertexAsylum
{

   class vaStream;
   class vaSimpleProfiler;

   //class vaDirectXNotifyTarget;

   class vaDirectXTools
   {
      //enum TextureLoadFlags
      //{
      //   TLF_NoFlags                      = 0,
      //   //TLF_InputI
      //};

   public:
      // Helper DirectX functions
      static ID3D11Texture1D *              CreateTexture1D(    DXGI_FORMAT format, UINT width, D3D11_SUBRESOURCE_DATA * initialData = NULL, 
                                                                UINT arraySize = 1, UINT mipLevels = 0, UINT bindFlags = D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
                                                                UINT cpuaccessFlags = 0, UINT miscFlags = 0 );
      static ID3D11Texture2D *              CreateTexture2D(    DXGI_FORMAT format, UINT width, UINT height, D3D11_SUBRESOURCE_DATA * initialData = NULL, 
                                                                UINT arraySize = 1, UINT mipLevels = 0, UINT bindFlags = D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
                                                                UINT cpuaccessFlags = 0, UINT sampleCount = 1, UINT sampleQuality = 0, UINT miscFlags = 0 );
      static ID3D11Texture3D *              CreateTexture3D(    DXGI_FORMAT format, UINT width, UINT height, UINT depth, D3D11_SUBRESOURCE_DATA * initialData = NULL, UINT mipLevels = 0,
                                                                UINT bindFlags = D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE usage = D3D11_USAGE_DEFAULT, UINT cpuaccessFlags = 0, UINT miscFlags = 0 );

      static ID3D11ShaderResourceView *   CreateShaderResourceView( ID3D11Resource * texture, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, int mipSlice = -1, int arraySlice = -1 );
      static ID3D11DepthStencilView *     CreateDepthStencilView( ID3D11Resource * texture, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN );
      static ID3D11RenderTargetView *     CreateRenderTargetView( ID3D11Resource * texture, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, int mipSlice = -1, int arraySlice = -1 );
      static ID3D11UnorderedAccessView *  CreateUnorderedAccessView( ID3D11Resource * texture, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN );
      static ID3D11Buffer *               CreateBuffer( uint32 sizeInBytes, uint32 bindFlags, D3D11_USAGE usage, uint32 cpuAccessFlags = 0, uint32 miscFlags = 0, uint32 structByteStride = 0, const void * initializeData = NULL );

      // Common depth/stencil states
      static ID3D11DepthStencilState *    GetDSS_DepthEnabledL_DepthWrite();
      static ID3D11DepthStencilState *    GetDSS_DepthEnabledG_DepthWrite();
      static ID3D11DepthStencilState *    GetDSS_DepthEnabledL_NoDepthWrite();
      static ID3D11DepthStencilState *    GetDSS_DepthEnabledLE_NoDepthWrite();
      static ID3D11DepthStencilState *    GetDSS_DepthEnabledG_NoDepthWrite();
      static ID3D11DepthStencilState *    GetDSS_DepthEnabledGE_NoDepthWrite();
      static ID3D11DepthStencilState *    GetDSS_DepthDisabled_NoDepthWrite();
      static ID3D11DepthStencilState *    GetDSS_DepthPassAlways_DepthWrite();

      static ID3D11DepthStencilState *    GetDSS_DepthDisabled_NoDepthWrite_StencilCreateMask();
      static ID3D11DepthStencilState *    GetDSS_DepthDisabled_NoDepthWrite_StencilUseMask();

      // Common rasterizer states
      static ID3D11RasterizerState *      GetRS_CullNone_Fill();
      static ID3D11RasterizerState *      GetRS_CullCCW_Fill();
      static ID3D11RasterizerState *      GetRS_CullCW_Fill();
      static ID3D11RasterizerState *      GetRS_CullNone_Wireframe( );
      static ID3D11RasterizerState *      GetRS_CullCCW_Wireframe();
      static ID3D11RasterizerState *      GetRS_CullCW_Wireframe( );

      // Common blend states
      static ID3D11BlendState *           GetBS_Opaque();
      static ID3D11BlendState *           GetBS_Additive();
      static ID3D11BlendState *           GetBS_AlphaBlend();
      static ID3D11BlendState *           GetBS_PremultAlphaBlend();
      static ID3D11BlendState *           GetBS_Mult( );

      // Common samplers
      static ID3D11SamplerState *         GetSamplerStatePointClamp      ( );
      static ID3D11SamplerState *         GetSamplerStatePointWrap       ( );
      static ID3D11SamplerState *         GetSamplerStatePointMirror     ( );
      static ID3D11SamplerState *         GetSamplerStateLinearClamp     ( );
      static ID3D11SamplerState *         GetSamplerStateLinearWrap      ( );
      static ID3D11SamplerState *         GetSamplerStateAnisotropicClamp( );
      static ID3D11SamplerState *         GetSamplerStateAnisotropicWrap ( );

      static ID3D11SamplerState * *       GetSamplerStatePtrPointClamp( );
      static ID3D11SamplerState * *       GetSamplerStatePtrPointWrap( );
      static ID3D11SamplerState * *       GetSamplerStatePtrLinearClamp( );
      static ID3D11SamplerState * *       GetSamplerStatePtrLinearWrap( );
      static ID3D11SamplerState * *       GetSamplerStatePtrAnisotropicClamp( );
      static ID3D11SamplerState * *       GetSamplerStatePtrAnisotropicWrap( );

      // Commonly used textures, etc
      static ID3D11ShaderResourceView *   GetTexture2D_SRV_White1x1();

      // Helper state-related functions
      static ID3D11RasterizerState *      CreateRasterizerState( const D3D11_RASTERIZER_DESC & desc );

      static ID3D11RasterizerState *      FindOrCreateRasterizerState( const D3D11_RASTERIZER_DESC & desc );
      
      // Helper basic rendering functions
      static void                         ClearColorDepthStencil( ID3D11DeviceContext * destContext, bool clearAllColorRTs, bool clearDepth, bool clearStencil, const vaVector4 & clearColor, float depth, uint8 stencil );
      static void                         CopyDepthStencil( ID3D11DeviceContext * destContext, ID3D11DepthStencilView * srcDSV );

      // Helper texture-related functions
      static int                          CalcApproxTextureSizeInMemory( DXGI_FORMAT format, int width, int height, int mipCount );
      static ID3D11Resource *             LoadTextureDDS( void * buffer, int64 bufferSize, bool assumeSourceIsInSRGB, bool dontAutogenerateMIPs, uint64 * outCRC = NULL );
      static ID3D11Resource *             LoadTextureDDS( const wchar_t * path, bool assumeSourceIsInSRGB, bool dontAutogenerateMIPs, uint64 * outCRC = NULL );
      static ID3D11Resource *             LoadTextureWIC( void * buffer, int64 bufferSize, bool assumeSourceIsInSRGB, bool dontAutogenerateMIPs, uint64 * outCRC = NULL );
      static ID3D11Resource *             LoadTextureWIC( const wchar_t * path, bool assumeSourceIsInSRGB, bool dontAutogenerateMIPs, uint64 * outCRC = NULL );
      //static ID3D11Texture2D *            LoadTexture2D( void * buffer, int64 bufferSize, bool assumeSourceIsInSRGB, bool dontAutogenerateMIPs, uint64 * outCRC = NULL );
      //static ID3D11Texture2D *            LoadTexture2D( const wchar_t * path, bool assumeSourceIsInSRGB, bool dontAutogenerateMIPs, uint64 * outCRC = NULL );
      //static ID3D11ShaderResourceView *   LoadTextureSRV( const wchar_t * path, bool assumeSourceIsInSRGB, bool dontAutogenerateMIPs, uint64 * outCRC = NULL );
      //static ID3D11Resource *             LoadTexture( VertexAsylum::vaStream * intStream );
      //static HRESULT                      StreamTexture( vaDirectXTextureStreaming::StreamingTask * outStorage, vaStream * inStream );

      // Misc
      static void                         RenderProfiler( int x, int y, int width, int height, vaSimpleProfiler * profiler );

      // // Save any DirectX resource wrapped in our own simple format
      // static bool                         SaveResource( VertexAsylum::vaStream & outStream, ID3D11Resource * d3dResource );
      // static ID3D11Resource *             LoadResource( VertexAsylum::vaStream & inStream, uint64 * outCRC = NULL );

      static bool                           SaveDDSTexture( VertexAsylum::vaStream & outStream, ID3D11Resource* pSource );

      template<typename OutType>
      static inline OutType *               QueryResourceInterface( ID3D11Resource * d3dResource, REFIID riid );
   
      static void                           SetToD3DContextAllShaderTypes( ID3D11DeviceContext * context, ID3D11Buffer * buffer, int32 slot );
      static void                           AssertSetToD3DContextAllShaderTypes( ID3D11DeviceContext * context, ID3D11Buffer * buffer, int32 slot );

      static void                           SetToD3DContextAllShaderTypes( ID3D11DeviceContext * context, ID3D11SamplerState * sampler, int32 slot );
      static void                           SetToD3DContextAllShaderTypes( ID3D11DeviceContext * context, ID3D11SamplerState ** samplers, int32 slot, int32 count );
      static void                           AssertSetToD3DContextAllShaderTypes( ID3D11DeviceContext * context, ID3D11SamplerState * buffer, int32 slot );

      static void                           SetToD3DContextAllShaderTypes( ID3D11DeviceContext * context, ID3D11ShaderResourceView * buffer, int32 slot );
      static void                           AssertSetToD3DContextAllShaderTypes( ID3D11DeviceContext * context, ID3D11ShaderResourceView * buffer, int32 slot );
   };


   // Utility class for creating vertex/index/shconst buffers
   template< typename ElementType >
   class vaDirectXBuffer
   {
   protected:
      ID3D11Buffer *                      m_buffer;
      int                                 m_elementCount;

   public:
                                          vaDirectXBuffer() : m_buffer(NULL), m_elementCount(0) { }
      virtual                             ~vaDirectXBuffer()                                    { Destroy(); assert( m_buffer == NULL ); }

      void                                Create( int elementCount, uint32 bindFlags, const ElementType * initializeData, D3D11_USAGE usage = D3D11_USAGE_DEFAULT, uint32 cpuAccessFlags = 0, uint32 miscFlags = 0 );
      void                                Create( int elementCount, ID3D11Resource * resource, bool verify, uint32 bindFlags, D3D11_USAGE usage = D3D11_USAGE_DEFAULT, uint32 cpuAccessFlags = 0, uint32 miscFlags = 0 );
      void                                Destroy( );
      
      int                                 GetElementCount( ) const            { return m_elementCount; }
      int64                               GetSizeInBytes( ) const             { return m_elementCount * sizeof(ElementType); }
      ID3D11Buffer *                      GetBuffer()                         { return m_buffer; }
                                          operator ID3D11Buffer *const * ()   { return &m_buffer; }

      std::shared_ptr<vaDirectXBuffer<ElementType>>
                                          CreateStagingCopy( ID3D11DeviceContext * copyContext ) const;
   };

   // vaDirectXBuffer specialization that handles only shader constants buffers
   template< typename ElementType, int DefaultSlot = -1 >
   class vaDirectXConstantsBuffer : public vaDirectXBuffer<ElementType>
   {
   public:
                                          vaDirectXConstantsBuffer()    { }
      virtual                             ~vaDirectXConstantsBuffer()   { }

      void                                Create( const ElementType * initializeData = NULL );
      void                                Destroy( );
      void                                Update( ID3D11DeviceContext * context, const ElementType & data );
      void                                SetToD3DContextAllShaderTypes( ID3D11DeviceContext * context, int32 slot = DefaultSlot ) const;

      ID3D11Buffer *                      GetBuffer()                         { return m_buffer; }
                                          operator ID3D11Buffer *const * ()   { return &m_buffer; }
   };

   // vaDirectXBuffer specialization that handles only vertex buffers
   template< typename ElementType >
   class vaDirectXVertexBuffer : public vaDirectXBuffer<ElementType>
   {
   public:
                                          vaDirectXVertexBuffer()    { }
      virtual                             ~vaDirectXVertexBuffer()   { }

      void                                Create( int elementCount, const ElementType * initializeData = NULL, D3D11_USAGE usage = D3D11_USAGE_DEFAULT, uint32 cpuAccessFlags = 0 );
      void                                Create( int elementCount, D3D11_USAGE usage = D3D11_USAGE_DEFAULT, uint32 cpuAccessFlags = 0 );
      void                                Create( int elementCount, ID3D11Resource * resource, D3D11_USAGE usage = D3D11_USAGE_DEFAULT, uint32 cpuAccessFlags = 0 );
      void                                Destroy( );
      void                                Update( ID3D11DeviceContext * context, int elementCount, const ElementType * data );

      int                                 GetElementCount( ) const            { return m_elementCount; }
      ID3D11Buffer *                      GetBuffer()                         { return m_buffer; }
                                          operator ID3D11Buffer *const * ()   { return &m_buffer; }

      void                                SetToD3DContext( ID3D11DeviceContext * context, uint32 slot = 0, uint32 offsetInBytes = 0 ) const;
   };

   // vaDirectXBuffer specialization that handles only index buffers
   template< typename ElementType >
   class vaDirectXIndexBuffer : public vaDirectXBuffer<ElementType>
   {
   public:
                                          vaDirectXIndexBuffer()    { }
      virtual                             ~vaDirectXIndexBuffer()   { }

      void                                Create( int elementCount, const ElementType * initializeData = NULL );
      void                                Create( int elementCount, ID3D11Resource * resource );
      void                                Destroy( );
      void                                Update( ID3D11DeviceContext * context, int elementCount, const ElementType * data );

      int                                 GetElementCount( ) const            { return m_elementCount; }
      ID3D11Buffer *                      GetBuffer()                         { return m_buffer; }
                                          operator ID3D11Buffer *const * ()   { return &m_buffer; }

      DXGI_FORMAT                         GetFormat() const                   { assert( (sizeof(ElementType) == 2) || (sizeof(ElementType) == 4) ); return (sizeof(ElementType) == 2)?(DXGI_FORMAT_R16_UINT):(DXGI_FORMAT_R32_UINT); }
      void                                SetToD3DContext( ID3D11DeviceContext * context, uint32 offset = 0 ) const;
   };
   /*
   class vaDirectXTexture
   {
   protected:
      DXGI_FORMAT                   m_format;
      DXGI_FORMAT                   m_formatSRV;
      ID3D11Texture2D *             m_texture;
      ID3D11ShaderResourceView *    m_textureSRV;

      int                           m_width;
      int                           m_height;

   public:
      vaDirectXTexture() : m_format(DXGI_FORMAT_UNKNOWN), m_texture( NULL ), m_textureSRV( NULL ) {}
      virtual ~vaDirectXTexture() { Destroy(); }

      void Create( DXGI_FORMAT format, DXGI_FORMAT formatSRV, UINT width, UINT height, UINT arraySize = 1, UINT mipLevels = 1, UINT sampleCount = 1, UINT sampleQuality = 0, UINT additionalBindFlags = 0, UINT miscFlags = 0, D3D11_SUBRESOURCE_DATA * initialData = NULL );
      void Destroy();

      ID3D11Texture2D *                   GetTexture() const         { return m_texture;     }
      ID3D11ShaderResourceView *          GetSRV() const             { return m_textureSRV;  }

      ID3D11ShaderResourceView * const *  GetSRVArr() const          { return &m_textureSRV; }

      DXGI_FORMAT                         GetFormat() const          { return m_format;      }
      DXGI_FORMAT                         GetFormatSRV() const       { return m_formatSRV;   }

      int                                 GetWidth() const           { return m_width; }
      int                                 GetHeight() const          { return m_height; }
   };

   // Utility class for creating and using a render target
   class vaDirectXRenderTarget : public vaDirectXTexture
   {
   protected:
      ID3D11RenderTargetView *      m_textureRTV;
      DXGI_FORMAT                   m_formatRTV;

   public:
      vaDirectXRenderTarget() : m_textureRTV( NULL ) {}
      virtual ~vaDirectXRenderTarget() { Destroy(); }

      void Create( DXGI_FORMAT format, DXGI_FORMAT formatSRV, DXGI_FORMAT formatRTV, UINT width, UINT height, UINT arraySize = 1, UINT mipLevels = 1, UINT sampleCount = 1, UINT sampleQuality = 0, UINT additionalBindFlags = 0, UINT miscFlags = 0, D3D11_SUBRESOURCE_DATA * initialData = NULL );
      void Destroy();

      ID3D11RenderTargetView *            GetRTV() const             { return m_textureRTV;  }

      ID3D11RenderTargetView * const *    GetRTVArr() const          { return &m_textureRTV; }

      DXGI_FORMAT                         GetFormatRTV() const       { return m_formatRTV; }
   };


   // Utility class for creating and using a depth buffer
   class vaDirectXDepthStencilBuffer
   {
   private:
      DXGI_FORMAT                   m_formatTex;
      DXGI_FORMAT                   m_formatSRV;
      DXGI_FORMAT                   m_formatDSV;
      ID3D11Texture2D *             m_texture;
      ID3D11ShaderResourceView *    m_textureSRV;
      ID3D11DepthStencilView *      m_textureDSV;
      int                           m_width;
      int                           m_height;

   public:
      vaDirectXDepthStencilBuffer() : m_formatTex(DXGI_FORMAT_UNKNOWN), m_formatSRV(DXGI_FORMAT_UNKNOWN), m_formatDSV(DXGI_FORMAT_UNKNOWN), m_texture( NULL ), m_textureSRV( NULL ), m_textureDSV( NULL ), m_width(0), m_height(0) {}
      virtual ~vaDirectXDepthStencilBuffer() { Destroy(); }

      void Create( DXGI_FORMAT formatTex, DXGI_FORMAT formatSRV, DXGI_FORMAT formatDSV, UINT width, UINT height, UINT arraySize = 1, UINT sampleCount = 1, UINT sampleQuality = 0, UINT miscFlags = 0 );
      void CreateCopy( vaDirectXDepthStencilBuffer * copyFrom );
      void Destroy();

      bool                                IsCreated() const          { return m_texture != NULL; }

      ID3D11Texture2D *                   GetTexture() const         { return m_texture;     }
      ID3D11ShaderResourceView *          GetSRV() const             { return m_textureSRV;  }
      ID3D11DepthStencilView *            GetDSV() const             { return m_textureDSV;  }

      ID3D11ShaderResourceView * const *  GetSRVArr() const          { return &m_textureSRV; }

      DXGI_FORMAT                         GetFormatTex() const       { return m_formatTex; }
      DXGI_FORMAT                         GetFormatSRV() const       { return m_formatSRV; }
      DXGI_FORMAT                         GetFormatDSV() const       { return m_formatDSV; }
      
      int                                 GetWidth() const           { return m_width; } 
      int                                 GetHeight() const          { return m_height; } 
   };
   */

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Inline definitions
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   //
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // vaDirectXTools
   template<typename OutType>
   static inline OutType * vaDirectXTools::QueryResourceInterface( ID3D11Resource * d3dResource, REFIID riid )
   {
      OutType * ret = NULL;
      if( SUCCEEDED( d3dResource->QueryInterface( riid, (void**)&ret ) ) )
      {
         return ret;
      }
      else
      {
         return NULL;
      }
   }
   //
   inline void vaDirectXTools::SetToD3DContextAllShaderTypes( ID3D11DeviceContext * context, ID3D11Buffer * buffer, int32 slot )
   {
       VA_ASSERT( slot >= 0, L"vaDirectXTools::SetToD3DContextAllShaderTypes : If you've left the DefaultSlot template parameter at default then you have to specify a valid shader slot!" )

        context->PSSetConstantBuffers( (uint32)slot, 1, &buffer );
        context->CSSetConstantBuffers( (uint32)slot, 1, &buffer );
        context->VSSetConstantBuffers( (uint32)slot, 1, &buffer );
        context->DSSetConstantBuffers( (uint32)slot, 1, &buffer );
        context->GSSetConstantBuffers( (uint32)slot, 1, &buffer );
        context->HSSetConstantBuffers( (uint32)slot, 1, &buffer );
   }
   //
   inline void vaDirectXTools::AssertSetToD3DContextAllShaderTypes( ID3D11DeviceContext * context, ID3D11Buffer * buffer, int32 slot )
   {
#ifdef _DEBUG
       ID3D11Buffer * _buffer = NULL;
       context->PSGetConstantBuffers( (uint32)slot, 1, &_buffer ); assert( buffer == _buffer ); SAFE_RELEASE( _buffer );
       context->CSGetConstantBuffers( (uint32)slot, 1, &_buffer ); assert( buffer == _buffer ); SAFE_RELEASE( _buffer );
       context->VSGetConstantBuffers( (uint32)slot, 1, &_buffer ); assert( buffer == _buffer ); SAFE_RELEASE( _buffer );
       context->DSGetConstantBuffers( (uint32)slot, 1, &_buffer ); assert( buffer == _buffer ); SAFE_RELEASE( _buffer );
       context->GSGetConstantBuffers( (uint32)slot, 1, &_buffer ); assert( buffer == _buffer ); SAFE_RELEASE( _buffer );
       context->HSGetConstantBuffers( (uint32)slot, 1, &_buffer ); assert( buffer == _buffer ); SAFE_RELEASE( _buffer );
#endif
   }
   //
   inline void vaDirectXTools::SetToD3DContextAllShaderTypes( ID3D11DeviceContext * context, ID3D11SamplerState * sampler, int32 slot )
   {
       VA_ASSERT( slot >= 0, L"vaDirectXTools::SetToD3DContextAllShaderTypes : If you've left the DefaultSlot template parameter at default then you have to specify a valid shader slot!" );

       context->PSSetSamplers( (uint32)slot, 1, &sampler );
       context->CSSetSamplers( (uint32)slot, 1, &sampler );
       context->VSSetSamplers( (uint32)slot, 1, &sampler );
       context->DSSetSamplers( (uint32)slot, 1, &sampler );
       context->GSSetSamplers( (uint32)slot, 1, &sampler );
       context->HSSetSamplers( (uint32)slot, 1, &sampler );
   }
   //
   inline void vaDirectXTools::SetToD3DContextAllShaderTypes( ID3D11DeviceContext * context, ID3D11SamplerState ** samplers, int32 slot, int32 count )
   {
       VA_ASSERT( slot >= 0, L"vaDirectXTools::SetToD3DContextAllShaderTypes : If you've left the DefaultSlot template parameter at default then you have to specify a valid shader slot!" );

       context->PSSetSamplers( (uint32)slot, count, samplers );
       context->CSSetSamplers( (uint32)slot, count, samplers );
       context->VSSetSamplers( (uint32)slot, count, samplers );
       context->DSSetSamplers( (uint32)slot, count, samplers );
       context->GSSetSamplers( (uint32)slot, count, samplers );
       context->HSSetSamplers( (uint32)slot, count, samplers );
   }
   //
   inline void vaDirectXTools::AssertSetToD3DContextAllShaderTypes( ID3D11DeviceContext * context, ID3D11SamplerState * sampler, int32 slot )
   {
#ifdef _DEBUG
       ID3D11SamplerState * _sampler = NULL;
       context->PSGetSamplers( (uint32)slot, 1, &_sampler ); assert( sampler == _sampler ); SAFE_RELEASE( _sampler );
       context->CSGetSamplers( (uint32)slot, 1, &_sampler ); assert( sampler == _sampler ); SAFE_RELEASE( _sampler );
       context->VSGetSamplers( (uint32)slot, 1, &_sampler ); assert( sampler == _sampler ); SAFE_RELEASE( _sampler );
       context->DSGetSamplers( (uint32)slot, 1, &_sampler ); assert( sampler == _sampler ); SAFE_RELEASE( _sampler );
       context->GSGetSamplers( (uint32)slot, 1, &_sampler ); assert( sampler == _sampler ); SAFE_RELEASE( _sampler );
       context->HSGetSamplers( (uint32)slot, 1, &_sampler ); assert( sampler == _sampler ); SAFE_RELEASE( _sampler );
#endif
   }
   //
   inline void vaDirectXTools::SetToD3DContextAllShaderTypes( ID3D11DeviceContext * context, ID3D11ShaderResourceView * srv, int32 slot )
   {
       VA_ASSERT( slot >= 0, L"vaDirectXTools::SetToD3DContextAllShaderTypes : If you've left the DefaultSlot template parameter at default then you have to specify a valid shader slot!" );

       context->PSSetShaderResources( (uint32)slot, 1, &srv );
       context->CSSetShaderResources( (uint32)slot, 1, &srv );
       context->VSSetShaderResources( (uint32)slot, 1, &srv );
       context->DSSetShaderResources( (uint32)slot, 1, &srv );
       context->GSSetShaderResources( (uint32)slot, 1, &srv );
       context->HSSetShaderResources( (uint32)slot, 1, &srv );
   }
   //
   inline void vaDirectXTools::AssertSetToD3DContextAllShaderTypes( ID3D11DeviceContext * context, ID3D11ShaderResourceView * srv, int32 slot )
   {
#ifdef _DEBUG
       ID3D11ShaderResourceView * _srv = NULL;
       context->PSGetShaderResources( (uint32)slot, 1, &_srv ); assert( srv == _srv ); SAFE_RELEASE( _srv );
       context->CSGetShaderResources( (uint32)slot, 1, &_srv ); assert( srv == _srv ); SAFE_RELEASE( _srv );
       context->VSGetShaderResources( (uint32)slot, 1, &_srv ); assert( srv == _srv ); SAFE_RELEASE( _srv );
       context->DSGetShaderResources( (uint32)slot, 1, &_srv ); assert( srv == _srv ); SAFE_RELEASE( _srv );
       context->GSGetShaderResources( (uint32)slot, 1, &_srv ); assert( srv == _srv ); SAFE_RELEASE( _srv );
       context->HSGetShaderResources( (uint32)slot, 1, &_srv ); assert( srv == _srv ); SAFE_RELEASE( _srv );
#endif
   }
   //
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // vaDirectXBuffer
   template< typename ElementType >
   void vaDirectXBuffer<ElementType>::Create( int elementCount, uint32 bindFlags, const ElementType * initializeData, D3D11_USAGE usage, uint32 cpuAccessFlags, uint32 miscFlags )
   {
      assert( m_buffer == NULL );
      SAFE_RELEASE( m_buffer );

      m_elementCount = elementCount;
      m_buffer = vaDirectXTools::CreateBuffer( elementCount * sizeof(ElementType), bindFlags, usage, cpuAccessFlags, miscFlags, 0, initializeData );
   }
   //
   template< typename ElementType >
   void vaDirectXBuffer<ElementType>::Create( int elementCount, ID3D11Resource * resource, bool verify, uint32 bindFlags, D3D11_USAGE usage, uint32 cpuAccessFlags, uint32 miscFlags )
   {
      assert( m_buffer == NULL );
      SAFE_RELEASE( m_buffer );

      m_buffer = vaDirectXTools::QueryResourceInterface<ID3D11Buffer>( resource, IID_ID3D11Buffer );
      if( m_buffer == NULL )
      {
         assert( false );
      }
      else
      {
         if( verify )
         {
            D3D11_BUFFER_DESC desc;
            m_buffer->GetDesc( &desc );
            assert( desc.Usage == usage );
            assert( desc.CPUAccessFlags == cpuAccessFlags );
            assert( desc.MiscFlags == miscFlags );
            int expectedSize = sizeof(ElementType) * elementCount;
            //assert( desc.ByteWidth == expectedSize );
         }
      }
   }
   //
   template< typename ElementType >
   void vaDirectXBuffer<ElementType>::Destroy( )
   {
      SAFE_RELEASE( m_buffer );
   }
   //
   template< typename ElementType >
   std::shared_ptr<vaDirectXBuffer<ElementType>> vaDirectXBuffer<ElementType>::CreateStagingCopy( ID3D11DeviceContext * copyContext ) const
   {
      if( m_buffer == NULL )
         return std::shared_ptr<vaDirectXBuffer<ElementType>>( NULL );

      std::shared_ptr<vaDirectXBuffer<ElementType>> newBuffer ( new vaDirectXBuffer<ElementType>( ) );
      newBuffer->Create( m_elementCount, 0, NULL, D3D11_USAGE_STAGING, D3D11_CPU_ACCESS_READ );

      copyContext->CopyResource( newBuffer->GetBuffer(), m_buffer );

      return newBuffer;
   }
   //
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // vaDirectXConstantsBuffer
   template< typename ElementType, int DefaultSlot >
   void vaDirectXConstantsBuffer<ElementType, DefaultSlot>::Create( const ElementType * initializeData )
   {
      vaDirectXBuffer<ElementType>::Create( 1, D3D11_BIND_CONSTANT_BUFFER, initializeData, D3D11_USAGE_DEFAULT );
   }
   //
   template< typename ElementType, int DefaultSlot >
   void vaDirectXConstantsBuffer<ElementType, DefaultSlot>::Destroy( )
   {
      vaDirectXBuffer<ElementType>::Destroy();
   }
   //
   template< typename ElementType, int DefaultSlot >
   void vaDirectXConstantsBuffer<ElementType, DefaultSlot>::Update( ID3D11DeviceContext * context, const ElementType & data )
   {
      assert( m_buffer != NULL ); 
      context->UpdateSubresource( m_buffer, 0, NULL, &data, sizeof(ElementType) * m_elementCount, 0 );
   }
   //
   template< typename ElementType, int DefaultSlot >
   void vaDirectXConstantsBuffer<ElementType, DefaultSlot>::SetToD3DContextAllShaderTypes( ID3D11DeviceContext * context, int32 slot ) const
   {
       vaDirectXTools::SetToD3DContextAllShaderTypes( context, m_buffer, slot );
   }
   //
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // vaDirectXVertexBuffer
   //
   template< typename ElementType >
   void vaDirectXVertexBuffer<ElementType>::Create( int elementCount, const ElementType * initializeData, D3D11_USAGE usage, uint32 cpuAccessFlags )
   {
      vaDirectXBuffer<ElementType>::Create( elementCount, D3D11_BIND_VERTEX_BUFFER, initializeData, usage, cpuAccessFlags );
   }
   //
   template< typename ElementType >
   void vaDirectXVertexBuffer<ElementType>::Create( int elementCount, D3D11_USAGE usage, uint32 cpuAccessFlags )
   {
       vaDirectXBuffer<ElementType>::Create( elementCount, D3D11_BIND_VERTEX_BUFFER, NULL, usage, cpuAccessFlags );
   }
   //
   template< typename ElementType >
   void vaDirectXVertexBuffer<ElementType>::Create( int elementCount, ID3D11Resource * resource, D3D11_USAGE usage, uint32 cpuAccessFlags )
   {
      vaDirectXBuffer<ElementType>::Create( elementCount, resource, true, D3D11_BIND_VERTEX_BUFFER, usage, cpuAccessFlags );
   }
   //
   template< typename ElementType >
   void vaDirectXVertexBuffer<ElementType>::Destroy( )
   {
      vaDirectXBuffer<ElementType>::Destroy();
   }
   //
   template< typename ElementType >
   void vaDirectXVertexBuffer<ElementType>::Update( ID3D11DeviceContext * context, int elementCount, const ElementType * data )
   {
      assert( m_elementCount >= elementCount );
      context->UpdateSubresource( m_buffer, 0, NULL, data, sizeof(ElementType) * m_elementCount, 0 );
   }
   //
   template< typename ElementType >
   void vaDirectXVertexBuffer<ElementType>::SetToD3DContext( ID3D11DeviceContext * context, uint32 slot, uint32 offsetInBytes ) const
   {
      const uint32 stride = sizeof(ElementType);
      context->IASetVertexBuffers( slot, 1, &m_buffer, &stride, &offsetInBytes );
   }
   //
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // vaDirectXIndexBuffer
   //
   template< typename ElementType >
   void vaDirectXIndexBuffer<ElementType>::Create( int elementCount, const ElementType * initializeData )
   {
      // Must be uint16 or uint32
      assert( sizeof(ElementType) == 2 || sizeof(ElementType) == 4 );

      vaDirectXBuffer<ElementType>::Create( elementCount, D3D11_BIND_INDEX_BUFFER, initializeData, D3D11_USAGE_DEFAULT );
   }
   //
   template< typename ElementType >
   void vaDirectXIndexBuffer<ElementType>::Create( int elementCount, ID3D11Resource * resource )
   {
      vaDirectXBuffer<ElementType>::Create( elementCount, resource, true, D3D11_BIND_INDEX_BUFFER, D3D11_USAGE_DEFAULT );
   }
   //
   template< typename ElementType >
   void vaDirectXIndexBuffer<ElementType>::Destroy( )
   {
      vaDirectXBuffer<ElementType>::Destroy();
   }
   //
   template< typename ElementType >
   void vaDirectXIndexBuffer<ElementType>::Update( ID3D11DeviceContext * context, int elementCount, const ElementType * data )
   {
      assert( m_elementCount >= elementCount );
      context->UpdateSubresource( m_buffer, 0, NULL, data, sizeof(ElementType) * m_elementCount, 0 );
   }
   //
   template< typename ElementType >
   void vaDirectXIndexBuffer<ElementType>::SetToD3DContext( ID3D11DeviceContext * context, uint32 offset ) const
   {
      context->IASetIndexBuffer( m_buffer, GetFormat(), offset );
   }
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
