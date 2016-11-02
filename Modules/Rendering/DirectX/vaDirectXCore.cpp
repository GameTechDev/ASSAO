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

#include "vaDirectXCore.h"
#include <assert.h>
#include <algorithm>

#include "Rendering/vaAssetPack.h"

#include "vaDirectXShader.h"

#pragma warning (default : 4995)

using namespace std;
using namespace VertexAsylum;


//
vaDirectXCore::vaDirectXCore( void )
{
    m_pd3dDevice = NULL;
    m_pSwapChain = NULL;
    m_immediateContext = NULL;
    memset( &m_backBufferSurfaceDesc, 0, sizeof( m_backBufferSurfaceDesc ) );

    m_traversingNotifyTargets = false;
    m_notifyTargetsCurrentMinZeroedIndex = INT_MAX;
    m_notifyTargetsNumberOfZeroed = 0;

    m_shaderManager = new vaDirectXShaderManager( );
}
//
vaDirectXCore::~vaDirectXCore( void )
{
    if( vaAssetPackManager::GetInstancePtr() )
        vaAssetPackManager::GetInstance().OnRenderingAPIAboutToShutdown();

    delete m_shaderManager;

    PostDeviceDestroyed( );

    assert( m_pd3dDevice == NULL );
    assert( m_pSwapChain == NULL );
    assert( m_notifyTargets.size( ) == 0 );
    assert( !m_traversingNotifyTargets );

    //vaDirectXTextureStreaming::DeleteInstance();
}
//
void vaDirectXCore::TickInternal( )
{
    // defragment and remove NULL pointers from the vector, in a quick way
    if( m_notifyTargetsNumberOfZeroed > 0 )
    {
        int lastNonZero = ( (int)m_notifyTargets.size( ) ) - 1;
        while( ( lastNonZero >= 0 ) && ( m_notifyTargets[lastNonZero] == NULL ) ) lastNonZero--;
        for( int i = m_notifyTargetsCurrentMinZeroedIndex; ( i < lastNonZero ) && ( m_notifyTargetsNumberOfZeroed>0 ); i++ )
        {
            if( m_notifyTargets[i] == NULL )
            {
                // swap with the last
                m_notifyTargets[i] = m_notifyTargets[lastNonZero];
                m_notifyTargets[lastNonZero] = NULL;
                // have to update the index now!
                m_notifyTargets[i]->m_storageIndex = i;
                // optimization
                m_notifyTargetsNumberOfZeroed--;
                while( ( lastNonZero >= 0 ) && ( m_notifyTargets[lastNonZero] == NULL ) ) lastNonZero--;
                if( lastNonZero < 0 )
                    break;
            }
        }
        while( ( lastNonZero >= 0 ) && ( m_notifyTargets[lastNonZero] == NULL ) ) lastNonZero--;
        if( lastNonZero < 0 )
        {
            m_notifyTargets.clear( );
        }
        else
        {
            m_notifyTargets.resize( lastNonZero + 1 );
        }
        m_notifyTargetsCurrentMinZeroedIndex = INT_MAX;
        m_notifyTargetsNumberOfZeroed = 0;
    }
}
//
void vaDirectXCore::RegisterNotifyTarget( vaDirectXNotifyTarget * rh )
{
    vaDirectXCore::GetInstance( ).RegisterNotifyTargetInternal( rh );
}

void vaDirectXCore::UnregisterNotifyTarget( vaDirectXNotifyTarget * rh )
{
    vaDirectXCore::GetInstance( ).UnregisterNotifyTargetInternal( rh );
}

void vaDirectXTools_OnDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain );
void vaDirectXTools_OnDeviceDestroyed( );

void vaDirectXCore::EarlySetDevice( const DXGI_ADAPTER_DESC1 & adapterDesc, ID3D11Device* device )
{
    assert( m_pd3dDevice == NULL );

    wstring name = wstring( adapterDesc.Description );
    std::replace( name.begin(), name.end(), L' ', L'_' );

    m_adapterNameShort  = name;
    m_adapterNameID     = vaStringTools::Format( L"%s-%08X_%08X", name.c_str(), adapterDesc.DeviceId, adapterDesc.Revision );

    m_pd3dDevice = device;
    m_pd3dDevice->AddRef( );
}

void vaDirectXCore::PostDeviceCreated( ID3D11Device* device, IDXGISwapChain* swapChain )
{
    if( device == NULL )
    {
        m_pd3dDevice = device;
        m_pd3dDevice->AddRef( );
    }
    else
    {
        assert( m_pd3dDevice == device );
    }
    assert( m_pSwapChain == NULL );
    m_pSwapChain = swapChain;
    m_pSwapChain->AddRef( );
    device->GetImmediateContext( &m_immediateContext );

    //vaDirectXTextureStreaming::GetInstance()->OnDeviceCreated( device, swapChain );
    vaDirectXTools_OnDeviceCreated( device, swapChain );

    assert( !m_traversingNotifyTargets );
    m_traversingNotifyTargets = true;

    // ensure the vector is defragmented and all NULL ptrs are removed
    TickInternal( );

    for( NotifyTargetContainerType::const_iterator it = m_notifyTargets.begin( ); it != m_notifyTargets.end( ); it++ )
        ( *it )->OnDeviceCreated( device, swapChain );

    assert( m_traversingNotifyTargets );
    m_traversingNotifyTargets = false;

}
//
void vaDirectXCore::PostDeviceDestroyed( )
{
    SAFE_RELEASE( m_immediateContext );
    SAFE_RELEASE( m_pd3dDevice );
    SAFE_RELEASE( m_pSwapChain );

    assert( !m_traversingNotifyTargets );
    m_traversingNotifyTargets = true;

    // ensure the vector is defragmented and all NULL ptrs are removed
    TickInternal( );

    for( NotifyTargetContainerType::const_reverse_iterator it = m_notifyTargets.rbegin( ); it != m_notifyTargets.rend( ); it++ )
        ( *it )->OnDeviceDestroyed( );

    assert( m_traversingNotifyTargets );
    m_traversingNotifyTargets = false;

    vaDirectXTools_OnDeviceDestroyed( );
    //vaDirectXTextureStreaming::GetInstance()->OnDeviceDestroyed();
}
//
void vaDirectXCore::PostReleasingSwapChain( )
{
    assert( !m_traversingNotifyTargets );
    m_traversingNotifyTargets = true;

    // ensure the vector is defragmented and all NULL ptrs are removed
    TickInternal( );

    for( NotifyTargetContainerType::const_reverse_iterator it = m_notifyTargets.rbegin( ); it != m_notifyTargets.rend( ); it++ )
        ( *it )->OnReleasingSwapChain( );

    assert( m_traversingNotifyTargets );
    m_traversingNotifyTargets = false;
}
//
void vaDirectXCore::PostResizedSwapChain( const DXGI_SURFACE_DESC & backBufferSurfaceDesc )
{
    m_backBufferSurfaceDesc = backBufferSurfaceDesc;

    assert( !m_traversingNotifyTargets );
    m_traversingNotifyTargets = true;

    // ensure the vector is defragmented and all NULL ptrs are removed
    TickInternal( );

    for( NotifyTargetContainerType::const_iterator it = m_notifyTargets.begin( ); it != m_notifyTargets.end( ); it++ )
        ( *it )->OnResizedSwapChain( backBufferSurfaceDesc );

    assert( m_traversingNotifyTargets );
    m_traversingNotifyTargets = false;
}
//
void vaDirectXCore::RegisterNotifyTargetInternal( vaDirectXNotifyTarget * rh )
{
    if( m_traversingNotifyTargets )
        VA_ERROR( L"Registering new vaDirectXNotifyTarget from the notification callback is currently not supported." );

    m_notifyTargets.push_back( rh );

    rh->m_storageIndex = ( (int)m_notifyTargets.size( ) ) - 1;
}
//
void vaDirectXCore::UnregisterNotifyTargetInternal( vaDirectXNotifyTarget * rh )
{
    if( m_traversingNotifyTargets )
        VA_ERROR( L"Registering new vaDirectXNotifyTarget from the notification callback is currently not supported." );

    assert( rh->m_storageIndex >= 0 );
    assert( m_notifyTargets[rh->m_storageIndex] == rh );
    m_notifyTargets[rh->m_storageIndex] = NULL;

    m_notifyTargetsCurrentMinZeroedIndex = vaMath::Min( m_notifyTargetsCurrentMinZeroedIndex, rh->m_storageIndex );
    m_notifyTargetsNumberOfZeroed++;
    rh->m_storageIndex = 0;
}
//
vaDirectXNotifyTarget::vaDirectXNotifyTarget( )
{
    m_helperMacroConstructorCalled = false;
    m_helperMacroDesctructorCalled = false;

    m_storageIndex = -1;
    vaDirectXCore::RegisterNotifyTarget( this );
}
//
vaDirectXNotifyTarget::~vaDirectXNotifyTarget( )
{
    assert( m_helperMacroConstructorCalled );   // forgot to put VA_DIRECTX_NOTIFY_TARGET_CONSTRUCT_PATCH in your constructor?
    assert( m_helperMacroDesctructorCalled );   // forgot to put VA_DIRECTX_NOTIFY_TARGET_DESTRUCT_PATCH in your destructor?

    vaDirectXCore::UnregisterNotifyTarget( this );
}
//
void vaDirectXCore::NameObject( ID3D11DeviceChild * object, const char * permanentNameString )
{
#ifndef NDEBUG
    // Only works if device is created with the D3D10 or D3D11 debug layer, or when attached to PIX for Windows
    object->SetPrivateData( WKPDID_D3DDebugObjectName, (UINT)strlen( permanentNameString ), permanentNameString );
#endif
}
//
void vaDirectXCore::NameObject( ID3D11Resource * resource, const char * permanentNameString )
{
#ifndef NDEBUG
    // Only works if device is created with the D3D10 or D3D11 debug layer, or when attached to PIX for Windows
    resource->SetPrivateData( WKPDID_D3DDebugObjectName, (UINT)strlen( permanentNameString ), permanentNameString );
#endif
}
//
void vaDirectXCore::ProcessInformationQueue( )
{
    //ID3D11InfoQueue* infoQueue = NULL;
    //HRESULT hr = GetDevice()->QueryInterface( __uuidof( ID3D11InfoQueue ), ( LPVOID* )&infoQueue );
    //if( SUCCEEDED( hr ) && (infoQueue != NULL) )
    //{
    //   BOOL mute = infoQueue->GetMuteDebugOutput();
    //   infoQueue->SetMuteDebugOutput( FALSE ) ;
    //}
}
