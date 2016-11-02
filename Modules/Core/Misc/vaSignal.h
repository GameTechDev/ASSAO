///////////////////////////////////////////////////////////////////////////////////////////////////
// Contents of this file are distributed under the licensing details supplied below.
///////////////////////////////////////////////////////////////////////////////////////////////////

// https://github.com/pbhogan/Signals
// Based on original code from https://github.com/pbhogan/Signals by Patrick Hogan, a lightweight
// and and non-intrusive implementation of the observer pattern (http://en.wikipedia.org/wiki/Observer_pattern), 
// more specifically a "signals and slots" pattern (http://en.wikipedia.org/wiki/Signals_and_slots).
//
// The original file info:
// *
// * A lightweight signals and slots implementation using fast delegates
// *
// * Created by Patrick Hogan on 5/18/09.
// *

#pragma once

#include "vaDelegate.h"
#include <set>

namespace VertexAsylum
{

   template< class Param0 = void >
   class vaSignal0
   {
   public:
	   typedef vaDelegate0< void > _Delegate;

   private:
	   typedef std::set<_Delegate> DelegateList;
	   typedef typename DelegateList::const_iterator DelegateIterator;
	   DelegateList delegateList;

   public:
	   void Connect( _Delegate delegate )
	   {
		   delegateList.insert( delegate );
	   }

	   template< class X, class Y >
	   void Connect( Y * obj, void (X::*func)() )
	   {
		   delegateList.insert( MakeDelegate( obj, func ) );
	   }

	   template< class X, class Y >
	   void Connect( Y * obj, void (X::*func)() const )
	   {
		   delegateList.insert( MakeDelegate( obj, func ) );
	   }

	   void Disconnect( _Delegate delegate )
	   {
		   delegateList.erase( delegate );
	   }

	   template< class X, class Y >
	   void Disconnect( Y * obj, void (X::*func)() )
	   {
		   delegateList.erase( MakeDelegate( obj, func ) );
	   }

	   template< class X, class Y >
	   void Disconnect( Y * obj, void (X::*func)() const )
	   {
		   delegateList.erase( MakeDelegate( obj, func ) );
	   }

	   void Clear()
	   {
		   delegateList.clear();
	   }

	   void Emit() const
	   {
		   for (DelegateIterator i = delegateList.begin(); i != delegateList.end(); ++i)
		   {
			   (*i)();
		   }
	   }

	   void operator() () const
	   {
		   Emit();
	   }
   };


   template< class Param1 >
   class vaSignal1
   {
   public:
	   typedef vaDelegate1< Param1 > _Delegate;

   private:
	   typedef std::set<_Delegate> DelegateList;
	   typedef typename DelegateList::const_iterator DelegateIterator;
	   DelegateList delegateList;

   public:
	   void Connect( _Delegate delegate )
	   {
		   delegateList.insert( delegate );
	   }

	   template< class X, class Y >
	   void Connect( Y * obj, void (X::*func)( Param1 p1 ) )
	   {
           assert( obj != NULL );
		   delegateList.insert( MakeDelegate( obj, func ) );
	   }

	   template< class X, class Y >
	   void Connect( Y * obj, void (X::*func)( Param1 p1 ) const )
	   {
           assert( obj != NULL );
		   delegateList.insert( MakeDelegate( obj, func ) );
	   }

	   void Disconnect( _Delegate delegate )
	   {
		   delegateList.erase( delegate );
	   }

	   template< class X, class Y >
	   void Disconnect( Y * obj, void (X::*func)( Param1 p1 ) )
	   {
           assert( obj != NULL );
		   delegateList.erase( MakeDelegate( obj, func ) );
	   }

	   template< class X, class Y >
	   void Disconnect( Y * obj, void (X::*func)( Param1 p1 ) const )
	   {
           assert( obj != NULL );
		   delegateList.erase( MakeDelegate( obj, func ) );
	   }

	   void Clear()
	   {
		   delegateList.clear();
	   }

	   void Emit( Param1 p1 ) const
	   {
		   for (DelegateIterator i = delegateList.begin(); i != delegateList.end(); ++i)
		   {
			   (*i)( p1 );
		   }
	   }

	   void operator() ( Param1 p1 ) const
	   {
		   Emit( p1 );
	   }
   };


   template< class Param1, class Param2 >
   class vaSignal2
   {
   public:
	   typedef vaDelegate2< Param1, Param2 > _Delegate;

   private:
	   typedef std::set<_Delegate> DelegateList;
	   typedef typename DelegateList::const_iterator DelegateIterator;
	   DelegateList delegateList;

   public:
	   void Connect( _Delegate delegate )
	   {
		   delegateList.insert( delegate );
	   }

	   template< class X, class Y >
	   void Connect( Y * obj, void (X::*func)( Param1 p1, Param2 p2 ) )
	   {
		   delegateList.insert( MakeDelegate( obj, func ) );
	   }

	   template< class X, class Y >
	   void Connect( Y * obj, void (X::*func)( Param1 p1, Param2 p2 ) const )
	   {
		   delegateList.insert( MakeDelegate( obj, func ) );
	   }

	   void Disconnect( _Delegate delegate )
	   {
		   delegateList.erase( delegate );
	   }

	   template< class X, class Y >
	   void Disconnect( Y * obj, void (X::*func)( Param1 p1, Param2 p2 ) )
	   {
		   delegateList.erase( MakeDelegate( obj, func ) );
	   }

	   template< class X, class Y >
	   void Disconnect( Y * obj, void (X::*func)( Param1 p1, Param2 p2 ) const )
	   {
		   delegateList.erase( MakeDelegate( obj, func ) );
	   }

	   void Clear()
	   {
		   delegateList.clear();
	   }

	   void Emit( Param1 p1, Param2 p2 ) const
	   {
		   for (DelegateIterator i = delegateList.begin(); i != delegateList.end(); ++i)
		   {
			   (*i)( p1, p2 );
		   }
	   }

	   void operator() ( Param1 p1, Param2 p2 ) const
	   {
		   Emit( p1, p2 );
	   }
   };


   template< class Param1, class Param2, class Param3 >
   class vaSignal3
   {
   public:
	   typedef vaDelegate3< Param1, Param2, Param3 > _Delegate;

   private:
	   typedef std::set<_Delegate> DelegateList;
	   typedef typename DelegateList::const_iterator DelegateIterator;
	   DelegateList delegateList;

   public:
	   void Connect( _Delegate delegate )
	   {
		   delegateList.insert( delegate );
	   }

	   template< class X, class Y >
	   void Connect( Y * obj, void (X::*func)( Param1 p1, Param2 p2, Param3 p3 ) )
	   {
		   delegateList.insert( MakeDelegate( obj, func ) );
	   }

	   template< class X, class Y >
	   void Connect( Y * obj, void (X::*func)( Param1 p1, Param2 p2, Param3 p3 ) const )
	   {
		   delegateList.insert( MakeDelegate( obj, func ) );
	   }

	   void Disconnect( _Delegate delegate )
	   {
		   delegateList.erase( delegate );
	   }

	   template< class X, class Y >
	   void Disconnect( Y * obj, void (X::*func)( Param1 p1, Param2 p2, Param3 p3 ) )
	   {
		   delegateList.erase( MakeDelegate( obj, func ) );
	   }

	   template< class X, class Y >
	   void Disconnect( Y * obj, void (X::*func)( Param1 p1, Param2 p2, Param3 p3 ) const )
	   {
		   delegateList.erase( MakeDelegate( obj, func ) );
	   }

	   void Clear()
	   {
		   delegateList.clear();
	   }

	   void Emit( Param1 p1, Param2 p2, Param3 p3 ) const
	   {
		   for (DelegateIterator i = delegateList.begin(); i != delegateList.end(); ++i)
		   {
			   (*i)( p1, p2, p3 );
		   }
	   }

	   void operator() ( Param1 p1, Param2 p2, Param3 p3 ) const
	   {
		   Emit( p1, p2, p3 );
	   }
   };


   template< class Param1, class Param2, class Param3, class Param4 >
   class vaSignal4
   {
   public:
	   typedef vaDelegate4< Param1, Param2, Param3, Param4 > _Delegate;

   private:
	   typedef std::set<_Delegate> DelegateList;
	   typedef typename DelegateList::const_iterator DelegateIterator;
	   DelegateList delegateList;

   public:
	   void Connect( _Delegate delegate )
	   {
		   delegateList.insert( delegate );
	   }

	   template< class X, class Y >
	   void Connect( Y * obj, void (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4 ) )
	   {
		   delegateList.insert( MakeDelegate( obj, func ) );
	   }

	   template< class X, class Y >
	   void Connect( Y * obj, void (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4 ) const )
	   {
		   delegateList.insert( MakeDelegate( obj, func ) );
	   }

	   void Disconnect( _Delegate delegate )
	   {
		   delegateList.erase( delegate );
	   }

	   template< class X, class Y >
	   void Disconnect( Y * obj, void (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4 ) )
	   {
		   delegateList.erase( MakeDelegate( obj, func ) );
	   }

	   template< class X, class Y >
	   void Disconnect( Y * obj, void (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4 ) const )
	   {
		   delegateList.erase( MakeDelegate( obj, func ) );
	   }

	   void Clear()
	   {
		   delegateList.clear();
	   }

	   void Emit( Param1 p1, Param2 p2, Param3 p3, Param4 p4 ) const
	   {
		   for (DelegateIterator i = delegateList.begin(); i != delegateList.end(); ++i)
		   {
			   (*i)( p1, p2, p3, p4 );
		   }
	   }

	   void operator() ( Param1 p1, Param2 p2, Param3 p3, Param4 p4 ) const
	   {
		   Emit( p1, p2, p3, p4 );
	   }
   };


   template< class Param1, class Param2, class Param3, class Param4, class Param5 >
   class vaSignal5
   {
   public:
	   typedef vaDelegate5< Param1, Param2, Param3, Param4, Param5 > _Delegate;

   private:
	   typedef std::set<_Delegate> DelegateList;
	   typedef typename DelegateList::const_iterator DelegateIterator;
	   DelegateList delegateList;

   public:
	   void Connect( _Delegate delegate )
	   {
		   delegateList.insert( delegate );
	   }

	   template< class X, class Y >
	   void Connect( Y * obj, void (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5 ) )
	   {
		   delegateList.insert( MakeDelegate( obj, func ) );
	   }

	   template< class X, class Y >
	   void Connect( Y * obj, void (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5 ) const )
	   {
		   delegateList.insert( MakeDelegate( obj, func ) );
	   }

	   void Disconnect( _Delegate delegate )
	   {
		   delegateList.erase( delegate );
	   }

	   template< class X, class Y >
	   void Disconnect( Y * obj, void (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5 ) )
	   {
		   delegateList.erase( MakeDelegate( obj, func ) );
	   }

	   template< class X, class Y >
	   void Disconnect( Y * obj, void (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5 ) const )
	   {
		   delegateList.erase( MakeDelegate( obj, func ) );
	   }

	   void Clear()
	   {
		   delegateList.clear();
	   }

	   void Emit( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5 ) const
	   {
		   for (DelegateIterator i = delegateList.begin(); i != delegateList.end(); ++i)
		   {
			   (*i)( p1, p2, p3, p4, p5 );
		   }
	   }

	   void operator() ( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5 ) const
	   {
		   Emit( p1, p2, p3, p4, p5 );
	   }
   };


   template< class Param1, class Param2, class Param3, class Param4, class Param5, class Param6 >
   class vaSignal6
   {
   public:
	   typedef vaDelegate6< Param1, Param2, Param3, Param4, Param5, Param6 > _Delegate;

   private:
	   typedef std::set<_Delegate> DelegateList;
	   typedef typename DelegateList::const_iterator DelegateIterator;
	   DelegateList delegateList;

   public:
	   void Connect( _Delegate delegate )
	   {
		   delegateList.insert( delegate );
	   }

	   template< class X, class Y >
	   void Connect( Y * obj, void (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6 ) )
	   {
		   delegateList.insert( MakeDelegate( obj, func ) );
	   }

	   template< class X, class Y >
	   void Connect( Y * obj, void (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6 ) const )
	   {
		   delegateList.insert( MakeDelegate( obj, func ) );
	   }

	   void Disconnect( _Delegate delegate )
	   {
		   delegateList.erase( delegate );
	   }

	   template< class X, class Y >
	   void Disconnect( Y * obj, void (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6 ) )
	   {
		   delegateList.erase( MakeDelegate( obj, func ) );
	   }

	   template< class X, class Y >
	   void Disconnect( Y * obj, void (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6 ) const )
	   {
		   delegateList.erase( MakeDelegate( obj, func ) );
	   }

	   void Clear()
	   {
		   delegateList.clear();
	   }

	   void Emit( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6 ) const
	   {
		   for (DelegateIterator i = delegateList.begin(); i != delegateList.end(); ++i)
		   {
			   (*i)( p1, p2, p3, p4, p5, p6 );
		   }
	   }

	   void operator() ( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6 ) const
	   {
		   Emit( p1, p2, p3, p4, p5, p6 );
	   }
   };


   template< class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7 >
   class vaSignal7
   {
   public:
	   typedef vaDelegate7< Param1, Param2, Param3, Param4, Param5, Param6, Param7 > _Delegate;

   private:
	   typedef std::set<_Delegate> DelegateList;
	   typedef typename DelegateList::const_iterator DelegateIterator;
	   DelegateList delegateList;

   public:
	   void Connect( _Delegate delegate )
	   {
		   delegateList.insert( delegate );
	   }

	   template< class X, class Y >
	   void Connect( Y * obj, void (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7 ) )
	   {
		   delegateList.insert( MakeDelegate( obj, func ) );
	   }

	   template< class X, class Y >
	   void Connect( Y * obj, void (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7 ) const )
	   {
		   delegateList.insert( MakeDelegate( obj, func ) );
	   }

	   void Disconnect( _Delegate delegate )
	   {
		   delegateList.erase( delegate );
	   }

	   template< class X, class Y >
	   void Disconnect( Y * obj, void (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7 ) )
	   {
		   delegateList.erase( MakeDelegate( obj, func ) );
	   }

	   template< class X, class Y >
	   void Disconnect( Y * obj, void (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7 ) const )
	   {
		   delegateList.erase( MakeDelegate( obj, func ) );
	   }

	   void Clear()
	   {
		   delegateList.clear();
	   }

	   void Emit( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7 ) const
	   {
		   for (DelegateIterator i = delegateList.begin(); i != delegateList.end(); ++i)
		   {
			   (*i)( p1, p2, p3, p4, p5, p6, p7 );
		   }
	   }

	   void operator() ( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7 ) const
	   {
		   Emit( p1, p2, p3, p4, p5, p6, p7 );
	   }
   };


   template< class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8 >
   class vaSignal8
   {
   public:
	   typedef vaDelegate8< Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8 > _Delegate;

   private:
	   typedef std::set<_Delegate> DelegateList;
	   typedef typename DelegateList::const_iterator DelegateIterator;
	   DelegateList delegateList;

   public:
	   void Connect( _Delegate delegate )
	   {
		   delegateList.insert( delegate );
	   }

	   template< class X, class Y >
	   void Connect( Y * obj, void (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8 ) )
	   {
		   delegateList.insert( MakeDelegate( obj, func ) );
	   }

	   template< class X, class Y >
	   void Connect( Y * obj, void (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8 ) const )
	   {
		   delegateList.insert( MakeDelegate( obj, func ) );
	   }

	   void Disconnect( _Delegate delegate )
	   {
		   delegateList.erase( delegate );
	   }

	   template< class X, class Y >
	   void Disconnect( Y * obj, void (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8 ) )
	   {
		   delegateList.erase( MakeDelegate( obj, func ) );
	   }

	   template< class X, class Y >
	   void Disconnect( Y * obj, void (X::*func)( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8 ) const )
	   {
		   delegateList.erase( MakeDelegate( obj, func ) );
	   }

	   void Clear()
	   {
		   delegateList.clear();
	   }

	   void Emit( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8 ) const
	   {
		   for (DelegateIterator i = delegateList.begin(); i != delegateList.end(); ++i)
		   {
			   (*i)( p1, p2, p3, p4, p5, p6, p7, p8 );
		   }
	   }

	   void operator() ( Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8 ) const
	   {
		   Emit( p1, p2, p3, p4, p5, p6, p7, p8 );
	   }
   };


} // namespace

