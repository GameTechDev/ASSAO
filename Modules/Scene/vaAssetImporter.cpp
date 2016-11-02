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

using namespace VertexAsylum;

bool LoadFileContents_SDKMESH( const std::shared_ptr<vaMemoryStream> & fileContents, const wstring & textureSearchPath, const wstring & name, vaAssetImporter::LoadingParameters & parameters, vaAssetImporter::LoadedContent * outContent );

bool LoadFileContents_Assimp( const wstring & path, vaAssetImporter::LoadingParameters & parameters, vaAssetImporter::LoadedContent * outContent );

bool vaAssetImporter::LoadFileContents( const wstring & path, LoadingParameters & parameters, LoadedContent * outContent )
{
    wstring filename;
    wstring ext;
    wstring textureSearchPath;
    vaStringTools::SplitPath( path.c_str(), &textureSearchPath, &filename, &ext );
    ext = vaStringTools::ToLower( ext );
    if( ext == L".sdkmesh" )
    {
        std::shared_ptr<vaMemoryStream> fileContents;


        wstring usedPath;

        // try asset paths
        if( fileContents == nullptr )
        {
            usedPath = vaRenderingCore::GetInstance( ).FindAssetFilePath( path );
            fileContents = vaFileTools::LoadFileToMemoryStream( usedPath.c_str( ) );
        }

        // found? try load and return!
        if( fileContents == nullptr )
        {
            vaFileTools::EmbeddedFileData embeddedFile = vaFileTools::EmbeddedFilesFind( ( L"textures:\\" + path ).c_str( ) );
            if( embeddedFile.HasContents( ) )
                fileContents = embeddedFile.MemStream;
            //
        }
        return LoadFileContents_SDKMESH( fileContents, textureSearchPath, filename, parameters, outContent );
    }
    else
    {
        return LoadFileContents_Assimp( path, parameters, outContent );
    }

//        VA_WARN( L"vaAssetImporter::LoadFileContents - don't know how to parse '%s' file type!", ext.c_str( ) );
    return false;
}
