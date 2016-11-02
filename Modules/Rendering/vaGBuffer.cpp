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

#include "vaGBuffer.h"

using namespace VertexAsylum;

vaGBuffer::vaGBuffer( )
{
    m_debugInfo = "GBuffer (uninitialized - forgot to call RenderTick?)";
    m_debugSelectedTexture = -1;

    m_resolution = vaVector2i( 0, 0 );
}

vaGBuffer::~vaGBuffer( )
{

}

void vaGBuffer::SetFormats( const BufferFormats & newFormats )
{
    m_formats = newFormats;
}

void vaGBuffer::IHO_Draw( )
{
#ifdef VA_IMGUI_INTEGRATION_ENABLED
    struct TextureInfo
    {
        string                  Name;
        shared_ptr<vaTexture>   Texture;
    };

    std::vector< TextureInfo > textures;

    textures.push_back( { "Depth Buffer", m_depthBuffer } );
    textures.push_back( { "Depth Buffer Viewspace Linear", m_depthBufferViewspaceLinear } );
    textures.push_back( { "Normal Map", m_normalMap } );
    textures.push_back( { "Albedo", m_albedo } );
    textures.push_back( { "Radiance", m_radiance} );
    textures.push_back( { "OutputColor", m_outputColor} );

    for( size_t i = 0; i < textures.size(); i++ )
    {
        if( ImGui::Selectable( textures[i].Name.c_str(), m_debugSelectedTexture == i ) )
        {
            if( m_debugSelectedTexture == i )
                m_debugSelectedTexture = -1;
            else
                m_debugSelectedTexture = (int)i;
        }
    }
#endif
}