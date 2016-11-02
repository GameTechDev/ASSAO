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

#include "vaTextureTypes.h"

using namespace VertexAsylum;


string vaTextureFormatHelpers::EnumToString( vaTextureFormat val )
{
    assert( (int)vaTextureFormat::MaxVal == 116 ); // need to update the table maybe?

    const char * table[] = {
        "Unknown",
        "R32G32B32A32_TYPELESS",
        "R32G32B32A32_FLOAT",
        "R32G32B32A32_UINT",
        "R32G32B32A32_SINT",
        "R32G32B32_TYPELESS",
        "R32G32B32_FLOAT",
        "R32G32B32_UINT",
        "R32G32B32_SINT",
        "R16G16B16A16_TYPELESS",
        "R16G16B16A16_FLOAT",
        "R16G16B16A16_UNORM",
        "R16G16B16A16_UINT",
        "R16G16B16A16_SNORM",
        "R16G16B16A16_SINT",
        "R32G32_TYPELESS",
        "R32G32_FLOAT",
        "R32G32_UINT",
        "R32G32_SINT",
        "R32G8X24_TYPELESS",
        "D32_FLOAT_S8X24_UINT",
        "R32_FLOAT_X8X24_TYPELESS",
        "X32_TYPELESS_G8X24_UINT",
        "R10G10B10A2_TYPELESS",
        "R10G10B10A2_UNORM",
        "R10G10B10A2_UINT",
        "R11G11B10_FLOAT",
        "R8G8B8A8_TYPELESS",
        "R8G8B8A8_UNORM",
        "R8G8B8A8_UNORM_SRGB",
        "R8G8B8A8_UINT",
        "R8G8B8A8_SNORM",
        "R8G8B8A8_SINT",
        "R16G16_TYPELESS",
        "R16G16_FLOAT",
        "R16G16_UNORM",
        "R16G16_UINT",
        "R16G16_SNORM",
        "R16G16_SINT",
        "R32_TYPELESS",
        "D32_FLOAT",
        "R32_FLOAT",
        "R32_UINT",
        "R32_SINT",
        "R24G8_TYPELESS",
        "D24_UNORM_S8_UINT",
        "R24_UNORM_X8_TYPELESS",
        "X24_TYPELESS_G8_UINT",
        "R8G8_TYPELESS",
        "R8G8_UNORM",
        "R8G8_UINT",
        "R8G8_SNORM",
        "R8G8_SINT",
        "R16_TYPELESS",
        "R16_FLOAT",
        "D16_UNORM",
        "R16_UNORM",
        "R16_UINT",
        "R16_SNORM",
        "R16_SINT",
        "R8_TYPELESS",
        "R8_UNORM",
        "R8_UINT",
        "R8_SNORM",
        "R8_SINT",
        "A8_UNORM",
        "R1_UNORM",
        "R9G9B9E5_SHAREDEXP",
        "R8G8_B8G8_UNORM",
        "G8R8_G8B8_UNORM",
        "BC1_TYPELESS",
        "BC1_UNORM",
        "BC1_UNORM_SRGB",
        "BC2_TYPELESS",
        "BC2_UNORM",
        "BC2_UNORM_SRGB",
        "BC3_TYPELESS",
        "BC3_UNORM",
        "BC3_UNORM_SRGB",
        "BC4_TYPELESS",
        "BC4_UNORM",
        "BC4_SNORM",
        "BC5_TYPELESS",
        "BC5_UNORM",
        "BC5_SNORM",
        "B5G6R5_UNORM",
        "B5G5R5A1_UNORM",
        "B8G8R8A8_UNORM",
        "B8G8R8X8_UNORM",
        "R10G10B10_XR_BIAS_A2_UNORM",
        "B8G8R8A8_TYPELESS",
        "B8G8R8A8_UNORM_SRGB",
        "B8G8R8X8_TYPELESS",
        "B8G8R8X8_UNORM_SRGB",
        "BC6H_TYPELESS",
        "BC6H_UF16",
        "BC6H_SF16",
        "BC7_TYPELESS",
        "BC7_UNORM",
        "BC7_UNORM_SRGB",
        "AYUV",
        "Y410",
        "Y416",
        "NV12",
        "P010",
        "P016",
        "F420_OPAQUE",
        "YUY2",
        "Y210",
        "Y216",
        "NV11",
        "AI44",
        "IA44",
        "P8",
        "A8P8",
        "B4G4R4A4_UNORM",
    };

    int index = (int)val;
    if( ( index < 0 ) || ( index > _countof( table ) - 1 ) )
    {
        assert( false );
        return "OutOfRangeError";
    }

    return table[index];
}

int vaTextureFormatHelpers::GetPixelSizeInBytes( vaTextureFormat val )
{
    switch( val )
    {
    case VertexAsylum::vaTextureFormat::Unknown:                    return 0;
    case VertexAsylum::vaTextureFormat::R32G32B32A32_TYPELESS:      return 4*4;
    case VertexAsylum::vaTextureFormat::R32G32B32A32_FLOAT:         return 4*4;
    case VertexAsylum::vaTextureFormat::R32G32B32A32_UINT:          return 4*4;
    case VertexAsylum::vaTextureFormat::R32G32B32A32_SINT:          return 4*4;
    case VertexAsylum::vaTextureFormat::R32G32B32_TYPELESS:         return 3*4;
    case VertexAsylum::vaTextureFormat::R32G32B32_FLOAT:            return 3*4;
    case VertexAsylum::vaTextureFormat::R32G32B32_UINT:             return 3*4;
    case VertexAsylum::vaTextureFormat::R32G32B32_SINT:             return 3*4;
    case VertexAsylum::vaTextureFormat::R16G16B16A16_TYPELESS:      return 4*2;
    case VertexAsylum::vaTextureFormat::R16G16B16A16_FLOAT:         return 4*2;
    case VertexAsylum::vaTextureFormat::R16G16B16A16_UNORM:         return 4*2;
    case VertexAsylum::vaTextureFormat::R16G16B16A16_UINT:          return 4*2;
    case VertexAsylum::vaTextureFormat::R16G16B16A16_SNORM:         return 4*2;
    case VertexAsylum::vaTextureFormat::R16G16B16A16_SINT:          return 4*2;
    case VertexAsylum::vaTextureFormat::R32G32_TYPELESS:            return 2*4;
    case VertexAsylum::vaTextureFormat::R32G32_FLOAT:               return 2*4;
    case VertexAsylum::vaTextureFormat::R32G32_UINT:                return 2*4;
    case VertexAsylum::vaTextureFormat::R32G32_SINT:                return 2*4;
    case VertexAsylum::vaTextureFormat::R32G8X24_TYPELESS:          return 4+1;
    case VertexAsylum::vaTextureFormat::D32_FLOAT_S8X24_UINT:       return 4+1;
    case VertexAsylum::vaTextureFormat::R32_FLOAT_X8X24_TYPELESS:   return 4+1;
    case VertexAsylum::vaTextureFormat::X32_TYPELESS_G8X24_UINT:    return 4+1;
    case VertexAsylum::vaTextureFormat::R10G10B10A2_TYPELESS:       return 4;
    case VertexAsylum::vaTextureFormat::R10G10B10A2_UNORM:          return 4;
    case VertexAsylum::vaTextureFormat::R10G10B10A2_UINT:           return 4;
    case VertexAsylum::vaTextureFormat::R11G11B10_FLOAT:            return 4;
    case VertexAsylum::vaTextureFormat::R8G8B8A8_TYPELESS:          return 4;
    case VertexAsylum::vaTextureFormat::R8G8B8A8_UNORM:             return 4;
    case VertexAsylum::vaTextureFormat::R8G8B8A8_UNORM_SRGB:        return 4;
    case VertexAsylum::vaTextureFormat::R8G8B8A8_UINT:              return 4;
    case VertexAsylum::vaTextureFormat::R8G8B8A8_SNORM:             return 4;
    case VertexAsylum::vaTextureFormat::R8G8B8A8_SINT:              return 4;
    case VertexAsylum::vaTextureFormat::R16G16_TYPELESS:            return 4;
    case VertexAsylum::vaTextureFormat::R16G16_FLOAT:               return 4;
    case VertexAsylum::vaTextureFormat::R16G16_UNORM:               return 4;
    case VertexAsylum::vaTextureFormat::R16G16_UINT:                return 4;
    case VertexAsylum::vaTextureFormat::R16G16_SNORM:               return 4;
    case VertexAsylum::vaTextureFormat::R16G16_SINT:                return 4;
    case VertexAsylum::vaTextureFormat::R32_TYPELESS:               return 4;
    case VertexAsylum::vaTextureFormat::D32_FLOAT:                  return 4;
    case VertexAsylum::vaTextureFormat::R32_FLOAT:                  return 4;
    case VertexAsylum::vaTextureFormat::R32_UINT:                   return 4;
    case VertexAsylum::vaTextureFormat::R32_SINT:                   return 4;
    case VertexAsylum::vaTextureFormat::R24G8_TYPELESS:             return 4;
    case VertexAsylum::vaTextureFormat::D24_UNORM_S8_UINT:          return 4;
    case VertexAsylum::vaTextureFormat::R24_UNORM_X8_TYPELESS:      return 4;
    case VertexAsylum::vaTextureFormat::X24_TYPELESS_G8_UINT:       return 4;
    case VertexAsylum::vaTextureFormat::R8G8_TYPELESS:              return 2;
    case VertexAsylum::vaTextureFormat::R8G8_UNORM:                 return 2;
    case VertexAsylum::vaTextureFormat::R8G8_UINT:                  return 2;
    case VertexAsylum::vaTextureFormat::R8G8_SNORM:                 return 2;
    case VertexAsylum::vaTextureFormat::R8G8_SINT:                  return 2;
    case VertexAsylum::vaTextureFormat::R16_TYPELESS:               return 2;
    case VertexAsylum::vaTextureFormat::R16_FLOAT:                  return 2;
    case VertexAsylum::vaTextureFormat::D16_UNORM:                  return 2;
    case VertexAsylum::vaTextureFormat::R16_UNORM:                  return 2;
    case VertexAsylum::vaTextureFormat::R16_UINT:                   return 2;
    case VertexAsylum::vaTextureFormat::R16_SNORM:                  return 2;
    case VertexAsylum::vaTextureFormat::R16_SINT:                   return 2;
    case VertexAsylum::vaTextureFormat::R8_TYPELESS:                return 1;
    case VertexAsylum::vaTextureFormat::R8_UNORM:                   return 1;
    case VertexAsylum::vaTextureFormat::R8_UINT:                    return 1;
    case VertexAsylum::vaTextureFormat::R8_SNORM:                   return 1;
    case VertexAsylum::vaTextureFormat::R8_SINT:                    return 1;
    case VertexAsylum::vaTextureFormat::A8_UNORM:                   return 1;
    case VertexAsylum::vaTextureFormat::R1_UNORM:                   return 1;
    case VertexAsylum::vaTextureFormat::R9G9B9E5_SHAREDEXP:         return 4;
    case VertexAsylum::vaTextureFormat::R8G8_B8G8_UNORM:            return 4;
    case VertexAsylum::vaTextureFormat::G8R8_G8B8_UNORM:            return 4;
    case VertexAsylum::vaTextureFormat::BC1_TYPELESS:               assert( false ); return 0; // not supported for compressed formats
    case VertexAsylum::vaTextureFormat::BC1_UNORM:                  assert( false ); return 0; // not supported for compressed formats
    case VertexAsylum::vaTextureFormat::BC1_UNORM_SRGB:             assert( false ); return 0; // not supported for compressed formats
    case VertexAsylum::vaTextureFormat::BC2_TYPELESS:               assert( false ); return 0; // not supported for compressed formats
    case VertexAsylum::vaTextureFormat::BC2_UNORM:                  assert( false ); return 0; // not supported for compressed formats
    case VertexAsylum::vaTextureFormat::BC2_UNORM_SRGB:             assert( false ); return 0; // not supported for compressed formats
    case VertexAsylum::vaTextureFormat::BC3_TYPELESS:               assert( false ); return 0; // not supported for compressed formats
    case VertexAsylum::vaTextureFormat::BC3_UNORM:                  assert( false ); return 0; // not supported for compressed formats
    case VertexAsylum::vaTextureFormat::BC3_UNORM_SRGB:             assert( false ); return 0; // not supported for compressed formats
    case VertexAsylum::vaTextureFormat::BC4_TYPELESS:               assert( false ); return 0; // not supported for compressed formats
    case VertexAsylum::vaTextureFormat::BC4_UNORM:                  assert( false ); return 0; // not supported for compressed formats
    case VertexAsylum::vaTextureFormat::BC4_SNORM:                  assert( false ); return 0; // not supported for compressed formats
    case VertexAsylum::vaTextureFormat::BC5_TYPELESS:               assert( false ); return 0; // not supported for compressed formats
    case VertexAsylum::vaTextureFormat::BC5_UNORM:                  assert( false ); return 0; // not supported for compressed formats
    case VertexAsylum::vaTextureFormat::BC5_SNORM:                  assert( false ); return 0; // not supported for compressed formats
    case VertexAsylum::vaTextureFormat::B5G6R5_UNORM:               return 2;
    case VertexAsylum::vaTextureFormat::B5G5R5A1_UNORM:             return 2;
    case VertexAsylum::vaTextureFormat::B8G8R8A8_UNORM:             return 4;
    case VertexAsylum::vaTextureFormat::B8G8R8X8_UNORM:             return 4;
    case VertexAsylum::vaTextureFormat::R10G10B10_XR_BIAS_A2_UNORM: return 4;
    case VertexAsylum::vaTextureFormat::B8G8R8A8_TYPELESS:          return 4;
    case VertexAsylum::vaTextureFormat::B8G8R8A8_UNORM_SRGB:        return 4;
    case VertexAsylum::vaTextureFormat::B8G8R8X8_TYPELESS:          return 4;
    case VertexAsylum::vaTextureFormat::B8G8R8X8_UNORM_SRGB:        return 4;
    case VertexAsylum::vaTextureFormat::BC6H_TYPELESS:              assert( false ); return 0; // not supported for compressed formats
    case VertexAsylum::vaTextureFormat::BC6H_UF16:                  assert( false ); return 0; // not supported for compressed formats
    case VertexAsylum::vaTextureFormat::BC6H_SF16:                  assert( false ); return 0; // not supported for compressed formats
    case VertexAsylum::vaTextureFormat::BC7_TYPELESS:               assert( false ); return 0; // not supported for compressed formats
    case VertexAsylum::vaTextureFormat::BC7_UNORM:                  assert( false ); return 0; // not supported for compressed formats
    case VertexAsylum::vaTextureFormat::BC7_UNORM_SRGB:             assert( false ); return 0; // not supported for compressed formats
    case VertexAsylum::vaTextureFormat::AYUV:                       assert( false ); return 0; // not yet implemented
    case VertexAsylum::vaTextureFormat::Y410:                       assert( false ); return 0; // not yet implemented
    case VertexAsylum::vaTextureFormat::Y416:                       assert( false ); return 0; // not yet implemented
    case VertexAsylum::vaTextureFormat::NV12:                       assert( false ); return 0; // not yet implemented
    case VertexAsylum::vaTextureFormat::P010:                       assert( false ); return 0; // not yet implemented
    case VertexAsylum::vaTextureFormat::P016:                       assert( false ); return 0; // not yet implemented
    case VertexAsylum::vaTextureFormat::F420_OPAQUE:                assert( false ); return 0; // not yet implemented
    case VertexAsylum::vaTextureFormat::YUY2:                       assert( false ); return 0; // not yet implemented
    case VertexAsylum::vaTextureFormat::Y210:                       assert( false ); return 0; // not yet implemented
    case VertexAsylum::vaTextureFormat::Y216:                       assert( false ); return 0; // not yet implemented
    case VertexAsylum::vaTextureFormat::NV11:                       assert( false ); return 0; // not yet implemented
    case VertexAsylum::vaTextureFormat::AI44:                       assert( false ); return 0; // not yet implemented
    case VertexAsylum::vaTextureFormat::IA44:                       assert( false ); return 0; // not yet implemented
    case VertexAsylum::vaTextureFormat::P8:                         return 1;
    case VertexAsylum::vaTextureFormat::A8P8:                       return 2;
    case VertexAsylum::vaTextureFormat::B4G4R4A4_UNORM:             return 2;
    default: break;
    }
    assert( false );
    return 0;
}