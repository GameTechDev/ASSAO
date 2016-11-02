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

#include "vaAssetImporter.h"
#include "Rendering/vaStandardShapes.h"

#include <d3d11_1.h>

#pragma warning(disable : 4121)

#define DIRECTX_CTOR_DEFAULT =default;
#define DIRECTX_CTOR_DELETE =delete;

#include <DirectXMath.h>
#pragma warning(disable : 4324 4481)

#include <exception>

//#include <mutex>

namespace DirectX
{
    // Helper utility converts D3D API failures into exceptions.
    inline void ThrowIfFailed( HRESULT hr )
    {
        if( FAILED( hr ) )
        {
            throw std::exception( );
        }
    }


    // Helper for output debug tracing
    inline void DebugTrace( _In_z_ _Printf_format_string_ const char* format, ... )
    {
#ifdef _DEBUG
        va_list args;
        va_start( args, format );

        char buff[1024] = { 0 };
        vsprintf_s( buff, format, args );
        OutputDebugStringA( buff );
        va_end( args );
#else
        UNREFERENCED_PARAMETER( format );
#endif
    }


    // Helper smart-pointers
    struct handle_closer { void operator()( HANDLE h ) { if( h ) CloseHandle( h ); } };

    typedef public std::unique_ptr<void, handle_closer> ScopedHandle;

    inline HANDLE safe_handle( HANDLE h ) { return ( h == INVALID_HANDLE_VALUE ) ? 0 : h; }
}


namespace DirectX
{
#if (DIRECTX_MATH_VERSION < 305) && !defined(XM_CALLCONV)
#define XM_CALLCONV __fastcall
    typedef const XMVECTOR& HXMVECTOR;
    typedef const XMMATRIX& FXMMATRIX;
#endif

    // Vertex struct holding position and color information.
    struct VertexPositionColor
    {
        VertexPositionColor( ) DIRECTX_CTOR_DEFAULT

            VertexPositionColor( XMFLOAT3 const& position, XMFLOAT4 const& color )
            : position( position ),
            color( color )
        { }

        VertexPositionColor( FXMVECTOR position, FXMVECTOR color )
        {
            XMStoreFloat3( &this->position, position );
            XMStoreFloat4( &this->color, color );
        }

        XMFLOAT3 position;
        XMFLOAT4 color;

        static const int InputElementCount = 2;
        static const D3D11_INPUT_ELEMENT_DESC InputElements[InputElementCount];
    };


    // Vertex struct holding position and texture mapping information.
    struct VertexPositionTexture
    {
        VertexPositionTexture( ) DIRECTX_CTOR_DEFAULT

            VertexPositionTexture( XMFLOAT3 const& position, XMFLOAT2 const& textureCoordinate )
            : position( position ),
            textureCoordinate( textureCoordinate )
        { }

        VertexPositionTexture( FXMVECTOR position, FXMVECTOR textureCoordinate )
        {
            XMStoreFloat3( &this->position, position );
            XMStoreFloat2( &this->textureCoordinate, textureCoordinate );
        }

        XMFLOAT3 position;
        XMFLOAT2 textureCoordinate;

        static const int InputElementCount = 2;
        static const D3D11_INPUT_ELEMENT_DESC InputElements[InputElementCount];
    };


    // Vertex struct holding position and normal vector.
    struct VertexPositionNormal
    {
        VertexPositionNormal( ) DIRECTX_CTOR_DEFAULT

            VertexPositionNormal( XMFLOAT3 const& position, XMFLOAT3 const& normal )
            : position( position ),
            normal( normal )
        { }

        VertexPositionNormal( FXMVECTOR position, FXMVECTOR normal )
        {
            XMStoreFloat3( &this->position, position );
            XMStoreFloat3( &this->normal, normal );
        }

        XMFLOAT3 position;
        XMFLOAT3 normal;

        static const int InputElementCount = 2;
        static const D3D11_INPUT_ELEMENT_DESC InputElements[InputElementCount];
    };


    // Vertex struct holding position, color, and texture mapping information.
    struct VertexPositionColorTexture
    {
        VertexPositionColorTexture( ) DIRECTX_CTOR_DEFAULT

            VertexPositionColorTexture( XMFLOAT3 const& position, XMFLOAT4 const& color, XMFLOAT2 const& textureCoordinate )
            : position( position ),
            color( color ),
            textureCoordinate( textureCoordinate )
        { }

        VertexPositionColorTexture( FXMVECTOR position, FXMVECTOR color, FXMVECTOR textureCoordinate )
        {
            XMStoreFloat3( &this->position, position );
            XMStoreFloat4( &this->color, color );
            XMStoreFloat2( &this->textureCoordinate, textureCoordinate );
        }

        XMFLOAT3 position;
        XMFLOAT4 color;
        XMFLOAT2 textureCoordinate;

        static const int InputElementCount = 3;
        static const D3D11_INPUT_ELEMENT_DESC InputElements[InputElementCount];
    };


    // Vertex struct holding position, normal vector, and color information.
    struct VertexPositionNormalColor
    {
        VertexPositionNormalColor( ) DIRECTX_CTOR_DEFAULT

            VertexPositionNormalColor( XMFLOAT3 const& position, XMFLOAT3 const& normal, XMFLOAT4 const& color )
            : position( position ),
            normal( normal ),
            color( color )
        { }

        VertexPositionNormalColor( FXMVECTOR position, FXMVECTOR normal, FXMVECTOR color )
        {
            XMStoreFloat3( &this->position, position );
            XMStoreFloat3( &this->normal, normal );
            XMStoreFloat4( &this->color, color );
        }

        XMFLOAT3 position;
        XMFLOAT3 normal;
        XMFLOAT4 color;

        static const int InputElementCount = 3;
        static const D3D11_INPUT_ELEMENT_DESC InputElements[InputElementCount];
    };


    // Vertex struct holding position, normal vector, and texture mapping information.
    struct VertexPositionNormalTexture
    {
        VertexPositionNormalTexture( ) DIRECTX_CTOR_DEFAULT

            VertexPositionNormalTexture( XMFLOAT3 const& position, XMFLOAT3 const& normal, XMFLOAT2 const& textureCoordinate )
            : position( position ),
            normal( normal ),
            textureCoordinate( textureCoordinate )
        { }

        VertexPositionNormalTexture( FXMVECTOR position, FXMVECTOR normal, FXMVECTOR textureCoordinate )
        {
            XMStoreFloat3( &this->position, position );
            XMStoreFloat3( &this->normal, normal );
            XMStoreFloat2( &this->textureCoordinate, textureCoordinate );
        }

        XMFLOAT3 position;
        XMFLOAT3 normal;
        XMFLOAT2 textureCoordinate;

        static const int InputElementCount = 3;
        static const D3D11_INPUT_ELEMENT_DESC InputElements[InputElementCount];
    };


    // Vertex struct holding position, normal vector, color, and texture mapping information.
    struct VertexPositionNormalColorTexture
    {
        VertexPositionNormalColorTexture( ) DIRECTX_CTOR_DEFAULT

            VertexPositionNormalColorTexture( XMFLOAT3 const& position, XMFLOAT3 const& normal, XMFLOAT4 const& color, XMFLOAT2 const& textureCoordinate )
            : position( position ),
            normal( normal ),
            color( color ),
            textureCoordinate( textureCoordinate )
        { }

        VertexPositionNormalColorTexture( FXMVECTOR position, FXMVECTOR normal, FXMVECTOR color, CXMVECTOR textureCoordinate )
        {
            XMStoreFloat3( &this->position, position );
            XMStoreFloat3( &this->normal, normal );
            XMStoreFloat4( &this->color, color );
            XMStoreFloat2( &this->textureCoordinate, textureCoordinate );
        }

        XMFLOAT3 position;
        XMFLOAT3 normal;
        XMFLOAT4 color;
        XMFLOAT2 textureCoordinate;

        static const int InputElementCount = 4;
        static const D3D11_INPUT_ELEMENT_DESC InputElements[InputElementCount];
    };


    // Vertex struct for Visual Studio Shader Designer (DGSL) holding position, normal,
    // tangent, color (RGBA), and texture mapping information
    struct VertexPositionNormalTangentColorTexture
    {
        VertexPositionNormalTangentColorTexture( ) DIRECTX_CTOR_DEFAULT

            XMFLOAT3 position;
        XMFLOAT3 normal;
        XMFLOAT4 tangent;
        uint32_t color;
        XMFLOAT2 textureCoordinate;

        VertexPositionNormalTangentColorTexture( XMFLOAT3 const& position, XMFLOAT3 const& normal, XMFLOAT4 const& tangent, uint32_t rgba, XMFLOAT2 const& textureCoordinate )
            : position( position ),
            normal( normal ),
            tangent( tangent ),
            color( rgba ),
            textureCoordinate( textureCoordinate )
        {
        }

        VertexPositionNormalTangentColorTexture( FXMVECTOR position, FXMVECTOR normal, FXMVECTOR tangent, uint32_t rgba, CXMVECTOR textureCoordinate )
            : color( rgba )
        {
            XMStoreFloat3( &this->position, position );
            XMStoreFloat3( &this->normal, normal );
            XMStoreFloat4( &this->tangent, tangent );
            XMStoreFloat2( &this->textureCoordinate, textureCoordinate );
        }

        VertexPositionNormalTangentColorTexture( XMFLOAT3 const& position, XMFLOAT3 const& normal, XMFLOAT4 const& tangent, XMFLOAT4 const& color, XMFLOAT2 const& textureCoordinate )
            : position( position ),
            normal( normal ),
            tangent( tangent ),
            textureCoordinate( textureCoordinate )
        {
            SetColor( color );
        }

        VertexPositionNormalTangentColorTexture( FXMVECTOR position, FXMVECTOR normal, FXMVECTOR tangent, CXMVECTOR color, CXMVECTOR textureCoordinate )
        {
            XMStoreFloat3( &this->position, position );
            XMStoreFloat3( &this->normal, normal );
            XMStoreFloat4( &this->tangent, tangent );
            XMStoreFloat2( &this->textureCoordinate, textureCoordinate );

            SetColor( color );
        }

        void __cdecl SetColor( XMFLOAT4 const& icolor ) { SetColor( XMLoadFloat4( &icolor ) ); }
        void XM_CALLCONV SetColor( FXMVECTOR icolor );

        static const int InputElementCount = 5;
        static const D3D11_INPUT_ELEMENT_DESC InputElements[InputElementCount];
    };


    // Vertex struct for Visual Studio Shader Designer (DGSL) holding position, normal,
    // tangent, color (RGBA), texture mapping information, and skinning weights
    struct VertexPositionNormalTangentColorTextureSkinning : public VertexPositionNormalTangentColorTexture
    {
        VertexPositionNormalTangentColorTextureSkinning( ) DIRECTX_CTOR_DEFAULT

            uint32_t indices;
        uint32_t weights;

        VertexPositionNormalTangentColorTextureSkinning( XMFLOAT3 const& position, XMFLOAT3 const& normal, XMFLOAT4 const& tangent, uint32_t rgba,
            XMFLOAT2 const& textureCoordinate, XMUINT4 const& indices, XMFLOAT4 const& weights )
            : VertexPositionNormalTangentColorTexture( position, normal, tangent, rgba, textureCoordinate )
        {
            SetBlendIndices( indices );
            SetBlendWeights( weights );
        }

        VertexPositionNormalTangentColorTextureSkinning( FXMVECTOR position, FXMVECTOR normal, FXMVECTOR tangent, uint32_t rgba, CXMVECTOR textureCoordinate,
            XMUINT4 const& indices, CXMVECTOR weights )
            : VertexPositionNormalTangentColorTexture( position, normal, tangent, rgba, textureCoordinate )
        {
            SetBlendIndices( indices );
            SetBlendWeights( weights );
        }

        VertexPositionNormalTangentColorTextureSkinning( XMFLOAT3 const& position, XMFLOAT3 const& normal, XMFLOAT4 const& tangent, XMFLOAT4 const& color,
            XMFLOAT2 const& textureCoordinate, XMUINT4 const& indices, XMFLOAT4 const& weights )
            : VertexPositionNormalTangentColorTexture( position, normal, tangent, color, textureCoordinate )
        {
            SetBlendIndices( indices );
            SetBlendWeights( weights );
        }

        VertexPositionNormalTangentColorTextureSkinning( FXMVECTOR position, FXMVECTOR normal, FXMVECTOR tangent, CXMVECTOR color, CXMVECTOR textureCoordinate,
            XMUINT4 const& indices, CXMVECTOR weights )
            : VertexPositionNormalTangentColorTexture( position, normal, tangent, color, textureCoordinate )
        {
            SetBlendIndices( indices );
            SetBlendWeights( weights );
        }

        void __cdecl SetBlendIndices( XMUINT4 const& iindices );

        void __cdecl SetBlendWeights( XMFLOAT4 const& iweights ) { SetBlendWeights( XMLoadFloat4( &iweights ) ); }
        void XM_CALLCONV SetBlendWeights( FXMVECTOR iweights );

        static const int InputElementCount = 7;
        static const D3D11_INPUT_ELEMENT_DESC InputElements[InputElementCount];
    };
}

//--------------------------------------------------------------------------------------
// SDKMESH format is generated by the legacy DirectX SDK's Content Exporter and
// originally rendered by the DXUT helper class SDKMesh
//
// http://go.microsoft.com/fwlink/?LinkId=226208
//--------------------------------------------------------------------------------------
namespace DXUT
{
    // Declarations for _Type fields
    //
    typedef enum _D3DDECLTYPE
    {
        D3DDECLTYPE_FLOAT1 = 0,  // 1D float expanded to (value, 0., 0., 1.)
        D3DDECLTYPE_FLOAT2 = 1,  // 2D float expanded to (value, value, 0., 1.)
        D3DDECLTYPE_FLOAT3 = 2,  // 3D float expanded to (value, value, value, 1.)
        D3DDECLTYPE_FLOAT4 = 3,  // 4D float
        D3DDECLTYPE_D3DCOLOR = 4,  // 4D packed unsigned bytes mapped to 0. to 1. range
                                     // Input is in D3DCOLOR format (ARGB) expanded to (R, G, B, A)
        D3DDECLTYPE_UBYTE4 = 5,  // 4D unsigned byte
        D3DDECLTYPE_SHORT2 = 6,  // 2D signed short expanded to (value, value, 0., 1.)
        D3DDECLTYPE_SHORT4 = 7,  // 4D signed short

    // The following types are valid only with vertex shaders >= 2.0


        D3DDECLTYPE_UBYTE4N = 8,  // Each of 4 bytes is normalized by dividing to 255.0
        D3DDECLTYPE_SHORT2N = 9,  // 2D signed short normalized (v[0]/32767.0,v[1]/32767.0,0,1)
        D3DDECLTYPE_SHORT4N = 10,  // 4D signed short normalized (v[0]/32767.0,v[1]/32767.0,v[2]/32767.0,v[3]/32767.0)
        D3DDECLTYPE_USHORT2N = 11,  // 2D unsigned short normalized (v[0]/65535.0,v[1]/65535.0,0,1)
        D3DDECLTYPE_USHORT4N = 12,  // 4D unsigned short normalized (v[0]/65535.0,v[1]/65535.0,v[2]/65535.0,v[3]/65535.0)
        D3DDECLTYPE_UDEC3 = 13,  // 3D unsigned 10 10 10 format expanded to (value, value, value, 1)
        D3DDECLTYPE_DEC3N = 14,  // 3D signed 10 10 10 format normalized and expanded to (v[0]/511.0, v[1]/511.0, v[2]/511.0, 1)
        D3DDECLTYPE_FLOAT16_2 = 15,  // Two 16-bit floating point values, expanded to (value, value, 0, 1)
        D3DDECLTYPE_FLOAT16_4 = 16,  // Four 16-bit floating point values
        D3DDECLTYPE_UNUSED = 17,  // When the type field in a decl is unused.
    } D3DDECLTYPE;

#define MAXD3DDECLTYPE      D3DDECLTYPE_UNUSED

    // Vertex element semantics
    //
    typedef enum _D3DDECLUSAGE
    {
        D3DDECLUSAGE_POSITION = 0,
        D3DDECLUSAGE_BLENDWEIGHT,   // 1
        D3DDECLUSAGE_BLENDINDICES,  // 2
        D3DDECLUSAGE_NORMAL,        // 3
        D3DDECLUSAGE_PSIZE,         // 4
        D3DDECLUSAGE_TEXCOORD,      // 5
        D3DDECLUSAGE_TANGENT,       // 6
        D3DDECLUSAGE_BINORMAL,      // 7
        D3DDECLUSAGE_TESSFACTOR,    // 8
        D3DDECLUSAGE_POSITIONT,     // 9
        D3DDECLUSAGE_COLOR,         // 10
        D3DDECLUSAGE_FOG,           // 11
        D3DDECLUSAGE_DEPTH,         // 12
        D3DDECLUSAGE_SAMPLE,        // 13
    } D3DDECLUSAGE;

    // .SDKMESH files

    // SDKMESH_HEADER
    // SDKMESH_VERTEX_BUFFER_HEADER header->VertexStreamHeadersOffset
    // SDKMESH_INDEX_BUFFER_HEADER  header->IndexStreamHeadersOffset
    // SDKMESH_MESH                 header->MeshDataOffset
    // SDKMESH_SUBSET               header->SubsetDataOffset
    // SDKMESH_FRAME                header->FrameDataOffset
    // SDKMESH_MATERIAL             header->MaterialDataOffset
    // [header->NonBufferDataSize]
    // { [ header->NumVertexBuffers]
    //      VB data
    // }
    // { [ header->NumIndexBuffers]
    //      IB data
    // }


    // .SDDKANIM files

    // SDKANIMATION_FILE_HEADER
    // BYTE[] - Length of fileheader->AnimationDataSize

    // .SDKMESH uses Direct3D 9 decls, but only a subset of these is ever generated by the legacy DirectX SDK Content Exporter

    // D3DDECLUSAGE_POSITION / D3DDECLTYPE_FLOAT3
    // (D3DDECLUSAGE_BLENDWEIGHT / D3DDECLTYPE_UBYTE4N
    // D3DDECLUSAGE_BLENDINDICES / D3DDECLTYPE_UBYTE4)?
    // (D3DDECLUSAGE_NORMAL / D3DDECLTYPE_FLOAT3, D3DDECLTYPE_FLOAT16_4, D3DDECLTYPE_SHORT4N, D3DDECLTYPE_UBYTE4N, or D3DDECLTYPE_DEC3N [not supported])?
    // (D3DDECLUSAGE_COLOR / D3DDECLTYPE_D3DCOLOR)?
    // (D3DDECLUSAGE_TEXCOORD / D3DDECLTYPE_FLOAT1, D3DDECLTYPE_FLOAT2 or D3DDECLTYPE_FLOAT16_2, D3DDECLTYPE_FLOAT3 or D3DDECLTYPE_FLOAT16_4, D3DDECLTYPE_FLOAT4 or D3DDECLTYPE_FLOAT16_4)*
    // (D3DDECLUSAGE_TANGENT / same as D3DDECLUSAGE_NORMAL)?
    // (D3DDECLUSAGE_BINORMAL / same as D3DDECLUSAGE_NORMAL)?
    /*
    enum D3DDECLUSAGE
    {
        D3DDECLUSAGE_POSITION = 0,
        D3DDECLUSAGE_BLENDWEIGHT =1,
        D3DDECLUSAGE_BLENDINDICES =2,
        D3DDECLUSAGE_NORMAL =3,
        D3DDECLUSAGE_TEXCOORD = 5,
        D3DDECLUSAGE_TANGENT = 6,
        D3DDECLUSAGE_BINORMAL = 7,
        D3DDECLUSAGE_COLOR = 10,
    };

    enum D3DDECLTYPE
    {
        D3DDECLTYPE_FLOAT1    =  0,  // 1D float expanded to (value, 0., 0., 1.)
        D3DDECLTYPE_FLOAT2    =  1,  // 2D float expanded to (value, value, 0., 1.)
        D3DDECLTYPE_FLOAT3    =  2,  // 3D float expanded to (value, value, value, 1.)
        D3DDECLTYPE_FLOAT4    =  3,  // 4D float
        D3DDECLTYPE_D3DCOLOR  =  4,  // 4D packed unsigned bytes mapped to 0. to 1. range
                                     // Input is in D3DCOLOR format (ARGB) expanded to (R, G, B, A)
        D3DDECLTYPE_UBYTE4    =  5,  // 4D unsigned byte
        D3DDECLTYPE_UBYTE4N   =  8,  // Each of 4 bytes is normalized by dividing to 255.0
        D3DDECLTYPE_SHORT4N   = 10,  // 4D signed short normalized (v[0]/32767.0,v[1]/32767.0,v[2]/32767.0,v[3]/32767.0)
        // Note: There is no equivalent to D3DDECLTYPE_DEC3N (14) as a DXGI_FORMAT
        D3DDECLTYPE_FLOAT16_2 = 15,  // Two 16-bit floating point values, expanded to (value, value, 0, 1)
        D3DDECLTYPE_FLOAT16_4 = 16,  // Four 16-bit floating point values

        D3DDECLTYPE_UNUSED    = 17,  // When the type field in a decl is unused.
    };*/

    #pragma pack(push,4)

    struct D3DVERTEXELEMENT9
    {
        WORD    Stream;     // Stream index
        WORD    Offset;     // Offset in the stream in bytes
        BYTE    Type;       // Data type
        BYTE    Method;     // Processing method
        BYTE    Usage;      // Semantics
        BYTE    UsageIndex; // Semantic index
    };

    #pragma pack(pop)

    //--------------------------------------------------------------------------------------
    // Hard Defines for the various structures
    //--------------------------------------------------------------------------------------
    const uint32_t SDKMESH_FILE_VERSION = 101;
    const uint32_t MAX_VERTEX_ELEMENTS = 32;
    const uint32_t MAX_VERTEX_STREAMS = 16;
    const uint32_t MAX_FRAME_NAME = 100;
    const uint32_t MAX_MESH_NAME = 100;
    const uint32_t MAX_SUBSET_NAME = 100;
    const uint32_t MAX_MATERIAL_NAME = 100;
    const uint32_t MAX_TEXTURE_NAME = MAX_PATH;
    const uint32_t MAX_MATERIAL_PATH = MAX_PATH;
    const uint32_t INVALID_FRAME = uint32_t(-1);
    const uint32_t INVALID_MESH =  uint32_t(-1);
    const uint32_t INVALID_MATERIAL = uint32_t(-1);
    const uint32_t INVALID_SUBSET = uint32_t(-1);
    const uint32_t INVALID_ANIMATION_DATA = uint32_t(-1);
    const uint32_t INVALID_SAMPLER_SLOT = uint32_t(-1);
    const uint32_t ERROR_RESOURCE_VALUE = 1;

    template<typename TYPE> bool IsErrorResource( TYPE data )
    {
        if( ( TYPE )ERROR_RESOURCE_VALUE == data )
            return true;
        return false;
    }

    //--------------------------------------------------------------------------------------
    // Enumerated Types.  These will have mirrors in both D3D9 and D3D11
    //--------------------------------------------------------------------------------------
    enum SDKMESH_PRIMITIVE_TYPE
    {
        PT_TRIANGLE_LIST = 0,
        PT_TRIANGLE_STRIP,
        PT_LINE_LIST,
        PT_LINE_STRIP,
        PT_POINT_LIST,
        PT_TRIANGLE_LIST_ADJ,
        PT_TRIANGLE_STRIP_ADJ,
        PT_LINE_LIST_ADJ,
        PT_LINE_STRIP_ADJ,
        PT_QUAD_PATCH_LIST,
        PT_TRIANGLE_PATCH_LIST,
    };

    enum SDKMESH_INDEX_TYPE
    {
        IT_16BIT = 0,
        IT_32BIT,
    };

    enum FRAME_TRANSFORM_TYPE
    {
        FTT_RELATIVE = 0,
        FTT_ABSOLUTE,		//This is not currently used but is here to support absolute transformations in the future
    };

    //--------------------------------------------------------------------------------------
    // Structures.
    //--------------------------------------------------------------------------------------
    #pragma pack(push,8)

    struct SDKMESH_HEADER
    {
        //Basic Info and sizes
        UINT Version;
        BYTE IsBigEndian;
        UINT64 HeaderSize;
        UINT64 NonBufferDataSize;
        UINT64 BufferDataSize;

        //Stats
        UINT NumVertexBuffers;
        UINT NumIndexBuffers;
        UINT NumMeshes;
        UINT NumTotalSubsets;
        UINT NumFrames;
        UINT NumMaterials;

        //Offsets to Data
        UINT64 VertexStreamHeadersOffset;
        UINT64 IndexStreamHeadersOffset;
        UINT64 MeshDataOffset;
        UINT64 SubsetDataOffset;
        UINT64 FrameDataOffset;
        UINT64 MaterialDataOffset;
    };

    struct SDKMESH_VERTEX_BUFFER_HEADER
    {
        UINT64 NumVertices;
        UINT64 SizeBytes;
        UINT64 StrideBytes;
        D3DVERTEXELEMENT9 Decl[MAX_VERTEX_ELEMENTS];
        union
        {
            UINT64 DataOffset;
            ID3D11Buffer* pVB11;
        };
    };

    struct SDKMESH_INDEX_BUFFER_HEADER
    {
        UINT64 NumIndices;
        UINT64 SizeBytes;
        UINT IndexType;
        union
        {
            UINT64 DataOffset;
            ID3D11Buffer* pIB11;
        };
    };

    struct SDKMESH_MESH
    {
        char Name[MAX_MESH_NAME];
        BYTE NumVertexBuffers;
        UINT VertexBuffers[MAX_VERTEX_STREAMS];
        UINT IndexBuffer;
        UINT NumSubsets;
        UINT NumFrameInfluences; //aka bones

        DirectX::XMFLOAT3 BoundingBoxCenter;
        DirectX::XMFLOAT3 BoundingBoxExtents;

        union
        {
            UINT64 SubsetOffset;
            INT* pSubsets;
        };
        union
        {
            UINT64 FrameInfluenceOffset;
            UINT* pFrameInfluences;
        };
    };

    struct SDKMESH_SUBSET
    {
        char Name[MAX_SUBSET_NAME];
        UINT MaterialID;
        UINT PrimitiveType;
        UINT64 IndexStart;
        UINT64 IndexCount;
        UINT64 VertexStart;
        UINT64 VertexCount;
    };

    struct SDKMESH_FRAME
    {
        char Name[MAX_FRAME_NAME];
        UINT Mesh;
        UINT ParentFrame;
        UINT ChildFrame;
        UINT SiblingFrame;
        DirectX::XMFLOAT4X4 Matrix;
        UINT AnimationDataIndex;		//Used to index which set of keyframes transforms this frame
    };

    struct SDKMESH_MATERIAL
    {
        char    Name[MAX_MATERIAL_NAME];

        // Use MaterialInstancePath
        char    MaterialInstancePath[MAX_MATERIAL_PATH];

        // Or fall back to d3d8-type materials
        char    DiffuseTexture[MAX_TEXTURE_NAME];
        char    NormalTexture[MAX_TEXTURE_NAME];
        char    SpecularTexture[MAX_TEXTURE_NAME];

        DirectX::XMFLOAT4 Diffuse;
        DirectX::XMFLOAT4 Ambient;
        DirectX::XMFLOAT4 Specular;
        DirectX::XMFLOAT4 Emissive;
        FLOAT Power;

        union
        {
            UINT64 Force64_1;			//Force the union to 64bits
            ID3D11Texture2D* pDiffuseTexture11;
        };
        union
        {
            UINT64 Force64_2;			//Force the union to 64bits
            ID3D11Texture2D* pNormalTexture11;
        };
        union
        {
            UINT64 Force64_3;			//Force the union to 64bits
            ID3D11Texture2D* pSpecularTexture11;
        };

        union
        {
            UINT64 Force64_4;			//Force the union to 64bits
            ID3D11ShaderResourceView* pDiffuseRV11;
        };
        union
        {
            UINT64 Force64_5;		    //Force the union to 64bits
            ID3D11ShaderResourceView* pNormalRV11;
        };
        union
        {
            UINT64 Force64_6;			//Force the union to 64bits
            ID3D11ShaderResourceView* pSpecularRV11;
        };
    };

    struct SDKANIMATION_FILE_HEADER
    {
        UINT Version;
        BYTE IsBigEndian;
        UINT FrameTransformType;
        UINT NumFrames;
        UINT NumAnimationKeys;
        UINT AnimationFPS;
        UINT64 AnimationDataSize;
        UINT64 AnimationDataOffset;
    };

    struct SDKANIMATION_DATA
    {
        DirectX::XMFLOAT3 Translation;
        DirectX::XMFLOAT4 Orientation;
        DirectX::XMFLOAT3 Scaling;
    };

    struct SDKANIMATION_FRAME_DATA
    {
        char FrameName[MAX_FRAME_NAME];
        union
        {
            UINT64 DataOffset;
            SDKANIMATION_DATA* pAnimationData;
        };
    };

    #pragma pack(pop)

}; // namespace

static_assert( sizeof(DXUT::D3DVERTEXELEMENT9) == 8, "Direct3D9 Decl structure size incorrect" );
static_assert( sizeof(DXUT::SDKMESH_HEADER)== 104, "SDK Mesh structure size incorrect" );
static_assert( sizeof(DXUT::SDKMESH_VERTEX_BUFFER_HEADER) == 288, "SDK Mesh structure size incorrect" );
static_assert( sizeof(DXUT::SDKMESH_INDEX_BUFFER_HEADER) == 32, "SDK Mesh structure size incorrect" );
static_assert( sizeof(DXUT::SDKMESH_MESH) == 224, "SDK Mesh structure size incorrect" );
static_assert( sizeof(DXUT::SDKMESH_SUBSET) == 144, "SDK Mesh structure size incorrect" );
static_assert( sizeof(DXUT::SDKMESH_FRAME) == 184, "SDK Mesh structure size incorrect" );
static_assert( sizeof(DXUT::SDKMESH_MATERIAL) == 1256, "SDK Mesh structure size incorrect" );
static_assert( sizeof(DXUT::SDKANIMATION_FILE_HEADER) == 40, "SDK Mesh structure size incorrect" );
static_assert( sizeof(DXUT::SDKANIMATION_DATA) == 40, "SDK Mesh structure size incorrect" );
static_assert( sizeof(DXUT::SDKANIMATION_FRAME_DATA) == 112, "SDK Mesh structure size incorrect" );


//--------------------------------------------------------------------------------------
struct MaterialRecordSDKMESH
{
//    std::shared_ptr<IEffect> effect;
    bool hasEffect;
    bool alpha;
};

using namespace VertexAsylum;
using namespace DirectX;

//--------------------------------------------------------------------------------------
// Direct3D 9 Vertex Declaration to DirectInput 11 Input Layout mapping

static void GetInputLayoutDesc( _In_reads_( 32 ) const DXUT::D3DVERTEXELEMENT9 decl[], std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc,
    bool &perVertexColor, bool& enableSkinning, bool& dualTexture )
{
    static const D3D11_INPUT_ELEMENT_DESC elements[] =
    {
        { "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",       0, DXGI_FORMAT_B8G8R8A8_UNORM,     0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BINORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES",0, DXGI_FORMAT_R8G8B8A8_UINT,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT", 0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    using namespace DXUT;

    uint32_t offset = 0;
    uint32_t texcoords = 0;

    bool posfound = false;

    for( uint32_t index = 0; index < DXUT::MAX_VERTEX_ELEMENTS; ++index )
    {
        if( decl[index].Usage == 0xFF )
            break;

        if( decl[index].Type == D3DDECLTYPE_UNUSED )
            break;

        if( decl[index].Offset != offset )
            break;

        if( decl[index].Usage == D3DDECLUSAGE_POSITION && decl[index].Type == D3DDECLTYPE_FLOAT3 )
        {
            inputDesc.push_back( elements[0] );
            offset += 12;
            posfound = true;
        }
        else if( decl[index].Usage == D3DDECLUSAGE_NORMAL )
        {
            if( decl[index].Type == D3DDECLTYPE_FLOAT3 )
            {
                inputDesc.push_back( elements[1] );
                offset += 12;
            }
            else if( decl[index].Type == D3DDECLTYPE_FLOAT16_4 )
            {
                D3D11_INPUT_ELEMENT_DESC desc = elements[1];
                desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
                inputDesc.push_back( desc );
                offset += 8;
            }
            else if( decl[index].Type == D3DDECLTYPE_SHORT4N )
            {
                D3D11_INPUT_ELEMENT_DESC desc = elements[1];
                desc.Format = DXGI_FORMAT_R16G16B16A16_SNORM;
                inputDesc.push_back( desc );
                offset += 8;
            }
            else if( decl[index].Type == D3DDECLTYPE_UBYTE4N )
            {
                D3D11_INPUT_ELEMENT_DESC desc = elements[1];
                desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                inputDesc.push_back( desc );
                offset += 4;
            }
            else
                break;
        }
        else if( decl[index].Usage == D3DDECLUSAGE_COLOR && decl[index].Type == D3DDECLTYPE_D3DCOLOR )
        {
            inputDesc.push_back( elements[2] );
            offset += 4;
            perVertexColor = true;
        }
        else if( decl[index].Usage == D3DDECLUSAGE_TANGENT )
        {
            if( decl[index].Type == D3DDECLTYPE_FLOAT3 )
            {
                inputDesc.push_back( elements[3] );
                offset += 12;
            }
            else if( decl[index].Type == D3DDECLTYPE_FLOAT16_4 )
            {
                D3D11_INPUT_ELEMENT_DESC desc = elements[3];
                desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
                inputDesc.push_back( desc );
                offset += 8;
            }
            else if( decl[index].Type == D3DDECLTYPE_SHORT4N )
            {
                D3D11_INPUT_ELEMENT_DESC desc = elements[3];
                desc.Format = DXGI_FORMAT_R16G16B16A16_SNORM;
                inputDesc.push_back( desc );
                offset += 8;
            }
            else if( decl[index].Type == D3DDECLTYPE_UBYTE4N )
            {
                D3D11_INPUT_ELEMENT_DESC desc = elements[3];
                desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                inputDesc.push_back( desc );
                offset += 4;
            }
            else
                break;
        }
        else if( decl[index].Usage == D3DDECLUSAGE_BINORMAL )
        {
            if( decl[index].Type == D3DDECLTYPE_FLOAT3 )
            {
                inputDesc.push_back( elements[4] );
                offset += 12;
            }
            else if( decl[index].Type == D3DDECLTYPE_FLOAT16_4 )
            {
                D3D11_INPUT_ELEMENT_DESC desc = elements[4];
                desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
                inputDesc.push_back( desc );
                offset += 8;
            }
            else if( decl[index].Type == D3DDECLTYPE_SHORT4N )
            {
                D3D11_INPUT_ELEMENT_DESC desc = elements[4];
                desc.Format = DXGI_FORMAT_R16G16B16A16_SNORM;
                inputDesc.push_back( desc );
                offset += 8;
            }
            else if( decl[index].Type == D3DDECLTYPE_UBYTE4N )
            {
                D3D11_INPUT_ELEMENT_DESC desc = elements[4];
                desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                inputDesc.push_back( desc );
                offset += 4;
            }
            else
                break;
        }
        else if( decl[index].Usage == D3DDECLUSAGE_TEXCOORD )
        {
            D3D11_INPUT_ELEMENT_DESC desc = elements[5];
            desc.SemanticIndex = decl[index].UsageIndex;

            bool unk = false;
            switch( decl[index].Type )
            {
            case D3DDECLTYPE_FLOAT2:    offset += 8; break;
            case D3DDECLTYPE_FLOAT1:    desc.Format = DXGI_FORMAT_R32_FLOAT; offset += 4; break;
            case D3DDECLTYPE_FLOAT3:    desc.Format = DXGI_FORMAT_R32G32B32_FLOAT; offset += 12; break;
            case D3DDECLTYPE_FLOAT4:    desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; offset += 16; break;
            case D3DDECLTYPE_FLOAT16_2: desc.Format = DXGI_FORMAT_R16G16_FLOAT; offset += 4; break;
            case D3DDECLTYPE_FLOAT16_4: desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; offset += 8; break;

            default:
                unk = true;
                break;
            }

            if( unk )
                break;

            ++texcoords;

            inputDesc.push_back( desc );
        }
        else if( decl[index].Usage == D3DDECLUSAGE_BLENDINDICES && decl[index].Type == D3DDECLTYPE_UBYTE4 )
        {
            enableSkinning = true;
            inputDesc.push_back( elements[6] );
            offset += 4;
        }
        else if( decl[index].Usage == D3DDECLUSAGE_BLENDWEIGHT && decl[index].Type == D3DDECLTYPE_UBYTE4N )
        {
            enableSkinning = true;
            inputDesc.push_back( elements[7] );
            offset += 4;
        }
        else
            break;
    }

    if( !posfound )
        throw std::exception( "SV_Position is required" );

    if( texcoords == 2 )
    {
        dualTexture = true;
    }
}

int GetFormatElementSize( DXGI_FORMAT format )
{
    switch( format )
    {
    case( DXGI_FORMAT_R32G32B32A32_FLOAT ) :    return 4 * 4; break;
    case( DXGI_FORMAT_R32G32B32_FLOAT ) :       return 3 * 4; break;
    case( DXGI_FORMAT_R32G32_FLOAT ) :          return 2 * 4; break;
    case( DXGI_FORMAT_R32_FLOAT ) :             return 1 * 4; break;
    case( DXGI_FORMAT_B8G8R8A8_UNORM ) :        return 1 * 4; break;
    case( DXGI_FORMAT_R8G8B8A8_UINT ) :         return 1 * 4; break;
    case( DXGI_FORMAT_R8G8B8A8_UNORM ) :        return 1 * 4; break;
    default:
        throw std::exception( "GetFormatElementSize format not recognized" );
        break;
    }
}

static shared_ptr<vaAssetTexture> FindOrLoadTexture( const wstring & textureSearchPath, const string & _name, vector<shared_ptr<vaAssetTexture>> & loadedTextures, vector<string> & loadedTextureNames, vaAssetImporter::LoadingParameters & parameters, vaAssetImporter::LoadedContent * outContent, bool assumeSourceIsInSRGB, bool dontAutogenerateMIPs )
{
    string name = vaStringTools::ToLower( _name );

    for( int i = 0; i < loadedTextureNames.size(); i++ )
        if( name == loadedTextureNames[i] )
            return loadedTextures[i];

    wstring filePath = textureSearchPath + vaStringTools::SimpleWiden(name);

    if( !vaFileTools::FileExists( filePath ) )
    {
        VA_LOG( "vaAssetImporter_SDKMESH - Unable to find texture '%s'", filePath.c_str() );
        return nullptr;
    }

    shared_ptr<vaTexture> textureOut = shared_ptr<vaTexture>( vaTexture::Import( filePath, assumeSourceIsInSRGB, dontAutogenerateMIPs ) );

    if( textureOut == nullptr )
    {
        VA_LOG( "vaAssetImporter_SDKMESH - Error while loading '%s'", filePath.c_str( ) );
        return nullptr;
    }

    shared_ptr<vaAssetTexture> textureAssetOut = parameters.AssetPack.Add( textureOut, parameters.AssetPack.FindSuitableAssetName( name ) );

    loadedTextures.push_back( textureAssetOut );
    loadedTextureNames.push_back( name );

    if( outContent != nullptr )
        outContent->LoadedAssets.push_back( textureAssetOut );

    return textureAssetOut;
}

static vector<shared_ptr<vaAssetRenderMaterial>> SDKMESH_LoadMaterials( const wstring & textureSearchPath, const DXUT::SDKMESH_MATERIAL* pMaterials, UINT numMaterials, vaAssetImporter::LoadingParameters & parameters, vaAssetImporter::LoadedContent * outContent )
{
    vector<shared_ptr<vaAssetRenderMaterial>>   loadedMaterials;

    vector<shared_ptr<vaAssetTexture>>          loadedTextures;
    vector<string>                              loadedTextureNames;

    for( UINT m = 0; m < numMaterials; m++ )
    {

        shared_ptr<vaRenderMaterial> material = vaRenderMaterialManager::GetInstance().CreateRenderMaterial();

        // load textures
        if( pMaterials[m].DiffuseTexture[0] != 0 )
        {
            shared_ptr<vaAssetTexture> loadedTextureAsset = FindOrLoadTexture( textureSearchPath, pMaterials[m].DiffuseTexture, loadedTextures, loadedTextureNames, parameters, outContent, true, false );
            if( loadedTextureAsset != nullptr )
                material->SetTextureAlbedo( loadedTextureAsset->Resource );
        }
        if( pMaterials[m].NormalTexture[0] != 0 )
        {
            shared_ptr<vaAssetTexture> loadedTextureAsset = FindOrLoadTexture( textureSearchPath, pMaterials[m].NormalTexture, loadedTextures, loadedTextureNames, parameters, outContent, false, false );
            if( loadedTextureAsset != nullptr )
                material->SetTextureNormalmap( loadedTextureAsset->Resource );
        }
        if( pMaterials[m].SpecularTexture[0] != 0 )
        {
            shared_ptr<vaAssetTexture> loadedTextureAsset = FindOrLoadTexture( textureSearchPath, pMaterials[m].SpecularTexture, loadedTextures, loadedTextureNames, parameters, outContent, false, false );
            if( loadedTextureAsset != nullptr )
                material->SetTextureSpecular( loadedTextureAsset->Resource );
        }

        vaRenderMaterial::MaterialSettings settings;

        // material->Settings().ColorMultAlbedo    = vaVector4( pMaterials[m].Diffuse.x, pMaterials[m].Diffuse.y, pMaterials[m].Diffuse.z, pMaterials[m].Diffuse.w );
        // material->Settings().ColorMultSpecular  = vaVector4( pMaterials[m].Specular.x, pMaterials[m].Specular.y, pMaterials[m].Specular.z, pMaterials[m].Specular.w );
        settings.AlphaTest          = false;
        settings.FaceCull           = vaFaceCull::Back;

        auto materialAsset = parameters.AssetPack.Add( material, parameters.AssetPack.FindSuitableAssetName( string(pMaterials[m].Name ) ) );

        loadedMaterials.push_back( materialAsset );

        if( outContent != nullptr )
            outContent->LoadedAssets.push_back( materialAsset );
    }

    return loadedMaterials;
}


bool LoadFileContents_SDKMESH( const std::shared_ptr<vaMemoryStream> & fileContents, const wstring & textureSearchPath, const wstring & name, vaAssetImporter::LoadingParameters & parameters, vaAssetImporter::LoadedContent * outContent )
{
    uint8_t* meshData = (uint8_t*)malloc( fileContents->GetLength() );
    size_t dataSize         = fileContents->GetLength();
    memcpy( meshData, fileContents->GetBuffer( ), dataSize );
    bool ccw                = false;
    bool pmalpha            = false;

    // File Headers
    if( dataSize < sizeof( DXUT::SDKMESH_HEADER ) )
        throw std::exception( "End of file" );

    // File Headers
    if( dataSize < sizeof( DXUT::SDKMESH_HEADER ) )
        throw std::exception( "End of file" );
    auto header = reinterpret_cast<const DXUT::SDKMESH_HEADER*>( meshData );

    size_t headerSize = sizeof( DXUT::SDKMESH_HEADER )
        + header->NumVertexBuffers * sizeof( DXUT::SDKMESH_VERTEX_BUFFER_HEADER )
        + header->NumIndexBuffers * sizeof( DXUT::SDKMESH_INDEX_BUFFER_HEADER );
    if( header->HeaderSize != headerSize )
        throw std::exception( "Not a valid SDKMESH file" );

    if( dataSize < header->HeaderSize )
        throw std::exception( "End of file" );

    if( header->Version != DXUT::SDKMESH_FILE_VERSION )
        throw std::exception( "Not a supported SDKMESH version" );

    if( header->IsBigEndian )
        throw std::exception( "Loading BigEndian SDKMESH files not supported" );

    if( !header->NumMeshes )
        throw std::exception( "No meshes found" );

    if( !header->NumVertexBuffers )
        throw std::exception( "No vertex buffers found" );

//    if( header->NumVertexBuffers > 1 )
//        throw std::exception( "vaAssetImporter_SDKMESH supports only 1 vertex buffer" );

    if( !header->NumIndexBuffers )
        throw std::exception( "No index buffers found" );

//    if( header->NumIndexBuffers > 1 )
//        throw std::exception( "vaAssetImporter_SDKMESH supports only 1 index buffer" );

    if( !header->NumTotalSubsets )
        throw std::exception( "No subsets found" );

//    if( header->NumTotalSubsets > 1 )
//        throw std::exception( "vaAssetImporter_SDKMESH supports only 1 subset" );

    if( !header->NumMaterials )
        throw std::exception( "No materials found" );

//    if( header->NumMaterials > 1 )
//        throw std::exception( "vaAssetImporter_SDKMESH supports only 1 material" );

    // Sub-headers
    if( dataSize < header->VertexStreamHeadersOffset
        || ( dataSize < ( header->VertexStreamHeadersOffset + header->NumVertexBuffers * sizeof( DXUT::SDKMESH_VERTEX_BUFFER_HEADER ) ) ) )
        throw std::exception( "End of file" );
    auto vbArray = reinterpret_cast<const DXUT::SDKMESH_VERTEX_BUFFER_HEADER*>( meshData + header->VertexStreamHeadersOffset );

    if( dataSize < header->IndexStreamHeadersOffset
        || ( dataSize < ( header->IndexStreamHeadersOffset + header->NumIndexBuffers * sizeof( DXUT::SDKMESH_INDEX_BUFFER_HEADER ) ) ) )
        throw std::exception( "End of file" );
    auto ibArray = reinterpret_cast<const DXUT::SDKMESH_INDEX_BUFFER_HEADER*>( meshData + header->IndexStreamHeadersOffset );

    if( dataSize < header->MeshDataOffset
        || ( dataSize < ( header->MeshDataOffset + header->NumMeshes * sizeof( DXUT::SDKMESH_MESH ) ) ) )
        throw std::exception( "End of file" );
    auto meshArray = reinterpret_cast<DXUT::SDKMESH_MESH*>( meshData + header->MeshDataOffset );

    if( dataSize < header->SubsetDataOffset
        || ( dataSize < ( header->SubsetDataOffset + header->NumTotalSubsets * sizeof( DXUT::SDKMESH_SUBSET ) ) ) )
        throw std::exception( "End of file" );
    auto subsetArray = reinterpret_cast<const DXUT::SDKMESH_SUBSET*>( meshData + header->SubsetDataOffset );

    if( dataSize < header->FrameDataOffset
        || ( dataSize < ( header->FrameDataOffset + header->NumFrames * sizeof( DXUT::SDKMESH_FRAME ) ) ) )
        throw std::exception( "End of file" );
    // TODO - auto frameArray = reinterpret_cast<const DXUT::SDKMESH_FRAME*>( meshData + header->FrameDataOffset );

    if( dataSize < header->MaterialDataOffset
        || ( dataSize < ( header->MaterialDataOffset + header->NumMaterials * sizeof( DXUT::SDKMESH_MATERIAL ) ) ) )
        throw std::exception( "End of file" );
    auto materialArray = reinterpret_cast<const DXUT::SDKMESH_MATERIAL*>( meshData + header->MaterialDataOffset );

    vector<shared_ptr<vaAssetRenderMaterial>> loadedMaterials = SDKMESH_LoadMaterials( textureSearchPath, materialArray, header->NumMaterials, parameters, outContent );

    // Setup subsets
    for( UINT i = 0; i < header->NumMeshes; i++ )
    {
        meshArray[i].pSubsets = (INT*)( meshData + meshArray[i].SubsetOffset );
        meshArray[i].pFrameInfluences = (UINT*)( meshData + meshArray[i].FrameInfluenceOffset );
    }

    // Buffer data
    uint64_t bufferDataOffset = header->HeaderSize + header->NonBufferDataSize;
    if( ( dataSize < bufferDataOffset )
        || ( dataSize < bufferDataOffset + header->BufferDataSize ) )
        throw std::exception( "End of file" );
    const uint8_t* bufferData = meshData + bufferDataOffset;

//    // Create vertex buffers
//    std::vector<ComPtr<ID3D11Buffer>> vbs;
//    vbs.resize( header->NumVertexBuffers );

    std::vector<std::shared_ptr<std::vector<D3D11_INPUT_ELEMENT_DESC>>> vbDecls;
    vbDecls.resize( header->NumVertexBuffers );

    std::vector<bool> perVertexColor;
    perVertexColor.resize( header->NumVertexBuffers );

    std::vector<bool> enableSkinning;
    enableSkinning.resize( header->NumVertexBuffers );

    std::vector<bool> enableDualTexture;
    enableDualTexture.resize( header->NumVertexBuffers );

    DXUT::SDKMESH_MESH* currentMesh = &meshArray[0];
    int tris = 0;
    for( UINT meshi = 0; meshi < header->NumMeshes; ++meshi ) 
    {
        DXUT::SDKMESH_MESH* currentMesh = &meshArray[meshi];

        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        // PROCESS VERTICES
        assert( currentMesh->NumVertexBuffers == 1 );
        auto& vheader = vbArray[currentMesh->VertexBuffers[0]];
        auto& iheader = ibArray[currentMesh->IndexBuffer];

        if( dataSize < vheader.DataOffset
            || ( dataSize < vheader.DataOffset + vheader.SizeBytes ) )
            throw std::exception( "End of file" );

        auto vbDecl = std::vector<D3D11_INPUT_ELEMENT_DESC>( );
        bool vertColor = false;
        bool skinning = false;
        bool dualTexture = false;
        GetInputLayoutDesc( vheader.Decl, vbDecl, vertColor, skinning, dualTexture );

        bool hasNormals = false;
        bool hasTangents = false;
        bool hasTexcoords0 = false;
        bool hasTexcoords1 = false;

        vector<vaVector3>   vertices;
        vector<uint32>      colors;
        vector<vaVector3>   normals;
        vector<vaVector4>   tangents;  // .w holds handedness
        vector<vaVector2>   texcoords0;
        vector<vaVector2>   texcoords1;

        vertices.resize( vheader.NumVertices );
        colors.resize( vertices.size( ) );
        normals.resize( vertices.size( ) );
        tangents.resize( vertices.size( ) );
        texcoords0.resize( vertices.size( ) );
        texcoords1.resize( vertices.size( ) );

        auto verts = reinterpret_cast<const uint8_t*>( bufferData + ( vheader.DataOffset - bufferDataOffset ) );

        for( size_t i = 0; i < vertices.size( ); i++ )
        {
            assert( ( i + 1 ) * vheader.StrideBytes <= vheader.SizeBytes );
            const uint8_t * vertexPtr = &verts[i * vheader.StrideBytes];

            int offset = 0;
            for( int id = 0; id < vbDecl.size( ); id++ )
            {
                D3D11_INPUT_ELEMENT_DESC vbde = vbDecl[id];
                if( vbde.AlignedByteOffset != D3D11_APPEND_ALIGNED_ELEMENT )
                    offset = vbde.AlignedByteOffset;

                const uint8_t * elementPtr = vertexPtr + offset;
                offset += GetFormatElementSize( vbde.Format );

                if( vbde.SemanticName == "SV_Position" )
                {
                    assert( vbde.Format == DXGI_FORMAT_R32G32B32_FLOAT );

                    vertices[i] = *reinterpret_cast<const vaVector3 *>( elementPtr );
                }
                else if( vbde.SemanticName == "NORMAL" )
                {
                    assert( vbde.Format == DXGI_FORMAT_R32G32B32_FLOAT );

                    normals[i] = *reinterpret_cast<const vaVector3 *>( elementPtr );
                    hasNormals = true;
                }
                else if( vbde.SemanticName == "COLOR" )
                {
                    assert( vbde.Format == DXGI_FORMAT_B8G8R8A8_UNORM );

                    colors[i] = *reinterpret_cast<const uint32 *>( elementPtr );
                }
                else if( vbde.SemanticName == "TANGENT" )
                {
                    assert( vbde.Format == DXGI_FORMAT_R32G32B32_FLOAT );

                    tangents[i] = vaVector4( *reinterpret_cast<const vaVector3 *>( elementPtr ), 1.0f );
                    hasTangents = true;
                }
                else if( vbde.SemanticName == "BINORMAL" )
                {
                    assert( vbde.Format == DXGI_FORMAT_R32G32B32_FLOAT );

                    vaVector3 bitangent = *reinterpret_cast<const vaVector3 *>( elementPtr );
                }
                else if( vbde.SemanticName == "TEXCOORD" )
                {
                    assert( vbde.Format == DXGI_FORMAT_R32G32_FLOAT );

                    if( vbde.SemanticIndex == 0 )
                    {
                        texcoords0[i] = *reinterpret_cast<const vaVector2 *>( elementPtr );
                        hasTexcoords0 = true;
                    }
                    else
                    {
                        texcoords1[i] = *reinterpret_cast<const vaVector2 *>( elementPtr );
                        hasTexcoords1 = true;
                    }
                }
                else if( vbde.SemanticName == "BLENDINDICES" )
                {
                    assert( vbde.Format == DXGI_FORMAT_R8G8B8A8_UINT );

                    UINT blendIndices = *reinterpret_cast<const uint32 *>( elementPtr );
                }
                else if( vbde.SemanticName == "BLENDWEIGHT" )
                {
                    assert( vbde.Format == DXGI_FORMAT_R8G8B8A8_UNORM );

                    UINT blendWeights = *reinterpret_cast<const uint32 *>( elementPtr );
                }
                else
                {
                    assert( false );
                }

            }
        }
        // END OF PROCESS VERTICES
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        // PROCESS INDICES

        vector<uint32>      indices;
        indices.resize( iheader.NumIndices );

        if( dataSize < iheader.DataOffset
            || ( dataSize < iheader.DataOffset + iheader.SizeBytes ) )
            throw std::exception( "End of file" );

        if( iheader.IndexType != DXUT::IT_16BIT && iheader.IndexType != DXUT::IT_32BIT )
            throw std::exception( "Invalid index buffer type found" );

        auto indicesPtr = reinterpret_cast<const uint8_t*>( bufferData + ( iheader.DataOffset - bufferDataOffset ) );
        indices.resize( iheader.NumIndices );

        if( iheader.IndexType == DXUT::IT_16BIT )
        {
            const uint16 * indicesPtrTyped = reinterpret_cast<const uint16 *>( indicesPtr );
            for( int ii = 0; ii < iheader.NumIndices; ii++ )
                indices[ii] = indicesPtrTyped[ii];
        }
        else if( iheader.IndexType == DXUT::IT_32BIT )
        {
            const uint32 * indicesPtrTyped = reinterpret_cast<const uint32 *>( indicesPtr );
            for( int ii = 0; ii < iheader.NumIndices; ii++ )
                indices[ii] = indicesPtrTyped[ii];
        }
        // END OF PROCESS INDICES
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        vector<vaRenderMesh::SubPart> parts;
        parts.resize( currentMesh->NumSubsets );

        vaWindingOrder windingOrder = vaWindingOrder::Clockwise;

        for( UINT subseti = 0; subseti < currentMesh->NumSubsets; subseti++ )
        {
            const DXUT::SDKMESH_SUBSET * subset = &subsetArray[ currentMesh->pSubsets[subseti] ];
            UINT IndexCount = (UINT)subset->IndexCount;
            UINT IndexStart = (UINT)subset->IndexStart;

            parts[subseti].IndexStart   = IndexStart;
            parts[subseti].IndexCount   = IndexCount;
            //parts[subseti].Material     = subset->Name
            if( ( subset->MaterialID >= 0 ) && ( subset->MaterialID < (UINT)loadedMaterials.size( ) ) )
            {
                assert( loadedMaterials[subset->MaterialID]->GetMaterial() != nullptr );
                const shared_ptr<vaRenderMaterial> & vamat = *loadedMaterials[subset->MaterialID]->GetMaterial();
                parts[subseti].Material     = vamat;
                parts[subseti].MaterialID   = vamat->UIDObject_GetUID();
            }
            else
            {
                assert( false );
            }

            if( !hasNormals )
            {
                vaTriangleMeshTools::GenerateNormals( normals, vertices, indices, windingOrder == vaWindingOrder::CounterClockwise, IndexStart, IndexCount );
            }
        }

        // this doesn't really work
        if( !hasTangents )
        {
            // disabled as UVs broken
    //        if( !hasTexcoords0 )
            {
                // no tex coords? hmm just create some dummy tangents... 
                for( size_t i = 0; i < vertices.size( ); i++ )
                {
                    vaVector3 bitangent = ( vertices[i] + vaVector3( 0.0f, 0.0f, -5.0f ) ).Normalize( );
                    if( vaMath::Abs( vaVector3::Dot( bitangent, normals[i] ) ) > 0.9f )
                        bitangent = ( vertices[i] + vaVector3( -5.0f, 0.0f, 0.0f ) ).Normalize( );
                    tangents[i] = vaVector4( vaVector3::Cross( bitangent, normals[i] ).Normalize( ), 1.0f );
                }
            }
            //        else
            //        {
            //            vaTriangleMeshTools::GenerateTangents( tangents, vertices, normals, texcoords0, indices );
            //        }
        }

        shared_ptr<vaRenderMesh> newMesh = vaRenderMesh::Create( parameters.BaseTransform, vertices, normals, tangents, texcoords0, texcoords1, indices, windingOrder );
        newMesh->SetParts( parts );
        shared_ptr<vaAsset> newAsset = parameters.AssetPack.Add( newMesh, parameters.AssetPack.FindSuitableAssetName( currentMesh->Name ) );

        if( outContent != nullptr )
            outContent->LoadedAssets.push_back( newAsset );
    }


    /*

    vaWindingOrder windingOrder = vaWindingOrder::Clockwise;


    */

    free( meshData );

    return true;
}

