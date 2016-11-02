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
   // Sparse array with automatic/manual defragmentation and index tracking
   // (not optimized or tested a lot but seems to work fine so far...)
   template< class ElementType >
   class vaSparseArray
   {
   public:
      struct ElementReference
      {
         ElementType    Value;

      private:
         int *          IndexPtr;
         int            IndexIfIndexPtrUnavailable;

      public:
         ElementReference( )                                         { }
         ElementReference( const ElementType & value, int * indexPtr ) : Value( value ), IndexPtr( indexPtr ) 
                                                                     { Index() = -1; }
         ElementReference( const ElementReference & copy ) : Value( copy.Value ), IndexPtr( copy.IndexPtr ), IndexIfIndexPtrUnavailable( copy.IndexIfIndexPtrUnavailable )
                                                                     { }

         int &          Index()                                      { return (IndexPtr == NULL)?(IndexIfIndexPtrUnavailable):(*IndexPtr); }
      };

   protected:
      typedef std::vector<ElementReference> InternalVectorType;
      InternalVectorType         m_elements;
      const ElementType          m_nullValue;

      int                        m_elementCount;
      int                        m_currentMinZeroedIndex;
      int                        m_numberOfZeroed;

   protected:
      //vaSparseArray( )                           { assert(false); }   // never to be implemented
      vaSparseArray( const vaSparseArray & copy )  { m_nullValue = copy.m_nullValue; assert(false); }    // not yet implemented
      void operator = ( const vaSparseArray & )    { assert( false ); }                                  // not yet implemented
   
   public:
      vaSparseArray( int reserveSize = 0, ElementType nullValue = 0 );
      ~vaSparseArray();

      // if you provide storage for index tracking by indexPtr, you don't have to pay for finding the element when removing it
      // (vaSparseArray will dynamically update the index value pointed to by indexPtr)
      void                       Add( const ElementType & element, int * indexPtr = NULL );
      void                       Remove( int index );

      // only guaranteed to valid until next Defragment()
      int                        FindCurrentIndex( const ElementType & element );

      // number of elements including zeroed elements
      int                        GetCount( ) const                            { return (int)m_elements.size(); }
      // number of elements WITHOUT zeroed elements
      int                        GetNonZeroedCount( ) const                   { return m_elementCount; }

      void                       Clear( );

      // get an element (includes zeroed elements!)
      const ElementType &        GetElementAt( int index ) const              { VA_ASSERT( index >= 0 && index < m_elements.size(), L"Index out of range, this will probably crash" ); return m_elements[index].Value; }
      ElementType &              GetElementAt( int index )                    { VA_ASSERT( index >= 0 && index < m_elements.size(), L"Index out of range, this will probably crash" ); return m_elements[index].Value; }

      void                       Defragment( );
   };

   //////////////////////////////////////////////////////////////////////////
   // Inline
   //////////////////////////////////////////////////////////////////////////


   template< class ElementType >
   inline vaSparseArray<ElementType>::vaSparseArray( int reserveSize = 0, ElementType nullValue = 0 ) : m_nullValue( nullValue ) 
   { 
      m_elements.reserve( reserveSize ); 
      m_elementCount             = 0;
      m_currentMinZeroedIndex    = INT_MAX;
      m_numberOfZeroed           = 0;
   }

   template< class ElementType >
   inline vaSparseArray<ElementType>::~vaSparseArray( )
   {

   }

   template< typename ElementType >
   inline void vaSparseArray<ElementType>::Add( const ElementType & element, int * indexPtr )
   {
      int newElementIndex = (int)m_elements.size();
      m_elements.push_back( ElementReference( element, indexPtr ) );
      if( indexPtr != NULL ) VA_ASSERT( *indexPtr == -1, L"value at indexPtr must be -1 when adding to array (safety feature)" );
      m_elements[ newElementIndex ].Index() = newElementIndex;
   }

   template< class ElementType >
   inline void                       vaSparseArray<ElementType>::Remove( int index )
   {
      VA_ASSERT( index >= 0 && index < m_elements.size(), L"Index out of range, this will probably crash" );
      VA_ASSERT( m_elements[ index ].Index() == index, L"ElementReference broken, this will probably crash" );

      m_elements[index].Value    = NULL;
      m_elements[index].Index()  = -1;

      m_currentMinZeroedIndex = vaMath::Min( m_currentMinZeroedIndex, index );
      m_numberOfZeroed++;
   }

   template< class ElementType >
   inline int vaSparseArray<ElementType>::FindCurrentIndex( const ElementType & element )
   {
      assert( false ); // not tested!!

      for( int i = 0; i < m_elements.size(); i++ )
      {
         if( m_elements[i].Value == element )
            return i;
      }
      return -1;
   }

   template< class ElementType >
   inline void vaSparseArray<ElementType>::Defragment( )
   {
      // defragment and remove NULL elements from the vector, in a quick way
      if( m_numberOfZeroed > 0 )
      {
         int lastNonZero = ((int)m_elements.size())-1;
         while( (lastNonZero >= 0) && (m_elements[lastNonZero].Value == m_nullValue) ) lastNonZero--;
         for( int i = m_currentMinZeroedIndex; (i < lastNonZero) && (m_numberOfZeroed>0); i++ )
         {
            if( m_elements[i].Value == m_nullValue )
            {
               // swap with the last
               m_elements[i] = m_elements[lastNonZero];
               m_elements[lastNonZero].Value = m_nullValue;
               // have to update the index now!
               m_elements[i].Index() = i;
               // optimization
               m_numberOfZeroed--;
               while( (lastNonZero >= 0) && (m_elements[lastNonZero].Value == m_nullValue) ) lastNonZero--;
               if( lastNonZero < 0 )
                  break;
            }
         }
         while( (lastNonZero >= 0) && (m_elements[lastNonZero].Value == m_nullValue) ) lastNonZero--;
         if( lastNonZero < 0 )
         {
            m_elements.clear();
         }
         else
         {
            m_elements.resize( lastNonZero+1 );
         }
         m_currentMinZeroedIndex = INT_MAX;
         m_numberOfZeroed = 0;
      }
   }

   template< class ElementType >
   inline void vaSparseArray<ElementType>::Clear( )
   {
      for( int i = 0; i < m_elements.size(); i++ )
      {
         if( m_elements[i].Value != m_nullValue )
         {
            m_elements[i].Value    = NULL;
            m_elements[i].Index()  = -1;
         }
      }
      m_elements.clear();
      m_elementCount             = 0;
      m_currentMinZeroedIndex    = INT_MAX;
      m_numberOfZeroed           = 0;
   }
}

