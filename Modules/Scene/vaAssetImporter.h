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

#include "Rendering/vaRenderMesh.h"

#include "Rendering/vaAssetPack.h"

namespace VertexAsylum
{
    class vaAssetImporter
    {
    private:
        vaAssetImporter( )                { }
        virtual ~vaAssetImporter( )       { }

    public:

        struct LoadingParameters
        {
            // pack to save assets into and to search dependencies to link to
            vaAssetPack &               AssetPack;

            string                      NamePrefix;             // added to loaded asset resource names (can be used to create hierarchy - "importfilename\"
            vaMatrix4x4                 BaseTransform;          // for conversion between coord systems, etc

            bool                        TextureOnlyLoadDDS;
            bool                        TextureTryLoadDDS;
            bool                        TextureAlphaMaskInColorAlpha;

            bool                        ForceGenerateNormals;
            bool                        GenerateNormalsIfNeeded;
            bool                        GenerateSmoothNormalsIfGenerating;
            float                       GenerateSmoothNormalsSmoothingAngle;    // in degrees, see AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE for more info
            //bool                        RegenerateTangents;      

            LoadingParameters( vaAssetPack & assetPack, const vaMatrix4x4 & baseTransform = vaMatrix4x4::Identity ) : AssetPack( assetPack ), BaseTransform( baseTransform )
            {
                NamePrefix                          = "";
                ForceGenerateNormals                = false;
                GenerateNormalsIfNeeded             = true;
                GenerateSmoothNormalsIfGenerating   = false;
                //RegenerateTangents              = false;
                TextureTryLoadDDS                   = true;
                TextureOnlyLoadDDS                  = true;
                TextureAlphaMaskInColorAlpha        = true;
                GenerateSmoothNormalsSmoothingAngle = 88.0f;
            }
        };

        struct LoadedContent
        {
            vector<shared_ptr<vaAsset>>                     LoadedAssets;
        };

    public:
        static bool                                         LoadFileContents(               const wstring & path, LoadingParameters & parameters, LoadedContent * outContent = nullptr );

    };


}