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

#include "Rendering/Effects/vaSky.h"

using namespace VertexAsylum;

vaSky::vaSky( )
    : vaDrawableRenderingModule( )
{ 
    assert( vaRenderingCore::IsInitialized() );

    m_sunDir = vaVector3( 0.0f, 0.0f, 0.0f );
    m_sunDirTargetL0 = vaVector3( 0.0f, 0.0f, 0.0f );
    m_sunDirTargetL1 = vaVector3( 0.0f, 0.0f, 0.0f );

    m_settings.SunAzimuth           = 0.320f;//0.0f / 180.0f * (float)VA_PI;
    m_settings.SunElevation         = 15.0f / 180.0f * (float)VA_PI;
    m_settings.SkyColorLowPow       = 6.0f;
    m_settings.SkyColorLowMul       = 1.0f;
    m_settings.SkyColorLow          = vaVector4( 0.4f, 0.4f, 0.9f, 0.0f );
    m_settings.SkyColorHigh         = vaVector4( 0.0f, 0.0f, 0.6f, 0.0f );
    m_settings.SunColorPrimary      = vaVector4( 1.0f, 1.0f, 0.9f, 0.0f );
    m_settings.SunColorSecondary    = vaVector4( 1.0f, 1.0f, 0.7f, 0.0f );
    m_settings.SunColorPrimaryPow   = 500.0f;
    m_settings.SunColorPrimaryMul   = 2.5f;
    m_settings.SunColorSecondaryPow = 5.0;
    m_settings.SunColorSecondaryMul = 0.2f;

    m_settings.FogColor             = vaVector3( 0.4f, 0.4f, 0.9f );
    m_settings.FogDistanceMin       = 100.0f;
    m_settings.FogDensity           = 0.0007f;

}

vaSky::~vaSky( )
{
}

// #ifdef VA_ANT_TWEAK_BAR_ENABLED   
// void vaSky::OnAntTweakBarInitialized( TwBar * mainBar )
// {
//     //m_debugBar = mainBar;
// 
//     if( mainBar == NULL )
//         return;
// 
//     // Create a new TwType to edit 3D points: a struct that contains two floats
//     TwStructMember structDef[] = {
//         { "Sun Azimuth",                TW_TYPE_FLOAT,      offsetof( Settings, SunAzimuth ),           " Min=0.0 Max=6.2831853     Step=0.01   Precision = 3" },
//         { "Sun Elevation",              TW_TYPE_FLOAT,      offsetof( Settings, SunElevation ),         " Min=-1.5708 Max=1.5708    Step=0.02   Precision = 3" },
//         { "Sky Colour High",            TW_TYPE_COLOR3F,    offsetof( Settings, SkyColorHigh ),         "" },
//         { "Sky Colour Low",             TW_TYPE_COLOR3F,    offsetof( Settings, SkyColorLow ),          "" },
//         { "Sun Colour Primary",         TW_TYPE_COLOR3F,    offsetof( Settings, SunColorPrimary ),      "" },
//         { "Sun Colour Secondary",       TW_TYPE_COLOR3F,    offsetof( Settings, SunColorSecondary ),    "" },
//         { "Sky Colour Low Pow",         TW_TYPE_FLOAT,      offsetof( Settings, SkyColorLowPow ),       "" },
//         { "Sky Colour Low Mul",         TW_TYPE_FLOAT,      offsetof( Settings, SkyColorLowMul ),       "" },
//         { "Sun Colour Primary Pow",     TW_TYPE_FLOAT,      offsetof( Settings, SunColorPrimaryPow ),   "" },
//         { "Sun Colour Primary Mul",     TW_TYPE_FLOAT,      offsetof( Settings, SunColorPrimaryMul ),   "" },
//         { "Sun Colour Secondary Pow",   TW_TYPE_FLOAT,      offsetof( Settings, SunColorSecondaryPow ), "" },
//         { "Sun Colour Secondary Mul",   TW_TYPE_FLOAT,      offsetof( Settings, SunColorSecondaryMul ), "" },
//         { "Fog Colour",                 TW_TYPE_COLOR3F,    offsetof( Settings, FogColor ),             "" },
//         { "FogMin",                     TW_TYPE_FLOAT,      offsetof( Settings, FogDistanceMin ),       " Min=0.0 Max=32768     Step=1.00       Precision = 2" },
//         { "FogDensity",                 TW_TYPE_FLOAT,      offsetof( Settings, FogDensity ),           " Min=0.0 Max=10        Step=0.0001     Precision = 4" },
//     };
// 
//     TwType TwSkySettings = TwDefineStruct( "SKYSETTINGS", structDef, _countof( structDef ), sizeof( Settings ), NULL, NULL );
// 
//     TwAddVarRW( mainBar, "Sky", TwSkySettings, &m_settings, "" );
// }
// #endif

void vaSky::Tick( float deltaTime, vaLighting * lightingToUpdate )
{
    // this smoothing is not needed here, but I'll leave it in anyway
    static float someValue = 10000000.0f;
    float lf = vaMath::TimeIndependentLerpF( deltaTime, someValue );

    vaVector3 sunDirTargetL0    = m_sunDirTargetL0;
    vaVector3 sunDirTargetL1    = m_sunDirTargetL1;
    vaVector3 sunDir            = m_sunDir;

    if( sunDir.x < 1e-5f )
        lf = 1.0f;

    vaMatrix4x4 mCameraRot;
    vaMatrix4x4 mRotationY = vaMatrix4x4::RotationY( m_settings.SunElevation );
    vaMatrix4x4 mRotationZ = vaMatrix4x4::RotationZ( m_settings.SunAzimuth );
    mCameraRot = mRotationY * mRotationZ;
    sunDirTargetL0 = -mCameraRot.GetRotationX();

    sunDirTargetL1 = vaMath::Lerp( sunDirTargetL1, sunDirTargetL0, lf );
    sunDir = vaMath::Lerp( sunDir, sunDirTargetL1, lf );

    sunDirTargetL0 = sunDirTargetL0.Normalize();
    sunDirTargetL1 = sunDirTargetL1.Normalize();
    sunDir = sunDir.Normalize();

    m_sunDirTargetL0= sunDirTargetL0;
    m_sunDirTargetL1= sunDirTargetL1;
    m_sunDir        = sunDir;

    vaVector3 ambientLightColor = vaVector3( 0.1f, 0.1f, 0.1f );
    
    if( lightingToUpdate != nullptr )
    {
        lightingToUpdate->SetDirectionalLightDirection( -m_sunDir );
        lightingToUpdate->SetFogParams( m_settings.FogColor, m_settings.FogDistanceMin, m_settings.FogDensity );
    }
}


