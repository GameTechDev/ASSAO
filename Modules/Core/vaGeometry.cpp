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

#include "vaGeometry.h"
#include <memory.h>

//for testing
//#include <DirectXMath.h>

using namespace VertexAsylum;


///////////////////////////////////////////////////////////////////////////////////////////////////
// vaMatrix4x4
///////////////////////////////////////////////////////////////////////////////////////////////////

vaMatrix3x3 vaMatrix3x3::Identity = vaMatrix3x3( 1, 0, 0, 
                                                0, 1, 0,
                                                0, 0, 1 );

vaMatrix3x3::vaMatrix3x3( const float * p )
{
    assert( p != NULL );
    memcpy( &_11, p, sizeof( vaMatrix3x3 ) );
}


vaMatrix4x4 vaMatrix4x4::Identity = vaMatrix4x4( 1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1 );

vaMatrix4x4::vaMatrix4x4( const float * p )
{
    assert( p != NULL );
    memcpy( &_11, p, sizeof( vaMatrix4x4 ) );
}

bool vaMatrix4x4::operator == ( const vaMatrix4x4 & mat ) const
{
    return 0 == memcmp( &this->_11, &mat._11, sizeof( vaMatrix4x4 ) );
}

bool vaMatrix4x4::operator != ( const vaMatrix4x4 & mat ) const
{
    return 0 != memcmp( &this->_11, &mat._11, sizeof( vaMatrix4x4 ) );
}

float vaMatrix4x4::Determinant( ) const
{
    vaVector4 minor, v1, v2, v3;
    float det;

    v1.x = m[0][0]; v1.y = m[1][0]; v1.z = m[2][0]; v1.w = m[3][0];
    v2.x = m[0][1]; v2.y = m[1][1]; v2.z = m[2][1]; v2.w = m[3][1];
    v3.x = m[0][2]; v3.y = m[1][2]; v3.z = m[2][2]; v3.w = m[3][2];
    minor = vaVector4::Cross( v1, v2, v3 );
    det = -( m[0][3] * minor.x + m[1][3] * minor.y + m[2][3] * minor.z + m[3][3] * minor.w );

    return det;
}

vaMatrix3x3 vaMatrix3x3::Transpose( ) const
{
    vaMatrix3x3 ret;
    for( int i = 0; i < 3; i++ )
    {
        for( int j = 0; j < 3; j++ )
        {
            ret.m[i][j] = m[j][i];
        }
    }
    return ret;
}

vaMatrix4x4 vaMatrix4x4::Transpose( ) const
{
    vaMatrix4x4 ret;
    for( int i = 0; i < 4; i++ )
    {
        for( int j = 0; j < 4; j++ )
        {
            ret.m[i][j] = m[j][i];
        }
    }
    return ret;
}

bool vaMatrix4x4::Inverse( vaMatrix4x4 & outMat, float * outDeterminant ) const
{
    float det = Determinant( );

    if( vaMath::Abs( det ) < VA_EPSf )
        return false;

    if( outDeterminant != NULL )
        *outDeterminant = det;

    int a;
    vaVector4 v, vec[3];

    for( int i = 0; i < 4; i++ )
    {
        for( int j = 0; j < 4; j++ )
        {
            if( j != i )
            {
                a = j;
                if( j > i ) a = a - 1;
                vec[a].x = m[j][0];
                vec[a].y = m[j][1];
                vec[a].z = m[j][2];
                vec[a].w = m[j][3];
            }
        }
        v = vaVector4::Cross( vec[0], vec[1], vec[2] );

        outMat.m[0][i] = vaMath::Pow( -1.0f, (float)i ) * v.x / det;
        outMat.m[1][i] = vaMath::Pow( -1.0f, (float)i ) * v.y / det;
        outMat.m[2][i] = vaMath::Pow( -1.0f, (float)i ) * v.z / det;
        outMat.m[3][i] = vaMath::Pow( -1.0f, (float)i ) * v.w / det;
    }

    return true;
}

bool vaMatrix4x4::Decompose( vaVector3 & outScale, vaQuaternion & outRotation, vaVector3 & outTranslation ) const
{
    vaMatrix4x4 normalized;
    vaVector3 vec;

    // Scaling 
    vec.x = this->m[0][0];
    vec.y = this->m[0][1];
    vec.z = this->m[0][2];
    outScale.x = vec.Length( );

    vec.x = this->m[1][0];
    vec.y = this->m[1][1];
    vec.z = this->m[1][2];
    outScale.y = vec.Length( );

    vec.x = this->m[2][0];
    vec.y = this->m[2][1];
    vec.z = this->m[2][2];
    outScale.z = vec.Length( );

    // Translation
    outTranslation.x = this->m[3][0];
    outTranslation.y = this->m[3][1];
    outTranslation.z = this->m[3][2];

    /*Let's calculate the rotation now*/
    if( ( outScale.x == 0.0f ) || ( outScale.y == 0.0f ) || ( outScale.z == 0.0f ) ) return false;

    normalized.m[0][0] = this->m[0][0] / outScale.x;
    normalized.m[0][1] = this->m[0][1] / outScale.x;
    normalized.m[0][2] = this->m[0][2] / outScale.x;
    normalized.m[1][0] = this->m[1][0] / outScale.y;
    normalized.m[1][1] = this->m[1][1] / outScale.y;
    normalized.m[1][2] = this->m[1][2] / outScale.y;
    normalized.m[2][0] = this->m[2][0] / outScale.z;
    normalized.m[2][1] = this->m[2][1] / outScale.z;
    normalized.m[2][2] = this->m[2][2] / outScale.z;

    outRotation = vaQuaternion::FromRotationMatrix( normalized );

    return true;
}

void vaMatrix4x4::DecomposeRotationYawPitchRoll( float & yaw, float & pitch, float & roll )
{
    //pitch = (float)vaMath::ASin( -m[2][1] ); 
    pitch = (float)vaMath::ASin( -m[0][2] );

    float threshold = 0.001f;

    float test = (float)vaMath::Cos( pitch );

    if( test > threshold )
    {

        //roll = (float)vaMath::ATan2( m[0][1], m[1][1] ); 
        roll = (float)vaMath::ATan2( m[1][2], m[2][2] );

        //yaw = (float)vaMath::ATan2( m[2][0], m[2][2] );
        yaw = (float)vaMath::ATan2( m[0][1], m[0][0] );

    }
    else
    {
        //roll = (float)vaMath::ATan2( -m[1][0], m[0][0] ); 
        roll = (float)vaMath::ATan2( -m[2][1], m[1][1] );
        yaw = 0.0f;
    }
}

// static

vaMatrix4x4 vaMatrix4x4::Multiply( const vaMatrix4x4 & a, const vaMatrix4x4 & b )
{
    vaMatrix4x4 ret;

    for( int i = 0; i < 4; i++ )
    {
        for( int j = 0; j < 4; j++ )
        {
            ret.m[i][j] = a.m[i][0] * b.m[0][j] + a.m[i][1] * b.m[1][j] + a.m[i][2] * b.m[2][j] + a.m[i][3] * b.m[3][j];
        }
    }

    return ret;
}

vaMatrix4x4 vaMatrix4x4::Scaling( float sx, float sy, float sz )
{
    vaMatrix4x4 ret = vaMatrix4x4::Identity;
    ret.m[0][0] = sx;
    ret.m[1][1] = sy;
    ret.m[2][2] = sz;
    return ret;
}

vaMatrix4x4 vaMatrix4x4::Translation( float x, float y, float z )
{
    vaMatrix4x4 ret = vaMatrix4x4::Identity;
    ret.m[3][0] = x;
    ret.m[3][1] = y;
    ret.m[3][2] = z;
    return ret;
}

vaMatrix4x4 vaMatrix4x4::RotationX( float angle )
{
    vaMatrix4x4 ret = vaMatrix4x4::Identity;
    ret.m[1][1] = vaMath::Cos( angle );
    ret.m[2][2] = vaMath::Cos( angle );
    ret.m[1][2] = vaMath::Sin( angle );
    ret.m[2][1] = -vaMath::Sin( angle );
    return ret;
}

vaMatrix4x4 vaMatrix4x4::RotationY( float angle )
{
    vaMatrix4x4 ret = vaMatrix4x4::Identity;
    ret.m[0][0] = vaMath::Cos( angle );
    ret.m[2][2] = vaMath::Cos( angle );
    ret.m[0][2] = -vaMath::Sin( angle );
    ret.m[2][0] = vaMath::Sin( angle );
    return ret;
}

vaMatrix4x4 vaMatrix4x4::RotationZ( float angle )
{
    vaMatrix4x4 ret = vaMatrix4x4::Identity;
    ret.m[0][0] = vaMath::Cos( angle );
    ret.m[1][1] = vaMath::Cos( angle );
    ret.m[0][1] = vaMath::Sin( angle );
    ret.m[1][0] = -vaMath::Sin( angle );
    return ret;
}

vaMatrix4x4 vaMatrix4x4::RotationAxis( const vaVector3 & vec, float angle )
{
    vaMatrix4x4 ret = vaMatrix4x4::Identity;
    vaVector3 v = vec.Normalize( );

    ret.m[0][0] = ( 1.0f - vaMath::Cos( angle ) ) * v.x * v.x + vaMath::Cos( angle );
    ret.m[1][0] = ( 1.0f - vaMath::Cos( angle ) ) * v.x * v.y - vaMath::Sin( angle ) * v.z;
    ret.m[2][0] = ( 1.0f - vaMath::Cos( angle ) ) * v.x * v.z + vaMath::Sin( angle ) * v.y;
    ret.m[0][1] = ( 1.0f - vaMath::Cos( angle ) ) * v.y * v.x + vaMath::Sin( angle ) * v.z;
    ret.m[1][1] = ( 1.0f - vaMath::Cos( angle ) ) * v.y * v.y + vaMath::Cos( angle );
    ret.m[2][1] = ( 1.0f - vaMath::Cos( angle ) ) * v.y * v.z - vaMath::Sin( angle ) * v.x;
    ret.m[0][2] = ( 1.0f - vaMath::Cos( angle ) ) * v.z * v.x - vaMath::Sin( angle ) * v.y;
    ret.m[1][2] = ( 1.0f - vaMath::Cos( angle ) ) * v.z * v.y + vaMath::Sin( angle ) * v.x;
    ret.m[2][2] = ( 1.0f - vaMath::Cos( angle ) ) * v.z * v.z + vaMath::Cos( angle );

    return ret;
}

vaMatrix4x4 vaMatrix4x4::FromQuaternion( const vaQuaternion & q )
{
    vaMatrix4x4 ret = vaMatrix4x4::Identity;
    ret.m[0][0] = 1.0f - 2.0f * ( q.y * q.y + q.z * q.z );
    ret.m[0][1] = 2.0f * ( q.x *q.y + q.z * q.w );
    ret.m[0][2] = 2.0f * ( q.x * q.z - q.y * q.w );
    ret.m[1][0] = 2.0f * ( q.x * q.y - q.z * q.w );
    ret.m[1][1] = 1.0f - 2.0f * ( q.x * q.x + q.z * q.z );
    ret.m[1][2] = 2.0f * ( q.y *q.z + q.x *q.w );
    ret.m[2][0] = 2.0f * ( q.x * q.z + q.y * q.w );
    ret.m[2][1] = 2.0f * ( q.y *q.z - q.x *q.w );
    ret.m[2][2] = 1.0f - 2.0f * ( q.x * q.x + q.y * q.y );
    return ret;
}


vaMatrix4x4 vaMatrix4x4::RotationYawPitchRoll( float yaw, float pitch, float roll )
{
    vaMatrix4x4 ret = vaMatrix4x4::Identity;

    const FLOAT    Sx = sinf( -roll );
    const FLOAT    Sy = sinf( -pitch );
    const FLOAT    Sz = sinf( -yaw );
    const FLOAT    Cx = cosf( -roll );
    const FLOAT    Cy = cosf( -pitch );
    const FLOAT    Cz = cosf( -yaw );
    
    //case ORDER_XYZ:
        ret.m[0][0] = Cy*Cz;
        ret.m[0][1] = -Cy*Sz;
        ret.m[0][2] = Sy;
        ret.m[1][0] = Cz*Sx*Sy + Cx*Sz;
        ret.m[1][1] = Cx*Cz - Sx*Sy*Sz;
        ret.m[1][2] = -Cy*Sx;
        ret.m[2][0] = -Cx*Cz*Sy + Sx*Sz;
        ret.m[2][1] = Cz*Sx + Cx*Sy*Sz;
        ret.m[2][2] = Cx*Cy;
    //    break;
    //
    // //case ORDER_YZX:
    //     ret.m[0][0] = Cy*Cz;
    //     ret.m[0][1] = Sx*Sy - Cx*Cy*Sz;
    //     ret.m[0][2] = Cx*Sy + Cy*Sx*Sz;
    //     ret.m[1][0] = Sz;
    //     ret.m[1][1] = Cx*Cz;
    //     ret.m[1][2] = -Cz*Sx;
    //     ret.m[2][0] = -Cz*Sy;
    //     ret.m[2][1] = Cy*Sx + Cx*Sy*Sz;
    //     ret.m[2][2] = Cx*Cy - Sx*Sy*Sz;
    // //    break;
    // //
    // //case ORDER_ZXY:
    //     ret.m[0][0] = Cy*Cz - Sx*Sy*Sz;
    //     ret.m[0][1] = -Cx*Sz;
    //     ret.m[0][2] = Cz*Sy + Cy*Sx*Sz;
    //     ret.m[1][0] = Cz*Sx*Sy + Cy*Sz;
    //     ret.m[1][1] = Cx*Cz;
    //     ret.m[1][2] = -Cy*Cz*Sx + Sy*Sz;
    //     ret.m[2][0] = -Cx*Sy;
    //     ret.m[2][1] = Sx;
    //     ret.m[2][2] = Cx*Cy;
    // //    break;
    // //
    // //case ORDER_ZYX:
    //     ret.m[0][0] = Cy*Cz;
    //     ret.m[0][1] = Cz*Sx*Sy - Cx*Sz;
    //     ret.m[0][2] = Cx*Cz*Sy + Sx*Sz;
    //     ret.m[1][0] = Cy*Sz;
    //     ret.m[1][1] = Cx*Cz + Sx*Sy*Sz;
    //     ret.m[1][2] = -Cz*Sx + Cx*Sy*Sz;
    //     ret.m[2][0] = -Sy;
    //     ret.m[2][1] = Cy*Sx;
    //     ret.m[2][2] = Cx*Cy;
    // //    break;
    // //
    // //case ORDER_YXZ:
    //     ret.m[0][0] = Cy*Cz + Sx*Sy*Sz;
    //     ret.m[0][1] = Cz*Sx*Sy - Cy*Sz;
    //     ret.m[0][2] = Cx*Sy;
    //     ret.m[1][0] = Cx*Sz;
    //     ret.m[1][1] = Cx*Cz;
    //     ret.m[1][2] = -Sx;
    //     ret.m[2][0] = -Cz*Sy + Cy*Sx*Sz;
    //     ret.m[2][1] = Cy*Cz*Sx + Sy*Sz;
    //     ret.m[2][2] = Cx*Cy;
    // //    break;
    // //
    // //case ORDER_XZY:
    //     ret.m[0][0] = Cy*Cz;
    //     ret.m[0][1] = -Sz;
    //     ret.m[0][2] = Cz*Sy;
    //     ret.m[1][0] = Sx*Sy + Cx*Cy*Sz;
    //     ret.m[1][1] = Cx*Cz;
    //     ret.m[1][2] = -Cy*Sx + Cx*Sy*Sz;
    //     ret.m[2][0] = -Cx*Sy + Cy*Sx*Sz;
    //     ret.m[2][1] = Cz*Sx;
    //     ret.m[2][2] = Cx*Cy + Sx*Sy*Sz;
    // //    break;

    return ret;
}


vaMatrix4x4 vaMatrix4x4::Transformation( const vaVector3 * pScalingCenter,
    const vaQuaternion * pScalingRotation, const vaVector3 * pScaling,
    const vaVector3 * pRotationCenter, const vaQuaternion * pRotation,
    const vaVector3 * pTranslation )
{
    vaMatrix4x4 ret;
    vaMatrix4x4 m1, m2, m3, m4, m5, m6, m7;
    vaQuaternion prc;
    vaVector3 psc, pt;

    if( pScalingCenter == NULL )
    {
        psc.x = 0.0f;
        psc.y = 0.0f;
        psc.z = 0.0f;
    }
    else
    {
        psc.x = pScalingCenter->x;
        psc.y = pScalingCenter->y;
        psc.z = pScalingCenter->z;
    }

    if( pRotationCenter == NULL )
    {
        prc.x = 0.0f;
        prc.y = 0.0f;
        prc.z = 0.0f;
    }
    else
    {
        prc.x = pRotationCenter->x;
        prc.y = pRotationCenter->y;
        prc.z = pRotationCenter->z;
    }

    if( pTranslation == NULL )
    {
        pt.x = 0.0f;
        pt.y = 0.0f;
        pt.z = 0.0f;
    }
    else
    {
        pt.x = pTranslation->x;
        pt.y = pTranslation->y;
        pt.z = pTranslation->z;
    }

    m1 = vaMatrix4x4::Translation( -psc.x, -psc.y, -psc.z );

    if( pScalingRotation == NULL )
    {
        m2 = vaMatrix4x4::Identity;
        m4 = vaMatrix4x4::Identity;
    }
    else
    {
        m4 = vaMatrix4x4::FromQuaternion( *pScalingRotation );
        m4.Inverse( m2, NULL );
    }

    if( pScaling == NULL )  m3 = vaMatrix4x4::Identity;
    else m3 = vaMatrix4x4::Scaling( pScaling->x, pScaling->y, pScaling->z );

    if( pRotation == NULL ) m6 = vaMatrix4x4::Identity;
    else m6 = vaMatrix4x4::FromQuaternion( *pRotation );

    m5 = vaMatrix4x4::Translation( psc.x - prc.x, psc.y - prc.y, psc.z - prc.z );
    m7 = vaMatrix4x4::Translation( prc.x + pt.x, prc.y + pt.y, prc.z + pt.z );

    m1 = vaMatrix4x4::Multiply( m1, m2 );
    m1 = vaMatrix4x4::Multiply( m1, m3 );
    m1 = vaMatrix4x4::Multiply( m1, m4 );
    m1 = vaMatrix4x4::Multiply( m1, m5 );
    m1 = vaMatrix4x4::Multiply( m1, m6 );
    ret = vaMatrix4x4::Multiply( m1, m7 );

    return ret;
}

vaMatrix4x4 vaMatrix4x4::Transformation2D( const vaVector2 * pScalingCenter,
    float scalingRotation, const vaVector2 * pScaling,
    const vaVector2 * pRotationCenter, float rotation,
    const vaVector2 * pTranslation )
{
    vaMatrix4x4 ret;
    vaQuaternion rot, sca_rot;
    vaVector3 rot_center, sca, sca_center, trans;

    if( pScalingCenter != NULL )
    {
        sca_center.x = pScalingCenter->x;
        sca_center.y = pScalingCenter->y;
        sca_center.z = 0.0f;
    }
    else
    {
        sca_center.x = 0.0f;
        sca_center.y = 0.0f;
        sca_center.z = 0.0f;
    }

    if( pScaling != NULL )
    {
        sca.x = pScaling->x;
        sca.y = pScaling->y;
        sca.z = 1.0f;
    }
    else
    {
        sca.x = 1.0f;
        sca.y = 1.0f;
        sca.z = 1.0f;
    }

    if( pRotationCenter != NULL )
    {
        rot_center.x = pRotationCenter->x;
        rot_center.y = pRotationCenter->y;
        rot_center.z = 0.0f;
    }
    else
    {
        rot_center.x = 0.0f;
        rot_center.y = 0.0f;
        rot_center.z = 0.0f;
    }

    if( pTranslation != NULL )
    {
        trans.x = pTranslation->x;
        trans.y = pTranslation->y;
        trans.z = 0.0f;
    }
    else
    {
        trans.x = 0.0f;
        trans.y = 0.0f;
        trans.z = 0.0f;
    }

    rot.w = vaMath::Cos( rotation / 2.0f );
    rot.x = 0.0f;
    rot.y = 0.0f;
    rot.z = vaMath::Sin( rotation / 2.0f );

    sca_rot.w = vaMath::Cos( scalingRotation / 2.0f );
    sca_rot.x = 0.0f;
    sca_rot.y = 0.0f;
    sca_rot.z = vaMath::Sin( scalingRotation / 2.0f );

    ret = vaMatrix4x4::Transformation( &sca_center, &sca_rot, &sca, &rot_center, &rot, &trans );

    return ret;
}

vaMatrix4x4 vaMatrix4x4::AffineTransformation( float scaling, const vaVector3 * pRotationCenter, const vaQuaternion * pRotation, const vaVector3 * pTranslation )
{
    vaMatrix4x4 m1, m2, m3, m4, m5;

    vaMatrix4x4::Scaling( scaling, scaling, scaling );

    if( pRotationCenter != NULL )
    {
        m2 = vaMatrix4x4::Identity;
        m4 = vaMatrix4x4::Identity;
    }
    else
    {
        m2 = vaMatrix4x4::Translation( -pRotationCenter->x, -pRotationCenter->y, -pRotationCenter->z );
        m4 = vaMatrix4x4::Translation( pRotationCenter->x, pRotationCenter->y, pRotationCenter->z );
    }

    if( !pRotation )
        m3 = vaMatrix4x4::Identity;
    else
        m3 = vaMatrix4x4::FromQuaternion( *pRotation );

    if( !pTranslation )
        m5 = vaMatrix4x4::Identity;
    else
        m5 = vaMatrix4x4::Translation( pTranslation->x, pTranslation->y, pTranslation->z );

    m1 = vaMatrix4x4::Multiply( m1, m2 );
    m1 = vaMatrix4x4::Multiply( m1, m3 );
    m1 = vaMatrix4x4::Multiply( m1, m4 );

    return vaMatrix4x4::Multiply( m1, m5 );
}

vaMatrix4x4 vaMatrix4x4::AffineTransformation2D( float scaling, const vaVector2 * pRotationCenter, float rotation, const vaVector2 * pTranslation )
{
    vaMatrix4x4 m1, m2, m3, m4, m5;
    vaQuaternion rot;
    vaVector3 rot_center, trans;

    rot.w = vaMath::Cos( rotation / 2.0f );
    rot.x = 0.0f;
    rot.y = 0.0f;
    rot.z = vaMath::Sin( rotation / 2.0f );

    if( pRotationCenter )
    {
        rot_center.x = pRotationCenter->x;
        rot_center.y = pRotationCenter->y;
        rot_center.z = 0.0f;
    }
    else
    {
        rot_center.x = 0.0f;
        rot_center.y = 0.0f;
        rot_center.z = 0.0f;
    }

    if( pTranslation )
    {
        trans.x = pTranslation->x;
        trans.y = pTranslation->y;
        trans.z = 0.0f;
    }
    else
    {
        trans.x = 0.0f;
        trans.y = 0.0f;
        trans.z = 0.0f;
    }

    m1 = vaMatrix4x4::Scaling( scaling, scaling, 1.0f );
    m2 = vaMatrix4x4::Translation( -rot_center.x, -rot_center.y, -rot_center.z );
    m4 = vaMatrix4x4::Translation( rot_center.x, rot_center.y, rot_center.z );
    m3 = vaMatrix4x4::FromQuaternion( rot );
    m5 = vaMatrix4x4::Translation( trans.x, trans.y, trans.z );

    m1 = vaMatrix4x4::Multiply( m1, m2 );
    m1 = vaMatrix4x4::Multiply( m1, m3 );
    m1 = vaMatrix4x4::Multiply( m1, m4 );
    return vaMatrix4x4::Multiply( m1, m5 );
}

vaMatrix4x4 vaMatrix4x4::LookAtRH( const vaVector3 & inEye, const vaVector3 & inAt, const vaVector3 & inUp )
{
    vaMatrix4x4 ret;
    vaVector3 right, rightn, up, upn, vec, vec2;
    vec2 = inAt - inEye;
    vec = vec2.Normalize( );
    right = vaVector3::Cross( inUp, vec );
    up = vaVector3::Cross( vec, right );
    rightn = right.Normalize( );
    upn = up.Normalize( );
    ret.m[0][0] = -rightn.x;
    ret.m[1][0] = -rightn.y;
    ret.m[2][0] = -rightn.z;
    ret.m[3][0] = vaVector3::Dot( rightn, inEye );
    ret.m[0][1] = upn.x;
    ret.m[1][1] = upn.y;
    ret.m[2][1] = upn.z;
    ret.m[3][1] = -vaVector3::Dot( upn, inEye );
    ret.m[0][2] = -vec.x;
    ret.m[1][2] = -vec.y;
    ret.m[2][2] = -vec.z;
    ret.m[3][2] = vaVector3::Dot( vec, inEye );
    ret.m[0][3] = 0.0f;
    ret.m[1][3] = 0.0f;
    ret.m[2][3] = 0.0f;
    ret.m[3][3] = 1.0f;
    return ret;
}

//using namespace DirectX;

vaMatrix4x4 vaMatrix4x4::LookAtLH( const vaVector3 & inEye, const vaVector3 & inAt, const vaVector3 & inUp )
{
    //    FXMVECTOR EyePosition   = XMLoadFloat3( &XMFLOAT3( &inEye.x ) );
    //    FXMVECTOR FocusPosition = XMLoadFloat3( &XMFLOAT3( &inAt.x ) );
    //    FXMVECTOR UpDirection   = XMLoadFloat3( &XMFLOAT3( &inUp.x ) );
    //    XMMATRIX    mm = XMMatrixLookAtLH( EyePosition, FocusPosition, UpDirection );
    //    XMFLOAT4X4 mmo;
    //    XMStoreFloat4x4( &mmo, mm );

    vaMatrix4x4 ret;
    vaVector3 right, up, vec;
    vec = ( inAt - inEye ).Normalize( );
    right = vaVector3::Cross( inUp, vec ).Normalize( );
    up = vaVector3::Cross( vec, right ).Normalize( );
    ret.m[0][0] = right.x;
    ret.m[1][0] = right.y;
    ret.m[2][0] = right.z;
    ret.m[3][0] = -vaVector3::Dot( right, inEye );
    ret.m[0][1] = up.x;
    ret.m[1][1] = up.y;
    ret.m[2][1] = up.z;
    ret.m[3][1] = -vaVector3::Dot( up, inEye );
    ret.m[0][2] = vec.x;
    ret.m[1][2] = vec.y;
    ret.m[2][2] = vec.z;
    ret.m[3][2] = -vaVector3::Dot( vec, inEye );
    ret.m[0][3] = 0.0f;
    ret.m[1][3] = 0.0f;
    ret.m[2][3] = 0.0f;
    ret.m[3][3] = 1.0f;
    return ret;
}

vaMatrix4x4 vaMatrix4x4::PerspectiveRH( float w, float h, float zn, float zf )
{
    vaMatrix4x4 ret = vaMatrix4x4::Identity;
    ret.m[0][0] = 2.0f * zn / w;
    ret.m[1][1] = 2.0f * zn / h;
    ret.m[2][2] = zf / ( zn - zf );
    ret.m[3][2] = ( zn * zf ) / ( zn - zf );
    ret.m[2][3] = -1.0f;
    ret.m[3][3] = 0.0f;
    return ret;
}

vaMatrix4x4 vaMatrix4x4::PerspectiveLH( float w, float h, float zn, float zf )
{
    vaMatrix4x4 ret = vaMatrix4x4::Identity;
    ret.m[0][0] = 2.0f * zn / w;
    ret.m[1][1] = 2.0f * zn / h;
    ret.m[2][2] = zf / ( zf - zn );
    ret.m[3][2] = ( zn * zf ) / ( zn - zf );
    ret.m[2][3] = 1.0f;
    ret.m[3][3] = 0.0f;
    return ret;
}

vaMatrix4x4 vaMatrix4x4::PerspectiveFovRH( float fovy, float aspect, float zn, float zf )
{
    vaMatrix4x4 ret = vaMatrix4x4::Identity;
    ret.m[0][0] = 1.0f / ( aspect * tan( fovy / 2.0f ) );
    ret.m[1][1] = 1.0f / tan( fovy / 2.0f );
    ret.m[2][2] = zf / ( zn - zf );
    ret.m[2][3] = -1.0f;
    ret.m[3][2] = ( zf * zn ) / ( zn - zf );
    ret.m[3][3] = 0.0f;
    return ret;
}

vaMatrix4x4 vaMatrix4x4::PerspectiveFovLH( float fovy, float aspect, float zn, float zf )
{
    vaMatrix4x4 ret = vaMatrix4x4::Identity;
    ret.m[0][0] = 1.0f / ( aspect * tan( fovy / 2.0f ) );
    ret.m[1][1] = 1.0f / tan( fovy / 2.0f );
    ret.m[2][2] = zf / ( zf - zn );
    ret.m[2][3] = 1.0f;
    ret.m[3][2] = ( zf * zn ) / ( zn - zf );
    ret.m[3][3] = 0.0f;
    return ret;

    // note:
    // to extract near far planes from matrix returned from this function use
    //  float clipNear1 = -mb / ma;
    //  float clipFar1 = ( ma * clipNear1 ) / ( ma - 1.0f );
    // where ma = m[2][2] and mb = m[3][2]
    // but beware of float precision issues!
}

vaMatrix4x4 vaMatrix4x4::PerspectiveOffCenterRH( float l, float r, float b, float t, float zn, float zf )
{
    vaMatrix4x4 ret = vaMatrix4x4::Identity;
    ret.m[0][0] = 2.0f * zn / ( r - l );
    ret.m[1][1] = -2.0f * zn / ( b - t );
    ret.m[2][0] = 1.0f + 2.0f * l / ( r - l );
    ret.m[2][1] = -1.0f - 2.0f * t / ( b - t );
    ret.m[2][2] = zf / ( zn - zf );
    ret.m[3][2] = ( zn * zf ) / ( zn - zf );
    ret.m[2][3] = -1.0f;
    ret.m[3][3] = 0.0f;
    return ret;
}

// Build a perspective projection matrix. (left-handed)
vaMatrix4x4 vaMatrix4x4::PerspectiveOffCenterLH( float l, float r, float b, float t, float zn, float zf )
{
    vaMatrix4x4 ret = vaMatrix4x4::Identity;
    ret.m[0][0] = 2.0f * zn / ( r - l );
    ret.m[1][1] = -2.0f * zn / ( b - t );
    ret.m[2][0] = -1.0f - 2.0f * l / ( r - l );
    ret.m[2][1] = 1.0f + 2.0f * t / ( b - t );
    ret.m[2][2] = -zf / ( zn - zf );
    ret.m[3][2] = ( zn * zf ) / ( zn - zf );
    ret.m[2][3] = 1.0f;
    ret.m[3][3] = 0.0f;
    return ret;
}

vaMatrix4x4 vaMatrix4x4::OrthoRH( float w, float h, float zn, float zf )
{
    vaMatrix4x4 ret = vaMatrix4x4::Identity;
    ret.m[0][0] = 2.0f / w;
    ret.m[1][1] = 2.0f / h;
    ret.m[2][2] = 1.0f / ( zn - zf );
    ret.m[3][2] = zn / ( zn - zf );
    return ret;
}

vaMatrix4x4 vaMatrix4x4::OrthoLH( float w, float h, float zn, float zf )
{
    vaMatrix4x4 ret = vaMatrix4x4::Identity;
    ret.m[0][0] = 2.0f / w;
    ret.m[1][1] = 2.0f / h;
    ret.m[2][2] = 1.0f / ( zf - zn );
    ret.m[3][2] = zn / ( zn - zf );
    return ret;
}

vaMatrix4x4 vaMatrix4x4::OrthoOffCenterRH( float l, float r, float b, float t, float zn, float zf )
{
    vaMatrix4x4 ret = vaMatrix4x4::Identity;
    ret.m[0][0] = 2.0f / ( r - l );
    ret.m[1][1] = 2.0f / ( t - b );
    ret.m[2][2] = 1.0f / ( zf - zn );
    ret.m[3][0] = -1.0f - 2.0f *l / ( r - l );
    ret.m[3][1] = 1.0f + 2.0f * t / ( b - t );
    ret.m[3][2] = zn / ( zn - zf );
    return ret;
}

vaMatrix4x4 vaMatrix4x4::OrthoOffCenterLH( float l, float r, float b, float t, float zn, float zf )
{
    vaMatrix4x4 ret = vaMatrix4x4::Identity;
    ret.m[0][0] = 2.0f / ( r - l );
    ret.m[1][1] = 2.0f / ( t - b );
    ret.m[2][2] = 1.0f / ( zn - zf );
    ret.m[3][0] = -1.0f - 2.0f *l / ( r - l );
    ret.m[3][1] = 1.0f + 2.0f * t / ( b - t );
    ret.m[3][2] = zn / ( zn - zf );
    return ret;
}

vaMatrix4x4 vaMatrix4x4::Shadow( const vaVector4 & light, const vaPlane & plane )
{
    vaPlane Nplane = plane.PlaneNormalize( );
    float dot = vaPlane::Dot( Nplane, light );

    vaMatrix4x4 ret;
    ret.m[0][0] = dot - Nplane.a * light.x;
    ret.m[0][1] = -Nplane.a * light.y;
    ret.m[0][2] = -Nplane.a * light.z;
    ret.m[0][3] = -Nplane.a * light.w;
    ret.m[1][0] = -Nplane.b * light.x;
    ret.m[1][1] = dot - Nplane.b * light.y;
    ret.m[1][2] = -Nplane.b * light.z;
    ret.m[1][3] = -Nplane.b * light.w;
    ret.m[2][0] = -Nplane.c * light.x;
    ret.m[2][1] = -Nplane.c * light.y;
    ret.m[2][2] = dot - Nplane.c * light.z;
    ret.m[2][3] = -Nplane.c * light.w;
    ret.m[3][0] = -Nplane.d * light.x;
    ret.m[3][1] = -Nplane.d * light.y;
    ret.m[3][2] = -Nplane.d * light.z;
    ret.m[3][3] = dot - Nplane.d * light.w;
    return ret;

}

vaMatrix4x4 vaMatrix4x4::Reflect( const vaPlane & plane )
{
    vaPlane Nplane = plane.PlaneNormalize( );
    vaMatrix4x4 ret = vaMatrix4x4::Identity;
    ret.m[0][0] = 1.0f - 2.0f * Nplane.a * Nplane.a;
    ret.m[0][1] = -2.0f * Nplane.a * Nplane.b;
    ret.m[0][2] = -2.0f * Nplane.a * Nplane.c;
    ret.m[1][0] = -2.0f * Nplane.a * Nplane.b;
    ret.m[1][1] = 1.0f - 2.0f * Nplane.b * Nplane.b;
    ret.m[1][2] = -2.0f * Nplane.b * Nplane.c;
    ret.m[2][0] = -2.0f * Nplane.c * Nplane.a;
    ret.m[2][1] = -2.0f * Nplane.c * Nplane.b;
    ret.m[2][2] = 1.0f - 2.0f * Nplane.c * Nplane.c;
    ret.m[3][0] = -2.0f * Nplane.d * Nplane.a;
    ret.m[3][1] = -2.0f * Nplane.d * Nplane.b;
    ret.m[3][2] = -2.0f * Nplane.d * Nplane.c;
    return ret;
}

bool vaMatrix4x4::NearEqual( const vaMatrix4x4 & a, const vaMatrix4x4 & b, float epsilon )
{
    for( int i = 0; i < 4; i++ )
        for( int j = 0; j < 4; j++ )
            if( !vaMath::NearEqual( a.m[i][j], b.m[i][j], epsilon ) )
                return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// vaQuaternion
///////////////////////////////////////////////////////////////////////////////////////////////////

vaQuaternion vaQuaternion::Identity = vaQuaternion( 0, 0, 0, 1 );

vaQuaternion vaQuaternion::FromRotationMatrix( const vaMatrix4x4 & mat )
{
    vaQuaternion ret;

    int i, maxi;
    float maxdiag, S, trace;

    trace = mat.m[0][0] + mat.m[1][1] + mat.m[2][2] + 1.0f;
    if( trace > 1.0f )
    {
        ret.x = ( mat.m[1][2] - mat.m[2][1] ) / ( 2.0f * sqrt( trace ) );
        ret.y = ( mat.m[2][0] - mat.m[0][2] ) / ( 2.0f * sqrt( trace ) );
        ret.z = ( mat.m[0][1] - mat.m[1][0] ) / ( 2.0f * sqrt( trace ) );
        ret.w = sqrt( trace ) / 2.0f;
        return ret;
    }
    maxi = 0;
    maxdiag = mat.m[0][0];
    for( i = 1; i < 3; i++ )
    {
        if( mat.m[i][i] > maxdiag )
        {
            maxi = i;
            maxdiag = mat.m[i][i];
        }
    }
    switch( maxi )
    {
    case 0:
        S = 2.0f * sqrt( 1.0f + mat.m[0][0] - mat.m[1][1] - mat.m[2][2] );
        ret.x = 0.25f * S;
        ret.y = ( mat.m[0][1] + mat.m[1][0] ) / S;
        ret.z = ( mat.m[0][2] + mat.m[2][0] ) / S;
        ret.w = ( mat.m[1][2] - mat.m[2][1] ) / S;
        break;
    case 1:
        S = 2.0f * sqrt( 1.0f + mat.m[1][1] - mat.m[0][0] - mat.m[2][2] );
        ret.x = ( mat.m[0][1] + mat.m[1][0] ) / S;
        ret.y = 0.25f * S;
        ret.z = ( mat.m[1][2] + mat.m[2][1] ) / S;
        ret.w = ( mat.m[2][0] - mat.m[0][2] ) / S;
        break;
    case 2:
        S = 2.0f * sqrt( 1.0f + mat.m[2][2] - mat.m[0][0] - mat.m[1][1] );
        ret.x = ( mat.m[0][2] + mat.m[2][0] ) / S;
        ret.y = ( mat.m[1][2] + mat.m[2][1] ) / S;
        ret.z = 0.25f * S;
        ret.w = ( mat.m[0][1] - mat.m[1][0] ) / S;
        break;
    }
    return ret;
}

vaQuaternion vaQuaternion::RotationAxis( const vaVector3 & v, float angle )
{
    vaVector3 temp = v.Normalize( );

    vaQuaternion ret;
    float hsin = vaMath::Sin( angle / 2.0f );
    ret.x = hsin * temp.x;
    ret.y = hsin * temp.y;
    ret.z = hsin * temp.z;
    ret.w = vaMath::Cos( angle / 2.0f );
    return ret;
}

vaQuaternion vaQuaternion::RotationYawPitchRoll( float yaw, float pitch, float roll )
{
    vaQuaternion ret;
    ret.x = vaMath::Cos( yaw / 2.0f ) * vaMath::Cos( pitch / 2.0f ) * vaMath::Sin( roll / 2.0f ) - vaMath::Sin( yaw / 2.0f ) * vaMath::Sin( pitch / 2.0f ) * vaMath::Cos( roll / 2.0f );
    ret.y = vaMath::Sin( yaw / 2.0f ) * vaMath::Cos( pitch / 2.0f ) * vaMath::Sin( roll / 2.0f ) + vaMath::Cos( yaw / 2.0f ) * vaMath::Sin( pitch / 2.0f ) * vaMath::Cos( roll / 2.0f );
    ret.z = vaMath::Sin( yaw / 2.0f ) * vaMath::Cos( pitch / 2.0f ) * vaMath::Cos( roll / 2.0f ) - vaMath::Cos( yaw / 2.0f ) * vaMath::Sin( pitch / 2.0f ) * vaMath::Sin( roll / 2.0f );
    ret.w = vaMath::Cos( yaw / 2.0f ) * vaMath::Cos( pitch / 2.0f ) * vaMath::Cos( roll / 2.0f ) + vaMath::Sin( yaw / 2.0f ) * vaMath::Sin( pitch / 2.0f ) * vaMath::Sin( roll / 2.0f );
    return ret;
}

vaQuaternion vaQuaternion::Slerp( const vaQuaternion & q1, const vaQuaternion & q2, float t )
{
    vaQuaternion ret;

    float dot, epsilon, temp, theta, u;

    epsilon = 1.0f;
    temp = 1.0f - t;
    u = t;
    dot = vaQuaternion::Dot( q1, q2 );
    if( dot < 0.0f )
    {
        epsilon = -1.0f;
        dot = -dot;
    }
    if( 1.0f - dot > 0.001f )
    {
        theta = acos( dot );
        temp = sin( theta * temp ) / sin( theta );
        u = sin( theta * u ) / sin( theta );
    }
    ret.x = temp * q1.x + epsilon * u * q2.x;
    ret.y = temp * q1.y + epsilon * u * q2.y;
    ret.z = temp * q1.z + epsilon * u * q2.z;
    ret.w = temp * q1.w + epsilon * u * q2.w;
    return ret;
}

vaQuaternion vaQuaternion::Squad( const vaQuaternion & q1, const vaQuaternion & q2, const vaQuaternion & q3, const vaQuaternion & q4, float t )
{
    vaQuaternion temp1 = vaQuaternion::Slerp( q1, q4, t );
    vaQuaternion temp2 = vaQuaternion::Slerp( q2, q3, t );

    return vaQuaternion::Slerp( temp1, temp2, 2.0f * t * ( 1.0f - t ) );
}

void vaQuaternion::DecomposeYawPitchRoll( float & yaw, float & pitch, float & roll ) const
{
    // should make a quaternion-specific version at some point... 
    vaMatrix4x4 rotMat = vaMatrix4x4::FromQuaternion( *this );
    rotMat.DecomposeRotationYawPitchRoll( yaw, pitch, roll );
    // (the version below is broken somehow)

    /*
    const float Epsilon = 0.0009765625f;
    const float Threshold = 0.5f - Epsilon;

    float XY = this->x * this->y;
    float ZW = this->z * this->w;

    float TEST = XY + ZW;

    if (TEST < -Threshold || TEST > Threshold)
    {
    int sign = (int)vaMath::Sign( TEST );

    yaw = sign * 2 * (float)vaMath::ATan2( this->x, this->w );

    pitch = sign * VA_PIf / 2.0f;;

    roll = 0;

    }
    else
    {

    float XX = this->x * this->x;
    float XZ = this->x * this->z;
    float XW = this->x * this->w;

    float YY = this->y * this->y;
    float YW = this->y * this->w;
    float YZ = this->y * this->z;

    float ZZ = this->z * this->z;

    yaw = (float)vaMath::ATan2(2 * YW - 2 * XZ, 1 - 2 * YY - 2 * ZZ);

    pitch = (float)vaMath::ATan2(2 * XW - 2 * YZ, 1 - 2 * XX - 2 * ZZ);

    roll = (float)vaMath::ASin(2 * TEST);
    }
    */
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// vaPlane
///////////////////////////////////////////////////////////////////////////////////////////////////

bool vaPlane::IntersectLine( vaVector3 & outPt, const vaVector3 & lineStart, const vaVector3 & lineEnd )
{
    vaVector3 direction = lineStart - lineEnd;

    return IntersectRay( outPt, lineStart, lineEnd );
}

bool vaPlane::IntersectRay( vaVector3 & outPt, const vaVector3 & lineStart, const vaVector3 & direction )
{
    vaVector3 normal( a, b, c );
    float dot, temp;

    dot = vaVector3::Dot( normal, direction );
    if( !dot ) return false;

    temp = ( this->d + vaVector3::Dot( normal, lineStart ) ) / dot;
    outPt.x = lineStart.x - temp * direction.x;
    outPt.y = lineStart.y - temp * direction.y;
    outPt.z = lineStart.z - temp * direction.z;

    return true;
}

vaPlane vaPlane::Transform( const vaPlane & plane, const vaMatrix4x4 & mat )
{
    vaPlane ret;
    ret.a = mat.m[0][0] * plane.a + mat.m[1][0] * plane.b + mat.m[2][0] * plane.c + mat.m[3][0] * plane.d;
    ret.b = mat.m[0][1] * plane.a + mat.m[1][1] * plane.b + mat.m[2][1] * plane.c + mat.m[3][1] * plane.d;
    ret.c = mat.m[0][2] * plane.a + mat.m[1][2] * plane.b + mat.m[2][2] * plane.c + mat.m[3][2] * plane.d;
    ret.d = mat.m[0][3] * plane.a + mat.m[1][3] * plane.b + mat.m[2][3] * plane.c + mat.m[3][3] * plane.d;
    return ret;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// vaGeometry
///////////////////////////////////////////////////////////////////////////////////////////////////

float vaGeometry::FresnelTerm( float cosTheta, float refractionIndex )
{
    float a, d, g, result;

    g = vaMath::Sqrt( refractionIndex * refractionIndex + cosTheta * cosTheta - 1.0f );
    a = g + cosTheta;
    d = g - cosTheta;
    result = ( cosTheta * a - 1.0f ) * ( cosTheta * a - 1.0f ) / ( ( cosTheta * d + 1.0f ) * ( cosTheta * d + 1.0f ) ) + 1.0f;
    result = result * 0.5f * d * d / ( a * a );
    return result;
}

void vaGeometry::CalculateFrustumPlanes( vaPlane planes[6], const vaMatrix4x4 & mCameraViewProj )
{
    // Left clipping plane
    planes[0] = vaPlane(
        mCameraViewProj( 0, 3 ) + mCameraViewProj( 0, 0 ),
        mCameraViewProj( 1, 3 ) + mCameraViewProj( 1, 0 ),
        mCameraViewProj( 2, 3 ) + mCameraViewProj( 2, 0 ),
        mCameraViewProj( 3, 3 ) + mCameraViewProj( 3, 0 ) );

    // Right clipping plane
    planes[1] = vaPlane(
        mCameraViewProj( 0, 3 ) - mCameraViewProj( 0, 0 ),
        mCameraViewProj( 1, 3 ) - mCameraViewProj( 1, 0 ),
        mCameraViewProj( 2, 3 ) - mCameraViewProj( 2, 0 ),
        mCameraViewProj( 3, 3 ) - mCameraViewProj( 3, 0 ) );

    // Top clipping plane
    planes[2] = vaPlane(
        mCameraViewProj( 0, 3 ) - mCameraViewProj( 0, 1 ),
        mCameraViewProj( 1, 3 ) - mCameraViewProj( 1, 1 ),
        mCameraViewProj( 2, 3 ) - mCameraViewProj( 2, 1 ),
        mCameraViewProj( 3, 3 ) - mCameraViewProj( 3, 1 ) );

    // Bottom clipping plane
    planes[3] = vaPlane(
        mCameraViewProj( 0, 3 ) + mCameraViewProj( 0, 1 ),
        mCameraViewProj( 1, 3 ) + mCameraViewProj( 1, 1 ),
        mCameraViewProj( 2, 3 ) + mCameraViewProj( 2, 1 ),
        mCameraViewProj( 3, 3 ) + mCameraViewProj( 3, 1 ) );

    // Near clipping plane
    planes[4] = vaPlane(
        mCameraViewProj( 0, 2 ),
        mCameraViewProj( 1, 2 ),
        mCameraViewProj( 2, 2 ),
        mCameraViewProj( 3, 2 ) );

    // Far clipping plane
    planes[5] = vaPlane(
        mCameraViewProj( 0, 3 ) - mCameraViewProj( 0, 2 ),
        mCameraViewProj( 1, 3 ) - mCameraViewProj( 1, 2 ),
        mCameraViewProj( 2, 3 ) - mCameraViewProj( 2, 2 ),
        mCameraViewProj( 3, 3 ) - mCameraViewProj( 3, 2 ) );
    // Normalize the plane equations, if requested

    for( int i = 0; i < 6; i++ )
        planes[i] = planes[i].PlaneNormalize( );
}

#pragma warning( suppress : 4056 4756 )
vaBoundingBox               vaBoundingBox::Degenerate( vaVector3( 0.0f, 0.0f, 0.0f ), vaVector3( -INFINITY, -INFINITY, -INFINITY ) );
#pragma warning( suppress : 4056 4756 )
vaOrientedBoundingBox       vaOrientedBoundingBox::Degenerate( vaVector3( 0.0f, 0.0f, 0.0f ), vaVector3( -INFINITY, -INFINITY, -INFINITY ), vaMatrix3x3( 1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f ) );
