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

#include "Rendering/vaRendering.h"
#include "Rendering/vaRenderMesh.h"

#include "IntegratedExternals/vaImguiIntegration.h"

namespace VertexAsylum
{
    class vaPosRotBoundsObjectBase
    {
    protected:
        vaBoundingBox                               m_boundingBox;
        vaVector3                                   m_position;
        vaQuaternion                                m_rotation;
    
    protected:
        vaPosRotBoundsObjectBase( )                 { m_boundingBox = vaBoundingBox::Degenerate; m_position = vaVector3( 0.0f, 0.0f, 0.0f ); m_rotation = vaQuaternion::Identity; }
        virtual ~vaPosRotBoundsObjectBase( )        { }
    
    public:
        // make these virtual if/when you need them and just those that need to actually be virtual (if you really, really need them to be virtual)
        void                                        SetPosition( const vaVector3 & newPos )             { m_position = newPos; }
        void                                        SetRotation( const vaQuaternion & newOri )          { m_rotation = newOri; }
        const vaVector3 &                           GetPosition( ) const                                { return m_position; }
        const vaQuaternion &                        GetRotation( ) const                                { return m_rotation; }
    
        vaMatrix4x4                                 GetTransform( ) const                               { return vaMatrix4x4::FromRotationTranslation( m_rotation, m_position ); }
        void                                        GetBounds( vaOrientedBoundingBox & bounds ) const   { bounds = vaOrientedBoundingBox::FromAABBAndTransform( m_boundingBox, GetTransform() ); }
    };

    class vaSceneObject : public vaPosRotBoundsObjectBase
    {

    };

    class vaScene : public vaImguiHierarchyObject
    {
    protected:
        std::shared_ptr<vaRenderMeshManager>        m_renderer;
        //std::shared_ptr<vaScenePhysics>        m_physics;

    public:
        vaScene( ) { }
        //vaScene( const std::shared_ptr<vaRenderMeshManager> & renderer ) : m_renderer( renderer ) { }
        virtual ~vaScene( ) { }
    };
}