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

#include "vaMath.h"

#include "vaRandom.h"

// vaGeometry is designed to match Microsoft's D3DX (old, DX9-gen) vector & matrix library as much as
// possible in functionality and interface, in order to simplify porting of my existing DX9-based codebase.
// However not all the functionality is the same and not all of it is tested so beware. Also, it is
// designed with simplicity and readability as a goal, therefore sometimes sacrificing performance.
//
// to fill in missing stuff consult http://doxygen.reactos.org/de/d57/dll_2directx_2wine_2d3dx9__36_2math_8c.html
// (ReactOS, Wine)
//
// ! COORDINATE SYSTEM WARNING !
// VertexAsylum uses a coordinate system in which 
//    FRONT is +X, 
//    RIGHT is +Y,
//    UP    is +Z
// This is different from some functions in DirectX (that assume +Y is up, +Z is forward, +X right). So far, 
// this is only relevant for functions such as various RollPitchYaw, etc., which should be made more generic anyway.
// ! COORDINATE SYSTEM WARNING !
//
// Few of the functions use code/ides from http://clb.demon.fi/MathGeoLib

namespace VertexAsylum
{
    class vaVector4;
    class vaMatrix4x4;
    class vaMatrix3x3;

    struct vaViewport
    {
        int   X;
        int   Y;
        int   Width;
        int   Height;
        float MinDepth;
        float MaxDepth;

        vaViewport( ) : X( 0 ), Y( 0 ), Width( 0 ), Height( 0 ), MinDepth( 0 ), MaxDepth( 1.0f ) { }

        vaViewport( int width, int height ) : X( 0 ), Y( 0 ), Width( width ), Height( height ), MinDepth( 0 ), MaxDepth( 1.0f ) { }
        vaViewport( int x, int y, int width, int height ) : X( x ), Y( y ), Width( width ), Height( height ), MinDepth( 0 ), MaxDepth( 1.0f ) { }
    };

    class vaVector2
    {
    public:
        float x, y;

    public:
        vaVector2( ) { };
        vaVector2( const float * p ) { assert( p != NULL ); x = p[0]; y = p[1]; }
        vaVector2( float x, float y ) : x( x ), y( y ) { }
        explicit vaVector2( const class vaVector2i & );

        // assignment operators
        vaVector2 &   operator += ( const vaVector2 & );
        vaVector2 &   operator -= ( const vaVector2 & );
        vaVector2 &   operator *= ( float );
        vaVector2 &   operator /= ( float );

        // unary operators
        vaVector2     operator + ( ) const;
        vaVector2     operator - ( ) const;

        // binary operators
        vaVector2     operator + ( const vaVector2 & ) const;
        vaVector2     operator - ( const vaVector2 & ) const;
        vaVector2     operator * ( float ) const;
        vaVector2     operator / ( float ) const;

        friend vaVector2 operator * ( float, const class vaVector2 & );

        bool operator == ( const vaVector2 & ) const;
        bool operator != ( const vaVector2 & ) const;

        float                 Length( ) const;
        float                 LengthSq( ) const;
        vaVector2             Normalize( ) const;      // this should be called NormalizeD or something similar to indicate it returning value
        vaVector2             ComponentAbs( ) const;

    public:
        static float          Dot( const vaVector2 & a, const vaVector2 & b );
        static float          Cross( const vaVector2 & a, const vaVector2 & b );
        static bool           CloseEnough( const vaVector2 & a, const vaVector2 & b, float epsilon = VA_EPSf );

        static vaVector2      ComponentMul( const vaVector2 & a, const vaVector2 & b );
        static vaVector2      ComponentDiv( const vaVector2 & a, const vaVector2 & b );
        static vaVector2      ComponentMin( const vaVector2 & a, const vaVector2 & b );
        static vaVector2      ComponentMax( const vaVector2 & a, const vaVector2 & b );

        static vaVector2      BaryCentric( const vaVector2 & v1, const vaVector2 & v2, const vaVector2 & v3, float f, float g );

        // Hermite interpolation between position v1, tangent t1 (when s == 0) and position v2, tangent t2 (when s == 1).
        static vaVector2      Hermite( const vaVector2 & v1, const vaVector2 & t1, const vaVector2 & v2, const vaVector2 & t2, float s );

        // CatmullRom interpolation between v1 (when s == 0) and v2 (when s == 1)
        static vaVector2      CatmullRom( const vaVector2 & v0, const vaVector2 & v1, const vaVector2 & v2, const vaVector2 & v3, float s );

        // Transform (x, y, 0, 1) by matrix.
        static vaVector4      Transform( const vaVector2 & v, const vaMatrix4x4 & mat );

        // Transform (x, y, 0, 1) by matrix, project result back into w=1.
        static vaVector2      TransformCoord( const vaVector2 & v, const vaMatrix4x4 & mat );

        // Transform (x, y, 0, 0) by matrix.
        static vaVector2      TransformNormal( const vaVector2 & v, const vaMatrix4x4 & mat );

        // Random point on circle with radius 1.0
        static vaVector2      RandomPointOnCircle( vaRandom & randomGeneratorToUse = vaRandom::Singleton );

        // Random point on or within a circle of radius 1.0 (on the disk)
        static vaVector2      RandomPointOnDisk( vaRandom & randomGeneratorToUse = vaRandom::Singleton );
    };



    //--------------------------
    // 3D Vector
    //--------------------------
    class vaVector3
    {
    public:
        float x, y, z;

    public:
        vaVector3( ) { };
        vaVector3( const float * p ) { assert( p != NULL ); x = p[0]; y = p[1]; z = p[2]; }
        vaVector3( float x, float y, float z ) : x( x ), y( y ), z( z ) { }
        vaVector3( const vaVector2 & a, float z ) : x( a.x ), y( a.y ), z( z ) { }

        const float & operator [] ( int i ) const       { assert( i >= 0 && i < 3 ); return (&x)[i]; }
        float & operator [] ( int i )                   { assert( i >= 0 && i < 3 ); return (&x)[i]; }

        // assignment operators
        vaVector3& operator += ( const vaVector3 & );
        vaVector3& operator -= ( const vaVector3 & );
        vaVector3& operator *= ( float );
        vaVector3& operator /= ( float );

        // unary operators
        vaVector3 operator + ( ) const;
        vaVector3 operator - ( ) const;

        // binary operators
        vaVector3 operator + ( const vaVector3 & ) const;
        vaVector3 operator - ( const vaVector3 & ) const;
        vaVector3 operator * ( const vaVector3 & ) const;
        vaVector3 operator / ( const vaVector3 & ) const;

        vaVector3 operator * ( float ) const;
        vaVector3 operator / ( float ) const;
        vaVector3 operator + ( float ) const;
        vaVector3 operator - ( float ) const;

        friend vaVector3 operator * ( float, const class vaVector3 & );

        bool operator == ( const vaVector3 & ) const;
        bool operator != ( const vaVector3 & ) const;

        float                   Length( ) const;
        float                   LengthSq( ) const;
        vaVector3               Normalize( ) const;      // this should be called NormalizeD or something similar to indicate it returning value
        vaVector3               ComponentAbs( ) const;
        bool                    IsNormal( float epsilon = VA_EPSf );

        vaVector2 &             AsVec2( ) { return *( (vaVector2*)( &x ) ); }
        const vaVector2 &       AsVec2( ) const { return *( (vaVector2*)( &x ) ); }

    public:
        static float            Dot( const vaVector3 & a, const vaVector3 & b );
        static vaVector3        Cross( const vaVector3 & a, const vaVector3 & b );
        static bool             CloseEnough( const vaVector3 & a, const vaVector3 & b, float epsilon = VA_EPSf );

        static vaVector3        ComponentMul( const vaVector3 & a, const vaVector3 & b );
        static vaVector3        ComponentDiv( const vaVector3 & a, const vaVector3 & b );
        static vaVector3        ComponentMin( const vaVector3 & a, const vaVector3 & b );
        static vaVector3        ComponentMax( const vaVector3 & a, const vaVector3 & b );

        static vaVector3        BaryCentric( const vaVector3 & v1, const vaVector3 & v2, const vaVector3 & v3, float f, float g );

        // Hermite interpolation between position v1, tangent t1 (when s == 0) and position v2, tangent t2 (when s == 1).
        static vaVector3        Hermite( const vaVector3 & v1, const vaVector3 & t1, const vaVector3 & v2, const vaVector3 &t2, float s );

        // CatmullRom interpolation between v1 (when s == 0) and v2 (when s == 1)
        static vaVector3        CatmullRom( const vaVector3 &v0, const vaVector3 &v1, const vaVector3 & v2, const vaVector3 & v3, float s );

        // Transform (x, y, z, 1) by matrix.
        static vaVector4        Transform( const vaVector3 & v, const vaMatrix4x4 & mat );

        // Transform (x, y, z, 1) by matrix, project result back into w=1.
        static vaVector3        TransformCoord( const vaVector3 & v, const vaMatrix4x4 & mat );

        // Transform (x, y, z, 0) by matrix.  If you are transforming a normal by a 
        // non-affine matrix, the matrix you pass to this function should be the 
        // transpose of the inverse of the matrix you would use to transform a coord.
        static vaVector3        TransformNormal( const vaVector3 & v, const vaMatrix4x4 & mat );

        // Same as above, just with a 3x3 matrix.
        static vaVector3        TransformNormal( const vaVector3 & v, const vaMatrix3x3 & mat );

        static vaVector3        Random( vaRandom & randomGeneratorToUse = vaRandom::Singleton );

        static vaVector3        RandomNormal( vaRandom & randomGeneratorToUse = vaRandom::Singleton );
        static vaVector3        RandomPointOnSphere( vaRandom & randomGeneratorToUse = vaRandom::Singleton ) { return RandomNormal( randomGeneratorToUse ); }

        static float            AngleBetweenVectors( const vaVector3 & a, const vaVector3 & b );

        // Project vector from object space into screen space
        static vaVector3        Project( const vaVector3 & v, const vaViewport & viewport,
            const vaMatrix4x4 & projection, const vaMatrix4x4 & view, const vaMatrix4x4 & world );

        // Project vector from screen space into object space
        static vaVector3        Unproject( const vaVector3 & v, const vaViewport & viewport,
            const vaMatrix4x4 & projection, const vaMatrix4x4 & view, const vaMatrix4x4 & world );
    };


    class vaVector4
    {
    public:
        float x, y, z, w;

    public:
        vaVector4( ) { };
        vaVector4( const float * p ) { assert( p != NULL ); x = p[0]; y = p[1]; z = p[2]; w = p[3]; }
        vaVector4( float x, float y, float z, float w ) : x( x ), y( y ), z( z ), w( w ) { };
        vaVector4( const vaVector3 & v, float w ) : x( v.x ), y( v.y ), z( v.z ), w( w ) { };
        explicit vaVector4( const class vaVector4i & );

        // assignment operators
        vaVector4 & operator += ( const vaVector4 & );
        vaVector4 & operator -= ( const vaVector4 & );
        vaVector4 & operator *= ( float );
        vaVector4 & operator /= ( float );

        // unary operators
        vaVector4 operator + ( ) const;
        vaVector4 operator - ( ) const;

        // binary operators
        vaVector4 operator + ( const vaVector4 & ) const;
        vaVector4 operator - ( const vaVector4 & ) const;
        vaVector4 operator * ( const vaVector4 & ) const;
        vaVector4 operator / ( const vaVector4 & ) const;
        vaVector4 operator + ( float ) const;
        vaVector4 operator - ( float ) const;
        vaVector4 operator * ( float ) const;
        vaVector4 operator / ( float ) const;

        friend vaVector4 operator * ( float, const vaVector4 & );

        bool operator == ( const vaVector4 & ) const;
        bool operator != ( const vaVector4 & ) const;

        float                Length( ) const;
        float                LengthSq( ) const;
        vaVector4            Normalize( ) const;      // this should be called NormalizeD or something similar to indicate it returning value

        vaVector3 &          AsVec3( ) { return *( (vaVector3*)( &x ) ); }
        const vaVector3 &    AsVec3( ) const { return *( (vaVector3*)( &x ) ); }

        vaVector2 &          AsVec2( ) { return *( (vaVector2*)( &x ) ); }
        const vaVector2 &    AsVec2( ) const { return *( (vaVector2*)( &x ) ); }

    public:
        static float          Dot( const vaVector4 & a, const vaVector4 & b );
        static vaVector4      Cross( const vaVector4 & a, const vaVector4 & b, const vaVector4 & c );

        inline vaVector4      ComponentMul( const vaVector4 & a, const vaVector4 & b );
        inline vaVector4      ComponentDiv( const vaVector4 & a, const vaVector4 & b );
        static vaVector4      ComponentMin( const vaVector4 & a, const vaVector4 & b );
        static vaVector4      ComponentMax( const vaVector4 & a, const vaVector4 & b );

        static vaVector4      BaryCentric( const vaVector4 & v1, const vaVector4 & v2, const vaVector4 & v3, float f, float g );

        static vaVector4      Random( vaRandom & randomGeneratorToUse = vaRandom::Singleton );

        // Hermite interpolation between position v1, tangent t1 (when s == 0) and position v2, tangent t2 (when s == 1).
        static vaVector4      Hermite( const vaVector4 & v1, const vaVector4 &t1, const vaVector4 &v2, const vaVector4 &t2, float s );

        // CatmullRom interpolation between V1 (when s == 0) and V2 (when s == 1)
        static vaVector4      CatmullRom( const vaVector4 & v0, const vaVector4 & v1, const vaVector4 & v2, const vaVector4 & v3, float s );

        // Transform vector by matrix.
        static vaVector4      Transform( const vaVector4 & v, const vaMatrix4x4 & mat );

        static vaVector4      FromBGRA( uint32 colour );
        static vaVector4      FromRGBA( uint32 colour );
        static vaVector4      FromABGR( uint32 colour );

        static uint32         ToBGRA( const vaVector4 & colour );
        static uint32         ToRGBA( const vaVector4 & colour );
        static uint32         ToABGR( const vaVector4 & colour );

        inline static uint32  ToRGBA( float r, float g, float b, float a ) { return ToRGBA( vaVector4( r, g, b, a ) ); }
        inline static uint32  ToABGR( float r, float g, float b, float a ) { return ToABGR( vaVector4( r, g, b, a ) ); }
    };


    class vaQuaternion
    {
    public:
        float x, y, z, w;

    public:
        static vaQuaternion        Identity;

    public:
        vaQuaternion( ) { }
        explicit vaQuaternion( const float * p ) { assert( p != NULL ); x = p[0]; y = p[1]; z = p[2]; w = p[3]; }
        vaQuaternion( float x, float y, float z, float w ) : x( x ), y( y ), z( z ), w( w ) { }
        explicit vaQuaternion( const vaVector4 & v ) : x( v.x ), y( v.y ), z( v.z ), w( v.w ) { }

        // assignment
        vaQuaternion & operator += ( const vaQuaternion & );
        vaQuaternion & operator -= ( const vaQuaternion & );
        vaQuaternion & operator *= ( const vaQuaternion & );
        vaQuaternion & operator *= ( float );
        vaQuaternion & operator /= ( float );

        // unary
        vaQuaternion  operator + ( ) const;
        vaQuaternion  operator - ( ) const;

        // binary
        vaQuaternion operator + ( const vaQuaternion & ) const;
        vaQuaternion operator - ( const vaQuaternion & ) const;
        vaQuaternion operator * ( const vaQuaternion & ) const;
        vaQuaternion operator * ( float ) const;
        vaQuaternion operator / ( float ) const;

        friend vaQuaternion operator * ( float, const vaQuaternion & );

        bool operator == ( const vaQuaternion & ) const;
        bool operator != ( const vaQuaternion & ) const;


        float                Length( ) const;
        float                LengthSq( ) const;
        vaQuaternion         Conjugate( ) const;
        void                 ToAxisAngle( vaVector3 & outAxis, float & outAngle ) const;
        vaQuaternion         Normalize( ) const;      // this should be called NormalizeD or something similar to indicate it returning value
        vaQuaternion         Inverse( ) const;

        // Expects unit quaternions.
        vaQuaternion         Ln( ) const;

        // Expects pure quaternions. (w == 0)  w is ignored in calculation.
        vaQuaternion         Exp( ) const;

        // Could also be called DecomposeEuler( float & zAngle, float & yAngle, float & xAngle );
        void                 DecomposeYawPitchRoll( float & yaw, float & pitch, float & roll ) const;

        // VA convention: X is forward
        vaVector3             GetAxisX( ) const;

        // VA convention: Y is right
        vaVector3             GetAxisY( ) const;

        // VA convention: Z is up
        vaVector3             GetAxisZ( ) const;

    public:
        static float         Dot( const vaQuaternion & a, const vaQuaternion & b );

        // Quaternion multiplication. The result represents the rotation b followed by the rotation a.
        static vaQuaternion  Multiply( const vaQuaternion & a, const vaQuaternion & b );

        // Build quaternion from rotation matrix.
        static vaQuaternion  FromRotationMatrix( const class vaMatrix4x4 & mat );

        // Build quaternion from axis and angle.
        static vaQuaternion  RotationAxis( const vaVector3 & v, float angle );

        // Yaw around the +Z (up) axis, a pitch around the +Y (right) axis, and a roll around the +X (forward) axis.
        static vaQuaternion  RotationYawPitchRoll( float yaw, float pitch, float roll );

        // Spherical linear interpolation between Q1 (t == 0) and Q2 (t == 1).
        // Expects unit quaternions.
        static vaQuaternion  Slerp( const vaQuaternion & q1, const vaQuaternion & q2, float t );

        // Spherical quadrangle interpolation.
        static vaQuaternion  Squad( const vaQuaternion & q1, const vaQuaternion & q2, const vaQuaternion & q3, const vaQuaternion & q4, float t );

        // Barycentric interpolation.
        // Slerp(Slerp(Q1, Q2, f+g), Slerp(Q1, Q3, f+g), g/(f+g))
        static vaQuaternion  BaryCentric( const vaQuaternion & q1, const vaQuaternion & q2, const vaQuaternion & q3, float f, float g );
    };

    class vaPlane
    {
    public:
        union
        {
            struct
            {
                float        a, b, c, d;
            };
            struct
            {
                vaVector3    normal;
            };
        };

    public:
        vaPlane( ) { }
        vaPlane( const float * p ) { assert( p != NULL ); a = p[0]; b = p[1]; c = p[2]; d = p[3]; }
        vaPlane( float a, float b, float c, float d ) : a( a ), b( b ), c( c ), d( d ) { }
        explicit vaPlane( const vaVector4 & v ) : a( v.x ), b( v.y ), c( v.z ), d( v.w ) { }

        // assignment operators
        vaPlane& operator *= ( float );
        vaPlane& operator /= ( float );

        // unary operators
        vaPlane operator + ( ) const;
        vaPlane operator - ( ) const;

        // binary operators
        vaPlane operator * ( float ) const;
        vaPlane operator / ( float ) const;

        friend vaPlane operator * ( float, const vaPlane& );

        bool operator == ( const vaPlane& ) const;
        bool operator != ( const vaPlane& ) const;

        vaPlane               PlaneNormalize( ) const;

        bool                  IntersectLine( vaVector3 & outPt, const vaVector3 & lineStart, const vaVector3 & lineEnd );
        bool                  IntersectRay( vaVector3 & outPt, const vaVector3 & lineStart, const vaVector3 & direction );

    public:
        static float          Dot( const vaPlane & plane, const vaVector4 & v );
        static float          DotCoord( const vaPlane & plane, const vaVector3 & v );
        static float          DotNormal( const vaPlane & plane, const vaVector3 & v );

        static vaPlane        FromPointNormal( const vaVector3 & point, const vaVector3 & normal );
        static vaPlane        FromPoints( const vaVector3 & v1, const vaVector3 & v2, const vaVector3 & v3 );

        // Transform a plane by a matrix.  The vector (a,b,c) must be normalized. M should be the inverse transpose of the transformation desired.
        vaPlane               Transform( const vaPlane & p, const vaMatrix4x4 & mat );
    };

    // this one is unfinished
    class vaMatrix3x3
    {
    public:
        union
        {
            struct
            {
                float       _11, _12, _13;
                float       _21, _22, _23;
                float       _31, _32, _33;
            };
            struct
            {
                vaVector3  r0;
                vaVector3  r1;
                vaVector3  r2;
            };
            float m[3][3];
        };

    public:
        static vaMatrix3x3      Identity;

    public:
        vaMatrix3x3( ) { };
        vaMatrix3x3( float _11, float _12, float _13,
            float _21, float _22, float _23,
            float _31, float _32, float _33 );
        vaMatrix3x3( const vaVector3 & axisX, const vaVector3 & axisY, const vaVector3 & axisZ ) { r0 = axisX; r1 = axisY; r2 = axisZ; }
        explicit vaMatrix3x3( const float * );
        explicit vaMatrix3x3( const vaMatrix4x4 & t );

        // access grants
        float& operator () ( uint32 row, uint32 col ) { return m[row][col]; }
        float  operator () ( uint32 row, uint32 col ) const { return m[row][col]; }

        // Build a matrix from quaternion
        static vaMatrix3x3      FromQuaternion( const vaQuaternion & q );

        vaMatrix3x3             Transpose( ) const;
    };

    class vaMatrix4x4
    {
    public:
        union
        {
            struct
            {
                float       _11, _12, _13, _14;
                float       _21, _22, _23, _24;
                float       _31, _32, _33, _34;
                float       _41, _42, _43, _44;
            };
            struct
            {
                vaVector4  r0;
                vaVector4  r1;
                vaVector4  r2;
                vaVector4  r3;
            };
            float m[4][4];
        };

    public:
        static vaMatrix4x4      Identity;

    public:
        vaMatrix4x4( ) { };
        explicit vaMatrix4x4( const float * );
        vaMatrix4x4( float _11, float _12, float _13, float _14,
            float _21, float _22, float _23, float _24,
            float _31, float _32, float _33, float _34,
            float _41, float _42, float _43, float _44 );
        explicit vaMatrix4x4( const vaMatrix3x3 & rm ) { m[0][0] = rm.m[0][0]; m[0][1] = rm.m[0][1]; m[0][2] = rm.m[0][2]; m[1][0] = rm.m[1][0], m[1][1] = rm.m[1][1], m[1][2] = rm.m[1][2], m[2][0] = rm.m[2][0], m[2][1] = rm.m[2][1], m[2][2] = rm.m[2][2]; m[0][3] = 0.0f; m[1][3] = 0.0f; m[2][3] = 0.0f; m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f; };

        // access grants
        float& operator () ( uint32 row, uint32 col ) { return m[row][col]; }
        float  operator () ( uint32 row, uint32 col ) const { return m[row][col]; }

        // assignment operators
        vaMatrix4x4 & operator *= ( const vaMatrix4x4 & mat );
        vaMatrix4x4 & operator += ( const vaMatrix4x4 & mat );
        vaMatrix4x4 & operator -= ( const vaMatrix4x4 & mat );
        vaMatrix4x4 & operator *= ( float );
        vaMatrix4x4 & operator /= ( float );

        // unary operators
        vaMatrix4x4 operator + ( ) const { return *this; }
        vaMatrix4x4 operator - ( ) const { return vaMatrix4x4( -_11, -_12, -_13, -_14, -_21, -_22, -_23, -_24, -_31, -_32, -_33, -_34, -_41, -_42, -_43, -_44 ); }

        // binary operators
        vaMatrix4x4 operator * ( const vaMatrix4x4 & ) const;
        vaMatrix4x4 operator + ( const vaMatrix4x4 & ) const;
        vaMatrix4x4 operator - ( const vaMatrix4x4 & ) const;
        vaMatrix4x4 operator * ( float ) const;
        vaMatrix4x4 operator / ( float ) const;

        friend vaMatrix4x4 operator * ( float, const vaMatrix4x4& );

        bool operator == ( const vaMatrix4x4& ) const;
        bool operator != ( const vaMatrix4x4& ) const;

        float                   Determinant( ) const;
        vaMatrix4x4             Transpose( ) const;
        bool                    Decompose( vaVector3 & outScale, vaQuaternion & outRotation, vaVector3 & outTranslation ) const;

        // Could also be called DecomposeRotationEuler( float & zAngle, float & yAngle, float & xAngle );
        void                    DecomposeRotationYawPitchRoll( float & yaw, float & pitch, float & roll );

        bool                    Inverse( vaMatrix4x4 & outMat, float * outDeterminant = NULL ) const;
        vaMatrix4x4             Inverse( float * outDeterminant = NULL ) const { vaMatrix4x4 ret; bool res = Inverse( ret, outDeterminant ); assert( res ); if( !res ) return vaMatrix4x4::Identity; else return ret; }

        vaVector3               GetRotationX( ) const { return vaVector3( m[0][0], m[0][1], m[0][2] ); }
        vaVector3               GetRotationY( ) const { return vaVector3( m[1][0], m[1][1], m[1][2] ); }
        vaVector3               GetRotationZ( ) const { return vaVector3( m[2][0], m[2][1], m[2][2] ); }

        vaMatrix3x3             GetRotationMatrix3x3( ) const { return vaMatrix3x3( m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2] ); }

        // VA convention: X is forward
        vaVector3               GetAxisX( ) const { return vaVector3( m[0][0], m[0][1], m[0][2] ); }

        // VA convention: Y is right
        vaVector3               GetAxisY( ) const { return vaVector3( m[1][0], m[1][1], m[1][2] ); }

        // VA convention: Z is up
        vaVector3               GetAxisZ( ) const { return vaVector3( m[2][0], m[2][1], m[2][2] ); }

        vaVector3               GetTranslation( ) const { return vaVector3( m[3][0], m[3][1], m[3][2] ); }

        void                    SetRotation( const vaMatrix3x3 & rm ) { m[0][0] = rm.m[0][0]; m[0][1] = rm.m[0][1]; m[0][2] = rm.m[0][2]; m[1][0] = rm.m[1][0], m[1][1] = rm.m[1][1], m[1][2] = rm.m[1][2], m[2][0] = rm.m[2][0], m[2][1] = rm.m[2][1], m[2][2] = rm.m[2][2]; m[0][3] = 0.0f; m[1][3] = 0.0f; m[2][3] = 0.0f; }
        void                    SetTranslation( const vaVector3 & vec ) { m[3][0] = vec.x; m[3][1] = vec.y; m[3][2] = vec.z; m[3][3] = 1.0f; }

    public:

        // Matrix multiplication.  The result represents transformation b followed by transformation a.
        static vaMatrix4x4      Multiply( const vaMatrix4x4 & a, const vaMatrix4x4 & b );

        // Build scaling matrix
        static vaMatrix4x4      Scaling( float sx, float sy, float sz );
        static vaMatrix4x4      Scaling( const vaVector3 & vec ) { return Scaling( vec.x, vec.y, vec.z ); }

        // Build translation matrix 
        static vaMatrix4x4      Translation( float x, float y, float z );
        static vaMatrix4x4      Translation( const vaVector3 & vec ) { return Translation( vec.x, vec.y, vec.z ); }

        // Build rotation matrix around X
        static vaMatrix4x4      RotationX( float angle );

        // Build a matrix which rotates around the Y axis
        static vaMatrix4x4      RotationY( float angle );

        // Build a matrix which rotates around the Z axis
        static vaMatrix4x4      RotationZ( float angle );

        // Build a matrix which rotates around an arbitrary axis
        static vaMatrix4x4      RotationAxis( const vaVector3 & vec, float angle );

        // Yaw around the +Z (up) axis, a pitch around the +Y (right) axis, and a roll around the +X (forward) axis.
        static vaMatrix4x4      RotationYawPitchRoll( float yaw, float pitch, float roll );

        // Build transformation matrix.  NULL arguments are treated as identity.
        // Mout = Msc-1 * Msr-1 * Ms * Msr * Msc * Mrc-1 * Mr * Mrc * Mt
        static vaMatrix4x4      Transformation( const vaVector3 * pScalingCenter,
            const vaQuaternion * pScalingRotation, const vaVector3 * pScaling,
            const vaVector3 * pRotationCenter, const vaQuaternion * pRotation,
            const vaVector3 * pTranslation );

        // Build 2D transformation matrix in XY plane.  NULL arguments are treated as identity.
        // Mout = Msc-1 * Msr-1 * Ms * Msr * Msc * Mrc-1 * Mr * Mrc * Mt
        static vaMatrix4x4      Transformation2D( const vaVector2 * pScalingCenter,
            float scalingRotation, const vaVector2 * pScaling,
            const vaVector2 * pRotationCenter, float rotation,
            const vaVector2 * pTranslation );

        // Build affine transformation matrix.  NULL arguments are treated as identity.
        // Mout = Ms * Mrc-1 * Mr * Mrc * Mt
        static vaMatrix4x4      AffineTransformation( float Scaling, const vaVector3 * pRotationCenter, const vaQuaternion * pRotation, const vaVector3 * pTranslation );

        // Build 2D affine transformation matrix in XY plane.  NULL arguments are treated as identity.
        // Mout = Ms * Mrc-1 * Mr * Mrc * Mt
        static vaMatrix4x4      AffineTransformation2D( float Scaling, const vaVector2 * pRotationCenter, float Rotation, const vaVector2 * pTranslation );

        // Build a lookat matrix. (right-handed)
        static vaMatrix4x4      LookAtRH( const vaVector3 & eye, const vaVector3 & at, const vaVector3 & up );

        // Build a lookat matrix. (left-handed)
        static vaMatrix4x4      LookAtLH( const vaVector3 & eye, const vaVector3 & at, const vaVector3 & up );

        // Build a perspective projection matrix. (right-handed)
        static vaMatrix4x4      PerspectiveRH( float w, float h, float zn, float zf );

        // Build a perspective projection matrix. (left-handed)
        static vaMatrix4x4      PerspectiveLH( float w, float h, float zn, float zf );

        // Build a perspective projection matrix. (right-handed)
        static vaMatrix4x4      PerspectiveFovRH( float fovy, float Aspect, float zn, float zf );

        // Build a perspective projection matrix. (left-handed)
        static vaMatrix4x4      PerspectiveFovLH( float fovy, float Aspect, float zn, float zf );

        // Build a perspective projection matrix. (right-handed)
        static vaMatrix4x4      PerspectiveOffCenterRH( float l, float r, float b, float t, float zn, float zf );

        // Build a perspective projection matrix. (left-handed)
        static vaMatrix4x4      PerspectiveOffCenterLH( float l, float r, float b, float t, float zn, float zf );

        // Build an ortho projection matrix. (right-handed)
        static vaMatrix4x4      OrthoRH( float w, float h, float zn, float zf );

        // Build an ortho projection matrix. (left-handed)
        static vaMatrix4x4      OrthoLH( float w, float h, float zn, float zf );

        // Build an ortho projection matrix. (right-handed)
        static vaMatrix4x4      OrthoOffCenterRH( float l, float r, float b, float t, float zn, float zf );

        // Build an ortho projection matrix. (left-handed)
        static vaMatrix4x4      OrthoOffCenterLH( float l, float r, float b, float t, float zn, float zf );

        // Build a matrix which flattens geometry into a plane, as if casting a shadow from a light.
        static vaMatrix4x4      Shadow( const vaVector4 & light, const vaPlane & plane );

        // Build a matrix which reflects the coordinate system about a plane
        static vaMatrix4x4      Reflect( const vaPlane & plane );

        // Build a matrix from quaternion
        static vaMatrix4x4      FromQuaternion( const vaQuaternion & q );

        // Build a matrix from rotation & translation, in two flavors
        static vaMatrix4x4      FromRotationTranslation( const vaQuaternion & rot, const vaVector3 & trans );
        static vaMatrix4x4      FromRotationTranslation( const vaMatrix3x3 & rot, const vaVector3 & trans );

        static bool             NearEqual( const vaMatrix4x4 & a, const vaMatrix4x4 & b, float epsilon = 1e-4f );

    };

    class vaRay3D
    {
    public:
        vaVector3                         Origin;
        vaVector3                         Direction;

    public:
        vaRay3D( ) {};

        inline vaVector3                  GetPointAt( float dist ) const;

        static inline vaRay3D             FromTwoPoints( const vaVector3 & p1, const vaVector3 & p2 );
        static inline vaRay3D             FromOriginAndDirection( const vaVector3 & origin, const vaVector3 & direction );
    };

    class vaBoundingSphere
    {
    public:
        vaVector3                        Center;
        float                            Radius;

    public:
        vaBoundingSphere( ) { }
        vaBoundingSphere( const vaVector3 & center, float radius ) : Center( center ), Radius( radius ) { }

        vaVector3                       RandomPointOnSurface( vaRandom & randomGeneratorToUse = vaRandom::Singleton ) { return vaVector3::RandomNormal( ) * Radius + Center; }
        vaVector3                       RandomPointInside( vaRandom & randomGeneratorToUse = vaRandom::Singleton ) { return vaVector3::RandomNormal( ) * ( vaMath::Pow( randomGeneratorToUse.NextFloat( ), 1.0f / 3.0f ) * Radius ) + Center; }
    };

    class vaBoundingBox
    {
    public:
        vaVector3                        Min;
        vaVector3                        Size;

        static vaBoundingBox             Degenerate;

    public:
        vaBoundingBox( ) { };
        vaBoundingBox( const vaVector3 & bmin, const vaVector3 & bsize ) : Min( bmin ), Size( bsize ) { }

        vaVector3                       Center( ) const { return Min + Size * 0.5f; }
        vaVector3                       Max( ) const { return Min + Size; }

        static vaBoundingBox            Combine( const vaBoundingBox & a, const vaBoundingBox & b );

    };

    class vaOrientedBoundingBox
    {
    public:
        vaVector3                        Center;
        vaVector3                        Extents;    // aka half-size
        vaMatrix3x3                      Axis;

        static vaOrientedBoundingBox     Degenerate;

    public:
        vaOrientedBoundingBox( ) { }
        vaOrientedBoundingBox( const vaVector3 & center, const vaVector3 & halfSize, const vaMatrix3x3 & axis ) : Center( center ), Extents( halfSize ), Axis( axis ) { }
        vaOrientedBoundingBox( const vaBoundingBox & box, const vaMatrix4x4 & transform ) { *this = FromAABBAndTransform( box, transform ); }

        vaVector3                        Min( ) const { return Center - Extents; }
        vaVector3                        Max( ) const { return Center + Extents; }

        static vaOrientedBoundingBox    FromAABBAndTransform( const vaBoundingBox & box, const vaMatrix4x4 & transform );
        void                            ToAABBAndTransform( vaBoundingBox & outBox, vaMatrix4x4 & outTransform ) const;

        // 0 means intersect, -1 means it's wholly in the negative halfspace of the plane, 1 means it's in the positive half-space of the plane
        int                             IntersectPlane( const vaPlane & plane );

        // 0 means intersect any, -1 means it's wholly outside, 1 means it's wholly outside
        bool                            IntersectFrustum( const vaPlane planes[], const int planeCount );

        vaVector3                       RandomPointInside( vaRandom & randomGeneratorToUse = vaRandom::Singleton );

        // supports only affine transformations
        static vaOrientedBoundingBox    Transform( const vaOrientedBoundingBox & obb, const vaMatrix4x4 & mat );
    };

    class vaVector2i
    {
    public:
        int x, y;

    public:
        vaVector2i( ) { };
        vaVector2i( int x, int y ) : x( x ), y( y ) { };
        explicit vaVector2i( const vaVector2 & v ) : x( (int)v.x ), y( (int)v.y ) { };

        // assignment operators
        vaVector2i &    operator += ( const vaVector2i & );
        vaVector2i &    operator -= ( const vaVector2i & );

        // unary operators
        vaVector2i      operator + ( ) const;
        vaVector2i      operator - ( ) const;

        // binary operators
        vaVector2i      operator + ( const vaVector2i & ) const;
        vaVector2i      operator - ( const vaVector2i & ) const;
        bool            operator == ( const vaVector2i & ) const;
        bool            operator != ( const vaVector2i & ) const;

        // cast operators
        explicit operator vaVector2 ( ) { return vaVector2( (float)x, (float)y ); }
    };

    class vaVector4i
    {
    public:
        int x, y, z, w;

    public:
        vaVector4i( ) { };
        vaVector4i( int x, int y, int z, int w ) : x( x ), y( y ), z( z ), w( w ) { };
        explicit vaVector4i( const vaVector4 & v ) : x( (int)v.x ), y( (int)v.y ), z( (int)v.z ), w( (int)v.w ) { };

        // assignment operators
        vaVector4i &    operator += ( const vaVector4i & );
        vaVector4i &    operator -= ( const vaVector4i & );

        // unary operators
        vaVector4i      operator + ( ) const;
        vaVector4i      operator - ( ) const;

        // binary operators
        vaVector4i      operator + ( const vaVector4i & ) const;
        vaVector4i      operator - ( const vaVector4i & ) const;
        bool            operator == ( const vaVector4i & eq ) const                    { return this->x == eq.x && this->y == eq.y && this->z == eq.z && this->w == eq.w; }
        bool            operator != ( const vaVector4i & eq ) const                    { return this->x != eq.x || this->y != eq.y || this->z != eq.z || this->w != eq.w; }

        // cast operators
        explicit operator vaVector4 ( ) { return vaVector4( (float)x, (float)y, (float)z, (float)w ); }
    };

    class vaVector4ui
    {
    public:
        uint32 x, y, z, w;

    public:
        vaVector4ui( ) { };
        vaVector4ui( uint32 x, uint32 y, uint32 z, uint32 w ) : x( x ), y( y ), z( z ), w( w ) { };

        // assignment operators
        vaVector4ui &   operator += ( const vaVector4ui & );
        vaVector4ui &   operator -= ( const vaVector4ui & );

        // binary operators
        vaVector4ui     operator + ( const vaVector4ui & ) const;
        vaVector4ui     operator - ( const vaVector4ui & ) const;
        bool            operator == ( const vaVector4i & eq ) const                    { return this->x == eq.x && this->y == eq.y && this->z == eq.z && this->w == eq.w; }
        bool            operator != ( const vaVector4i & eq ) const                    { return this->x != eq.x || this->y != eq.y || this->z != eq.z || this->w != eq.w; }
    };

    enum class vaWindingOrder : int32
    {
        None,            // todo: remove this, can't be none
        Clockwise,
        CounterClockwise
    };

    enum class vaFaceCull : int32
    {
        None,
        Front,
        Back
    };

    class vaGeometry
    {
    public:

        static float            FresnelTerm( float cosTheta, float refractionIndex );

        static void             CalculateFrustumPlanes( vaPlane planes[6], const vaMatrix4x4 & cameraViewProj );

        static inline bool      EqualWithinEps( const vaVector2 & a, const vaVector2 & b, const float fEps );
        static inline bool      EqualWithinEps( const vaVector3 & a, const vaVector3 & b, const float fEps );
        static inline bool      EqualWithinEps( const vaVector4 & a, const vaVector4 & b, const float fEps );

        static inline bool      IntersectSegments2D( const vaVector2 & p1, const vaVector2 & p2, const vaVector2 & p3, const vaVector2 & p4, vaVector2 & outPt );

        static inline vaVector3 WorldToViewportSpace( const vaVector3 & worldPos, const vaMatrix4x4 & viewProj, const vaViewport & viewport );
        static inline vaVector3 ViewportToWorldSpace( const vaVector3 & screenPos, const vaMatrix4x4 & inverseViewProj, const vaViewport & viewport );

    };

#include "vaGeometry.inl"


}
