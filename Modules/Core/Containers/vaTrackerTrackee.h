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

#include "Core/vaCore.h"

#include "Core/vaSTL.h"

namespace VertexAsylum
{
    // There might be a generic C++ pattern for this problem, but I couldn't find one so I wrote these two classes.
    //
    // for example check vaUIDObject!
    // 
    // - vaTT_Trackee can only be created with a reference to vaTT_Tracker, and is then tracked in an array by the tracker.
    // - vaTT_Trackee object can only be tracked by one vaTT_Tracker object.
    // - On destruction _Trackee gets automatically removed from the _Tracker list, and it always knows its index so removing/adding is fast.
    // - If a _Tracker is destroyed, its tracked objects will get disconnected and become untracked and they can destruct at later time but cannot be tracked again.
    // - The array of tracked objects can be obtained by using vaTT_Tracker::TTGetTrackedObjects for read-only purposes.
    // - One vaTT_Trackee object cannot be tracked by more than one vaTT_Trackers, but you can create multiple Trackee-s and assign them to different trackers.

    template< class TTTagType >
    class vaTT_Trackee;


    template< class TTTagType >
    class vaTT_Tracker
    {
    private:
        template< class TTTagType >
        friend class vaTT_Trackee;
        std::vector< vaTT_Trackee<TTTagType> * >            m_tracker_objects;

    public:
        typedef std::function<void( int newTrackeeIndex )>                                         TrackeeAddedCallbackType;
        typedef std::function<void( int toBeRemovedTrackeeIndex, int toBeReplacedByTrackeeIndex )> TrackeeBeforeRemovedCallbackType;

        TrackeeAddedCallbackType                            m_onAddedCallback;
        TrackeeBeforeRemovedCallbackType                    m_beforeRemovedCallback;

    public:
        vaTT_Tracker( )                                     { }
        virtual ~vaTT_Tracker( );
        const std::vector< vaTT_Trackee<TTTagType> * > &    GetTrackedObjects( ) const          { return m_tracker_objects; };

        TTTagType                                           operator[]( std::size_t idx)        { return m_tracker_objects[idx]->m_tag; }
        const TTTagType                                     operator[]( std::size_t idx) const  { return m_tracker_objects[idx]->m_tag; }
        size_t                                              size( ) const                       { return m_tracker_objects.size();  }

        void                                                SetAddedCallback( const TrackeeAddedCallbackType & callback )                   { m_onAddedCallback   = callback; }
        void                                                SetBeforeRemovedCallback( const TrackeeBeforeRemovedCallbackType & callback )   { m_beforeRemovedCallback = callback; }
    };


    template< class TTTagType >
    class vaTT_Trackee
    {
        typedef vaTT_Tracker<TTTagType>     vaTT_TrackerT;
        friend vaTT_TrackerT;
    private:
        vaTT_TrackerT *                     m_tracker;
        int                                 m_index;
        TTTagType const                     m_tag;

    public:
        vaTT_Trackee( vaTT_TrackerT * tracker, TTTagType tag )
            : m_tag( tag )
        {
            m_tracker = tracker;
            assert( tracker != nullptr );

            m_tracker->m_tracker_objects.push_back( this );
            m_index = (int)m_tracker->m_tracker_objects.size( ) - 1;
            assert( m_index == (int)m_tracker->m_tracker_objects.size( ) - 1 );

            if( m_tracker->m_onAddedCallback != nullptr )
                m_tracker->m_onAddedCallback( m_index );
        }
        virtual ~vaTT_Trackee( )
        {
            if( m_tracker == nullptr )
                return;

            int index = m_index;
            assert( this == m_tracker->m_tracker_objects[index] );

            // not last one? move the last one to our place and update its index
            if( index < ( (int)m_tracker->m_tracker_objects.size( ) - 1 ) )
            {
                int replacedByIndex = (int)m_tracker->m_tracker_objects.size( ) - 1;
                if( m_tracker->m_beforeRemovedCallback != nullptr )
                    m_tracker->m_beforeRemovedCallback( m_index, replacedByIndex );
                m_tracker->m_tracker_objects[index] = m_tracker->m_tracker_objects[ replacedByIndex ];
                m_tracker->m_tracker_objects[index]->m_index = index;
            }
            else
            {
                if( m_tracker->m_beforeRemovedCallback != nullptr )
                    m_tracker->m_beforeRemovedCallback( m_index, -1 );
            }
            m_tracker->m_tracker_objects.pop_back( );

        }

    public:
        const vaTT_TrackerT *           GetTracker( ) const { return m_tracker; };
        TTTagType                       GetTag( ) const     { return m_tag; }
        int                             GetIndex( ) const   { return m_index; }
    };


    template< class TTTagType >
    inline vaTT_Tracker<TTTagType>::~vaTT_Tracker( )
    {
        for( int i = 0; i < m_tracker_objects.size( ); i++ )
            m_tracker_objects[i]->m_tracker = nullptr;
    }



}
