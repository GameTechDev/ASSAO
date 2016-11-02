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

////////////////////////////////////////////////////////////////////////////////////////////////
// Code in vaStandardShapes.h/cpp uses ideas from Microsoft's DXUTShapes.cpp and Assimp library
// ( http://assimp.sourceforge.net/, 3.1.1 )
// Please find Assimp license at the bottom of header file 
////////////////////////////////////////////////////////////////////////////////////////////////

#include "Core/vaCoreIncludes.h"

#include "vaTriangleMesh.h"

namespace VertexAsylum
{ 

    class vaStandardShapes
    {
        vaStandardShapes( ) {}

    public:
        // all of these produce shapes with center in (0, 0, 0) and each vertex magnitude of 1 (normalized), except where specified otherwise
        // front faces are counter-clockwise
        static void CreatePlane( std::vector<vaVector3> & outVertices, std::vector<uint32> & outIndices, float sizeX, float sizeY );
        static void CreateTetrahedron( std::vector<vaVector3> & outVertices, std::vector<uint32> & outIndices, bool shareVertices );
        static void CreateCube( std::vector<vaVector3> & outVertices, std::vector<uint32> & outIndices, bool shareVertices, float edgeHalfLength = 0.7071067811865475f );
        static void CreateOctahedron( std::vector<vaVector3> & outVertices, std::vector<uint32> & outIndices, bool shareVertices );
        static void CreateIcosahedron( std::vector<vaVector3> & outVertices, std::vector<uint32> & outIndices, bool shareVertices );
        static void CreateDodecahedron( std::vector<vaVector3> & outVertices, std::vector<uint32> & outIndices, bool shareVertices );
        static void CreateSphere( std::vector<vaVector3> & outVertices, std::vector<uint32> & outIndices, int tessellationLevel, bool shareVertices );
        static void CreateCylinder( std::vector<vaVector3> & outVertices, std::vector<uint32> & outIndices, float height, float radiusBottom, float radiusTop, int tessellation, bool openTopBottom, bool shareVertices );
        static void CreateTeapot( std::vector<vaVector3> & outVertices, std::vector<uint32> & outIndices );
    };

}


////////////////////////////////////////////////////////////////////////////////////////////////
// Code in vaStandardShapes.h/cpp uses ideas from Assimp library, http://assimp.sourceforge.net/, 3.1.1
// Please refer to Assimp license below
////////////////////////////////////////////////////////////////////////////////////////////////
//
// /*
// Open Asset Import Library (assimp)
// ----------------------------------------------------------------------
// 
// Copyright (c) 2006-2012, assimp team
// All rights reserved.
// 
// Redistribution and use of this software in source and binary forms, 
// with or without modification, are permitted provided that the 
// following conditions are met:
// 
// * Redistributions of source code must retain the above
//   copyright notice, this list of conditions and the
//   following disclaimer.
// 
// * Redistributions in binary form must reproduce the above
//   copyright notice, this list of conditions and the
//   following disclaimer in the documentation and/or other
//   materials provided with the distribution.
// 
// * Neither the name of the assimp team, nor the names of its
//   contributors may be used to endorse or promote products
//   derived from this software without specific prior
//   written permission of the assimp team.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// ----------------------------------------------------------------------
// */
//////////////////////////////////////////////////////////////////////////
