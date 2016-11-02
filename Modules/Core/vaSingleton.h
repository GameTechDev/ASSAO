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

#include "vaPlatformBase.h"

#include "vaCoreTypes.h"
#include "vaSTL.h"

// Frequently used includes
#include <assert.h>


namespace VertexAsylum
{
   ////////////////////////////////////////////////////////////////////////////////////////////////
   // Simple base class for a singleton.
   //  - ensures that the class is indeed a singleton
   //  - provides access to it
   //  1.) inherit YourClass from vaSystemManagerSingletonBase<YourClass>
   //  2.) you're responsible for creation/destruction of the object!
   //
   template< class SingletonType >
   class vaSingletonBase
   {
   private:
      static SingletonType *       s_instance;

   protected:
      vaSingletonBase( )
      {
         assert( s_instance == NULL );
         s_instance = static_cast<SingletonType *>(this);
      }
      virtual ~vaSingletonBase() 
      {
         assert( s_instance != NULL );
         s_instance = NULL;
      }

   public:

      static SingletonType &        GetInstance( )      { assert( s_instance != NULL ); return *s_instance; }
      static SingletonType *        GetInstancePtr( )   { return s_instance; }
   };
   //
   template <class SingletonType>
   SingletonType * vaSingletonBase<SingletonType>::s_instance = NULL;
   ////////////////////////////////////////////////////////////////////////////////////////////////
}