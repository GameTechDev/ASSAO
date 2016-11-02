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

#include "vaCore.h"
#include "vaSingleton.h"
#include "Containers\vaTrackerTrackee.h"
#include "System\vaStream.h"

namespace VertexAsylum
{   
   ////////////////////////////////////////////////////////////////////////////////////////////////
   // vaUIDObject
   class vaUIDObject
   {
   protected:
       friend class vaUIDObjectRegistrar;
       bool                                         m_correctlyTracked;
       vaGUID const                                 m_uid;
       vaTT_Trackee< vaUIDObject * >                m_trackee;

   protected:
       vaUIDObject( const vaGUID & uid );
       virtual ~vaUIDObject( ) { }

   public:
       const vaGUID &                               UIDObject_GetUID( ) const               { return m_uid; }
       bool                                         UIDObject_IsCorrectlyTracked( ) const   { return m_correctlyTracked; }
   };

   class vaUIDObjectRegistrar : public vaSingletonBase< vaUIDObjectRegistrar >
   {
   protected:
       friend class vaUIDObject;
       vaTT_Tracker< vaUIDObject * >                m_objects;

       map< vaGUID, vaUIDObject*, vaGUIDComparer >  m_objectsMap;

   private:
       friend class vaCore;
       vaUIDObjectRegistrar( )  ;
       ~vaUIDObjectRegistrar( ) 
       { 
           // not 0? memory leak or not all objects deleted before the registrar was deleted (bug)
           assert( m_objects.size() == 0 );
           assert( m_objectsMap.size() == 0 ); 
       }
       
   protected:
       void                                         UIDObjectTrackeeAddedCallback( int newTrackeeIndex );
       void                                         UIDObjectTrackeeBeforeRemovedCallback( int toBeRemovedTrackeeIndex, int toBeReplacedByTrackeeIndex );

   public:
       template< class T >
       static T *                                   Find( const vaGUID & uid );

       template< class T >
       static void                                  ReconnectDependency( std::shared_ptr<T> & outSharedPtr, const vaGUID & uid );
   };

   // inline 

   template< class T>
   inline T * vaUIDObjectRegistrar::Find( const vaGUID & uid )
   {
       if( uid == vaCore::GUIDNull() )
           return NULL;

       auto it = vaUIDObjectRegistrar::GetInstance( ).m_objectsMap.find( uid );
       if( it == vaUIDObjectRegistrar::GetInstance( ).m_objectsMap.end( ) )
       {
           return nullptr;
       }
       else
       {
#ifdef _DEBUG
           T * ret = dynamic_cast<T*>( it->second );
           assert( ret != NULL );
           return ret;
#else
           return static_cast<T*>( it->second );
#endif
       }
   }

   template< class T >
   inline void vaUIDObjectRegistrar::ReconnectDependency( std::shared_ptr<T> & outSharedPtr, const vaGUID & uid )
   {
       T * obj;
       obj = Find<T>( uid );
       if( obj != nullptr )
           outSharedPtr = std::static_pointer_cast<T>( obj->shared_from_this( ) );
       else
           outSharedPtr = nullptr;
   }

   inline bool SaveUIDObjectUID( vaStream & outStream, const shared_ptr<vaUIDObject> & obj )
   {
       if( obj == nullptr )
       {
           VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<vaGUID>( vaCore::GUIDNull( ) ) );
       }
       else
       {
           VERIFY_TRUE_RETURN_ON_FALSE( outStream.WriteValue<vaGUID>( obj->UIDObject_GetUID( ) ) );
       }
       return true;
   }

}