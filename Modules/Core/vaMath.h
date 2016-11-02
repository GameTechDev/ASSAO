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

#include "vaCoreTypes.h"

#include <math.h>

#ifndef VA_PI
#define VA_PI        (3.1415926535897932384626433832795)
#endif
#ifndef VA_PIf
#define VA_PIf       (3.1415926535897932384626433832795f)
#endif
#ifndef VA_EPSf
#define VA_EPSf      (1e-6f)
#endif

namespace VertexAsylum
{
    // TODO: most of this is no longer needed as it's part of std::xxx, so clean up the codebase and remove it
    // (leave the stuff that makes sense to stay in for any kind of framework-wide optimization/profiling/tracking purposes)
   class vaMath
   {
   public:

      static inline float     TimeIndependentLerpF(float deltaTime, float lerpRate);

      template<class T>
      static inline T         Min(T const & a, T const & b);

      template<class T>
      static inline T         Max(T const & a, T const & b);

      template<class T>
      static inline T         Clamp(T const & v, T const & min, T const & max);
      
      template<class T>
      static inline T         Saturate( T const & a );

      template<class T>
      static inline T         Lerp(T const & a, T const & b, const float f);

      // why not just use std::swap?
      //template<class T>
      //static inline void      Swap(T & a, T & b);

      static inline float     Abs(float a);
      static inline double    Abs(double a);

      static inline double    Frac( double a );
      static inline float     Frac( float a );
      static inline int       FloorLog2(unsigned int n); 

      static inline float     AngleWrap( float angle );
      static inline float     AngleSmallestDiff( float a, float b );

      static inline float     DegreeToRadian( float degree )            { return degree * VA_PIf / 180.0f; }
      static inline float     RadianToDegree( float radian )            { return radian * 180.0f / VA_PIf; }

      static inline bool      IsPowOf2(int val);
      static inline int       PowOf2Ceil(int val);
      static inline int       Log2(int val);

      template<class T>
      static inline T         Sq(const T & a);

      static inline float     Sqrt( float a )                           { return ::sqrtf( a ); }
      static inline double    Sqrt( double a )                          { return ::sqrt( a ); }

      static inline float     Pow( float a, float p )                   { return powf( a, p ); }
      static inline double    Pow( double a, double p )                 { return pow( a, p ); }

      static inline float     Exp( float p )                            { return expf( p ); }
      static inline double    Exp( double p )                           { return exp( p ); }

      static inline float     Sin( float a )                            { return sinf( a ); }
      static inline double    Sin( double a )                           { return sin( a ); }

      static inline float     Cos( float a )                            { return cosf( a ); }
      static inline double    Cos( double a )                           { return cos( a ); }

      static inline float     ASin( float a )                           { return asinf( a ); }
      static inline double    ASin( double a )                          { return asin( a ); }

      static inline float     ACos( float a )                           { return acosf( a ); }
      static inline double    ACos( double a )                          { return acos( a ); }

      static inline float     ATan2( float y, float x )                 { return atan2f( y, x ); }

      static inline float       Round( float x )                            { return ::roundf(x); }
      static inline double      Round( double x )                           { return ::round(x); }

      static inline float       Ceil( float x )                             { return ::ceilf(x); }
      static inline double      Ceil( double x )                            { return ::ceil(x); }

      static inline float       Floor( float x )                            { return ::floorf(x); }
      static inline double      Floor( double x )                           { return ::floor(x); }

      template<class T>
      static inline T         Sign( const T & a );

      static float              Randf( );

      static inline bool      NearEqual( float a, float b, float epsilon = 1e-5f )  { return vaMath::Abs( a - b ) < epsilon; }

      static inline float     Smoothstep( const float t );                                                                        // gives something similar to "sin( (x-0.5) * PI)*0.5 + 0.5 )" for [0, 1]
      
   private:
      friend class vaCore;
      static void             Initialize( );
      static void             Deinitialize( );
   };

   // Time independent lerp function. The bigger the lerpRate, the faster the lerp!
   inline float vaMath::TimeIndependentLerpF(float deltaTime, float lerpRate)
   {
   	return 1.0f - expf( -fabsf(deltaTime*lerpRate) );
   }

   template<class T>
   inline T vaMath::Min(T const & a, T const & b)
   {
      return (a < b)?(a):(b);
   }

   template<class T>
   inline T vaMath::Max(T const & a, T const & b)
   {
      return (a > b)?(a):(b);
   }

   template<class T>
   inline T vaMath::Clamp(T const & v, T const & min, T const & max)
   {
   	if( v < min ) return min;
   	if( v > max ) return max;
   	return v;
   }

   // short for clamp( a, 0, 1 )
   template<class T>
   inline T vaMath::Saturate( T const & a )
   {
   	return Clamp( a, (T)0.0, (T)1.0 );
   }

   template<class T>
   inline T vaMath::Lerp(T const & a, T const & b, const float f)
   {
   	return a + (b-a)*f;
   }

   // why not just use std::swap?
   //template<class T>
   //inline void vaMath::Swap(T & a, T & b)
   //{
   //    assert( &a != &b ); // swapping with itself? 
   //	T temp = b;
   //	b = a;
   //	a = temp;
   //}

   template<class T>
   inline T vaMath::Sq(const T & a)
   {
   	return a * a;
   }

   inline double vaMath::Frac( double a )
   {
   	return fmod( a, 1.0 );
   }

   inline float vaMath::Frac( float a )
   {
   	return fmodf( a, 1.0 );
   }

   inline int vaMath::FloorLog2(unsigned int n) 
   {
      int pos = 0;
      if (n >= 1<<16) { n >>= 16; pos += 16; }
      if (n >= 1<< 8) { n >>=  8; pos +=  8; }
      if (n >= 1<< 4) { n >>=  4; pos +=  4; }
      if (n >= 1<< 2) { n >>=  2; pos +=  2; }
      if (n >= 1<< 1) {           pos +=  1; }
      return ((n == 0) ? (-1) : pos);
   }

   inline float vaMath::AngleWrap( float angle )
   {
      return ( angle > 0 ) ? ( fmodf( angle + VA_PIf, VA_PIf * 2.0f )-VA_PIf ) : ( fmodf( angle - VA_PIf, VA_PIf * 2.0f ) + VA_PIf );
   }

   inline float vaMath::AngleSmallestDiff( float a, float b )
   {
       a = AngleWrap( a );
       b = AngleWrap( b );
       float v = AngleWrap( a - b );
       if( v > VA_PIf )
           v -= VA_PIf * 2.0f;
       return v;
   }

   inline bool vaMath::IsPowOf2(int val) 
   {
      if( val < 1 ) return false;
      return (val & (val-1)) == 0;
   }

   inline int vaMath::PowOf2Ceil(int val)
   {
      int l2 = Log2( Max( 0, val-1 ) ) + 1;
      return 1 << l2;
   }

   inline int vaMath::Log2(int val)
   {
      unsigned r = 0;

      while( val >>= 1 )
      {
         r++;
      }

      return r;
   }

   inline float vaMath::Abs(float a)
   {
      return fabsf( a );
   }

   inline double vaMath::Abs(double a)
   {
      return fabs( a );
   }

   template<class T>
   inline T vaMath::Sign( const T & a )
   {
      if( a > 0 ) return 1;
      if( a < 0 ) return -1;
      return 0;
   }

	// "fat" version:
	// float smoothstep(float edge0, float edge1, float x)
	// {
    //	// Scale, bias and saturate x to 0..1 range
    //	x = clamp((x - edge0)/(edge1 - edge0), 0.0, 1.0); 
    //  // Evaluate polynomial
    //	return x*x*(3 - 2*x);
	//}

   inline float vaMath::Smoothstep( const float t )
   {
       return t * t * ( 3 - 2 * t );
   }


}