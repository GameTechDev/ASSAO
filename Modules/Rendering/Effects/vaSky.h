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

#include "Core/vaCoreIncludes.h"

#include "Rendering/vaRenderingIncludes.h"

namespace VertexAsylum
{
    class vaSky : public VertexAsylum::vaDrawableRenderingModule
    {
    public:
        struct Settings
        {
            float                       SunAzimuth;
            float                       SunElevation;

            vaVector4                   SkyColorLow;
            vaVector4                   SkyColorHigh;

            vaVector4                   SunColorPrimary;
            vaVector4                   SunColorSecondary;

            float                       SkyColorLowPow;
            float                       SkyColorLowMul;

            float                       SunColorPrimaryPow;
            float                       SunColorPrimaryMul;
            float                       SunColorSecondaryPow;
            float                       SunColorSecondaryMul;

            vaVector3                   FogColor;
            float                       FogDistanceMin;
            float                       FogDensity;
        };

    private:

        // these are calculated from azimuth & elevation, but smoothly interpolated to avoid sudden changes
        vaVector3                   m_sunDirTargetL0;    // directly calculated from azimuth & elevation
        vaVector3                   m_sunDirTargetL1;    // lerped to m_sunDirTargetL0
        vaVector3                   m_sunDir;            // final, lerped to m_sunDirTargetL1

        Settings                    m_settings;

    protected:


    protected:
        vaSky( );
    public:
        ~vaSky( );

    public:

    public:
        Settings &                  GetSettings( )          { return m_settings; }
        const Settings &            GetSettings( ) const    { return m_settings; }
        const vaVector3 &           GetSunDir( ) const      { return m_sunDir; }

        void                        Tick( float deltaTime, vaLighting * lightingToUpdate );

    };

}
