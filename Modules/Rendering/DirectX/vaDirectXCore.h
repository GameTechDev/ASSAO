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

#include "vaDirectXIncludes.h"

//#pragma warning (disable : 4995)
//#include <vector>

namespace VertexAsylum
{

    class vaDirectXNotifyTarget;
    class vaDirectXShaderManager;

    // Singleton utility class for accessing and handling the main DirectX device
    class vaDirectXCore : public vaSingletonBase < vaDirectXCore >
    {
    private:
        friend class vaDirectXShaderManager;
        friend class vaTextureManager;

    private:
        ID3D11Device*                               m_pd3dDevice;
        IDXGISwapChain*                             m_pSwapChain;
        DXGI_SURFACE_DESC 			                m_backBufferSurfaceDesc;
        ID3D11DeviceContext*                        m_immediateContext;
        //
        typedef std::vector<vaDirectXNotifyTarget*> NotifyTargetContainerType;
        NotifyTargetContainerType                   m_notifyTargets;
        int                                         m_notifyTargetsCurrentMinZeroedIndex;
        int                                         m_notifyTargetsNumberOfZeroed;
        //
        bool                                        m_traversingNotifyTargets;
        //
        vaDirectXShaderManager *                    m_shaderManager;
        //
        wstring                                     m_adapterNameShort;
        wstring                                     m_adapterNameID;
        //
    private:
        friend class vaRenderDeviceDX11;
        //
                                            vaDirectXCore( );
                                            ~vaDirectXCore( );
        //
        void                                EarlySetDevice( const DXGI_ADAPTER_DESC1 & adapterDesc, ID3D11Device* device );
        //
        void                                PostDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain );
        void                                PostDeviceDestroyed( );
        void                                PostReleasingSwapChain( );
        void                                PostResizedSwapChain( const DXGI_SURFACE_DESC & backBufferSurfaceDesc );
        //
        void                                TickInternal( );
        //
    public:
        //
        static ID3D11Device *               GetDevice( )                   { return GetInstance( ).m_pd3dDevice; }
        static ID3D11DeviceContext *        GetImmediateContext( )         { return GetInstance( ).m_immediateContext; }
        static IDXGISwapChain *             GetSwapChain( )                { return GetInstance( ).m_pSwapChain; }
        static const DXGI_SURFACE_DESC &    GetBackBufferSurfaceDesc( )    { return GetInstance( ).m_backBufferSurfaceDesc; }
        //
        static void                         NameObject( ID3D11DeviceChild * object, const char * permanentNameString );
        static void                         NameObject( ID3D11Resource * resourceobject, const char * permanentNameString );
        //
        static void                         ProcessInformationQueue( );
        //
        // generic GPU-specific name - used to serialize adapter-specific stuff like settings or shader cache
        const wstring &                     GetAdapterNameShort( ) const    { return m_adapterNameShort; }
        const wstring &                     GetAdapterNameID( ) const       { return m_adapterNameID; }
        //
    private:
        friend class vaDirectXNotifyTarget;
        //
        void                                RegisterNotifyTargetInternal( vaDirectXNotifyTarget * rh );
        void                                UnregisterNotifyTargetInternal( vaDirectXNotifyTarget * rh );
        //
        static void                         RegisterNotifyTarget( vaDirectXNotifyTarget * rh );
        static void                         UnregisterNotifyTarget( vaDirectXNotifyTarget * rh );
        //
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // also check out VA_DIRECTX_NOTIFY_TARGET_CONSTRUCT_PATCH and VA_DIRECTX_NOTIFY_TARGET_DESTRUCT_PATCH helpers below!
    class vaDirectXNotifyTarget
    {
    protected:
        // used for fast searching
        int			m_storageIndex;
        //
        friend class vaDirectXCore;
        //
        bool        m_helperMacroConstructorCalled;
        bool        m_helperMacroDesctructorCalled;

    protected:
        //
        vaDirectXNotifyTarget( );
        virtual ~vaDirectXNotifyTarget( );
        //
        virtual void                  OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain )      {}
        virtual void                  OnDeviceDestroyed( )                                                    {}
        virtual void                  OnReleasingSwapChain( )                                                 {}
        virtual void                  OnResizedSwapChain( const DXGI_SURFACE_DESC & backBufferSurfaceDesc )   {}
    };
    //
    // Put these in your constructor/destructor if you need to ensure that your type's callbacks are called if the object
    // is created after the device is created (and/or if it's deleted before the device is destroyed).
    // It is needed in almost all cases, so it is asserted on! to disable assert set the stuff below manually
#define VA_DIRECTX_NOTIFY_TARGET_CONSTRUCT_PATCH( MyType )                                      \
   VA_ASSERT( typeid(MyType) == typeid(*this), L"Invalid MyType type." );                       \
   if( (vaDirectXCore::GetDevice() != NULL) && ( vaDirectXCore::GetSwapChain() != NULL ) )      \
      {                                                                                         \
      MyType::OnDeviceCreated( vaDirectXCore::GetDevice(), vaDirectXCore::GetSwapChain() );     \
      MyType::OnResizedSwapChain( vaDirectXCore::GetBackBufferSurfaceDesc() );                  \
      }                                                                                         \
    m_helperMacroConstructorCalled = true;                                                   
    //
#define VA_DIRECTX_NOTIFY_TARGET_DESTRUCT_PATCH( MyType )                                       \
   VA_ASSERT( typeid(MyType) == typeid(*this), L"Invalid MyType type." );                       \
   if( (vaDirectXCore::GetDevice() != NULL) && ( vaDirectXCore::GetSwapChain() != NULL ) )      \
      {                                                                                         \
      MyType::OnReleasingSwapChain( );                                                          \
      MyType::OnDeviceDestroyed( );                                                             \
      }                                                                                         \
    m_helperMacroDesctructorCalled = true;                                                    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //! A template COM smart pointer.
    template<typename I>
    class vaCOMSmartPtr
    {
    public:
        //! Constructs an empty smart pointer.
        vaCOMSmartPtr( ) : m_p( 0 ) {}

        //! Assumes ownership of the given instance, if non-null.
        /*! \param	p	A pointer to an existing COM instance, or 0 to create an
                    empty COM smart pointer.
                    \note The smart pointer will assume ownership of the given instance.
                    It will \b not AddRef the contents, but it will Release the object
                    as it goes out of scope.
                    */
        vaCOMSmartPtr( I* p ) : m_p( p ) {}

        //! Releases the contained instance.
        ~vaCOMSmartPtr( )
        {
            SafeRelease( );
        }

        //! Copy-construction.
        vaCOMSmartPtr( vaCOMSmartPtr<I> const& ptr ) : m_p( ptr.m_p )
        {
            if( m_p )
                m_p->AddRef( );
        }

        //! Assignment.
        vaCOMSmartPtr<I>& operator=( vaCOMSmartPtr<I> const& ptr )
        {
            vaCOMSmartPtr<I> copy( ptr );
            Swap( copy );
            return *this;
        }

        //! Releases a contained instance, if present.
        /*! \note You should never need to call this function unless you wish to
           take control a Release an instance before the smart pointer goes out
           of scope.
           */
        void SafeRelease( )
        {
            if( m_p )
                m_p->Release( );
            m_p = 0;
        }

        //! Explicitly gets the address of the pointer.
        /*! \note This function should not be called on a smart pointer with
           non-zero contents. This is to avoid memory leaks by blatting over the
           contents without calling Release. Hence the complaint to std::cerr.
           */
        I** AddressOf( )
        {
            if( m_p )
            {
                std::cerr << __FUNCTION__
                    << ": non-zero contents - possible memory leak" << std::endl;
            }
            return &m_p;
        }

        //! Gets the encapsulated pointer.
        I* Get( ) const { return m_p; }

        //! Gets the encapsulated pointer.
        I* operator->( ) const { return m_p; }

        //! Swaps the encapsulated pointer with that of the argument.
        void Swap( vaCOMSmartPtr<I>& ptr )
        {
            I* p = m_p;
            m_p = ptr.m_p;
            ptr.m_p = p;
        }

        //! Returns true if non-empty.
        operator bool( ) const { return m_p != 0; }

    private:
        //! The encapsulated instance.
        I* m_p;
    };

}
