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

#include "vaUIDObject.h"

#include "vaLog.h"

using namespace VertexAsylum;

vaUIDObject::vaUIDObject( const vaGUID & uid ) : 
    m_correctlyTracked( false ),
    m_uid( uid ),
    m_trackee( &vaUIDObjectRegistrar::GetInstance().m_objects, this )
{ 
}

vaUIDObjectRegistrar::vaUIDObjectRegistrar()
{
    m_objects.SetAddedCallback( std::bind( &vaUIDObjectRegistrar::UIDObjectTrackeeAddedCallback, this, std::placeholders::_1 ) );
    m_objects.SetBeforeRemovedCallback( std::bind( &vaUIDObjectRegistrar::UIDObjectTrackeeBeforeRemovedCallback, this, std::placeholders::_1, std::placeholders::_2 ) );
}

void vaUIDObjectRegistrar::UIDObjectTrackeeAddedCallback( int newTrackeeIndex )
{
    auto it = m_objectsMap.find( m_objects[newTrackeeIndex]->m_uid );
    if( it != m_objectsMap.end( ) )
    {
        //VA_ASSERT_ALWAYS( "New vaUIDObject created but the ID already exists: this is a bug, the new object will not be tracked and will not be searchable by vaUIDObjectRegistrar::Find" );
        VA_LOG_ERROR( "New vaUIDObject created but the ID already exists: this is a bug, the new object will not be tracked and will not be searchable by vaUIDObjectRegistrar::Find" );
        m_objects[newTrackeeIndex]->m_correctlyTracked = false;
    }
    else
    {
        m_objectsMap.insert( std::make_pair( m_objects[newTrackeeIndex]->m_uid, m_objects[newTrackeeIndex] ) );
        m_objects[newTrackeeIndex]->m_correctlyTracked = true;
    }
}

void vaUIDObjectRegistrar::UIDObjectTrackeeBeforeRemovedCallback( int toBeRemovedTrackeeIndex, int toBeReplacedByTrackeeIndex )
{
    // if we're not correctly tracked, there's no point removing us from the map (in fact, we will likely remove another instance)
    if( !m_objects[toBeRemovedTrackeeIndex]->m_correctlyTracked )
    {
        VA_LOG_WARNING( "Deleting an untracked vaUIDObject; There were errors on creation, check the log." );
        return;
    }

    auto it = m_objectsMap.find( m_objects[toBeRemovedTrackeeIndex]->m_uid );
    if( it == m_objectsMap.end( ) )
    {
        VA_LOG_ERROR( "Deleting a tracked vaUIDObject that couldn't be found: this is an indicator of a more serious error such as an algorithm bug or a memory overwrite. Don't ignore it." );
        assert( false );
    }
    else
    {
        // if this happens, we're removing wrong object - this is a serious error, don't ignore it!
        assert( m_objects[toBeRemovedTrackeeIndex] == it->second );

        m_objects[toBeRemovedTrackeeIndex]->m_correctlyTracked = false;
        m_objectsMap.erase( it );
    }
}
