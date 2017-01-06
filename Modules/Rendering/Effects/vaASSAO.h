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
// 2016-09-07: filip.strugar@intel.com: first commit
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rendering/Media/Shaders/vaShaderCore.h"

#include "Rendering/vaRendering.h"

#include "Rendering/vaTexture.h"

#include "IntegratedExternals/vaImguiIntegration.h"

#define INTEL_SSAO_INCLUDED_FROM_CPP

#include "Rendering\Media\Shaders\vaASSAO_types.h"

namespace VertexAsylum
{
    class vaASSAO : public vaRenderingModule, public vaImguiHierarchyObject
    {
        VA_RENDERING_MODULE_MAKE_FRIENDS( );
    private:

    public:
        struct Settings
        {
            float       Radius;                             // [0.0,  ~ ]   World (view) space size of the occlusion sphere.
            float       ShadowMultiplier;                   // [0.0, 5.0]   Effect strength linear multiplier
            float       ShadowPower;                        // [0.5, 5.0]   Effect strength pow modifier
            float       ShadowClamp;                        // [0.0, 1.0]   Effect max limit (applied after multiplier but before blur)
            float       HorizonAngleThreshold;              // [0.0, 0.2]   Limits self-shadowing (makes the sampling area less of a hemisphere, more of a spherical cone, to avoid self-shadowing and various artifacts due to low tessellation and depth buffer imprecision, etc.)
            float       FadeOutFrom;                        // [0.0,  ~ ]   Distance to start start fading out the effect.
            float       FadeOutTo;                          // [0.0,  ~ ]   Distance at which the effect is faded out.
            int         QualityLevel;                       // [  0,    ]   Effect quality; 0 - low, 1 - medium, 2 - high, 3 - very high / adaptive; each quality level is roughly 2x more costly than the previous, except the q3 which is variable but, in general, above q2.
            float       AdaptiveQualityLimit;               // [0.0, 1.0]   (only for Quality Level 3)
    		int         BlurPassCount;                      // [  0,   6]   Number of edge-sensitive smart blur passes to apply. Quality 0 is an exception with only one 'dumb' blur pass used.
            float       Sharpness;                          // [0.0, 1.0]   (How much to bleed over edges; 1: not at all, 0.5: half-half; 0.0: completely ignore edges)
            float       TemporalSupersamplingAngleOffset;   // [0.0,  PI]   Used to rotate sampling kernel; If using temporal AA / supersampling, suggested to rotate by ( (frame%3)/3.0*PI ) or similar. Kernel is already symmetrical, which is why we use PI and not 2*PI.
            float       TemporalSupersamplingRadiusOffset;  // [0.0, 2.0]   Used to scale sampling kernel; If using temporal AA / supersampling, suggested to scale by ( 1.0f + (((frame%3)-1.0)/3.0)*0.1 ) or similar.
    		float       DetailShadowStrength;               // [0.0, 5.0]   Used for high-res detail AO using neighboring depth pixels: adds a lot of detail but also reduces temporal stability (adds aliasing).
            bool        SkipHalfPixelsOnLowQualityLevel;    // [true/false] Use half of the pixels (checkerboard pattern) when in Low quality for "Lowest" setting

            Settings( )
            {
                Radius                              = 1.2f;
                ShadowMultiplier                    = 1.0f;
                ShadowPower                         = 1.50f;
                ShadowClamp                         = 0.98f;
                HorizonAngleThreshold               = 0.06f;
                FadeOutFrom                         = 50.0f;
                FadeOutTo                           = 300.0f;
                AdaptiveQualityLimit                = 0.45f;
                QualityLevel                        = 2;
                BlurPassCount                       = 2;
                Sharpness                           = 0.98f;
                TemporalSupersamplingAngleOffset    = 0.0f;
                TemporalSupersamplingRadiusOffset   = 1.0f;
        		DetailShadowStrength                = 0.5f;
                SkipHalfPixelsOnLowQualityLevel     = false;
            }
        };

    protected:
        struct BufferFormats
        {
            vaTextureFormat         DepthBufferViewspaceLinear;
            vaTextureFormat         AOResult;
#if SSAO_USE_SEPARATE_LOWQ_AORESULTS_TEXTURE
            vaTextureFormat         AOResultLowQ;
#endif
            vaTextureFormat         Normals;
            vaTextureFormat         ImportanceMap;

            BufferFormats( )
            {
                DepthBufferViewspaceLinear  = vaTextureFormat::R16_FLOAT;       // increase to R32_FLOAT if using very low FOVs (for ex, for sniper scope effect) or similar, or in case you suspect artifacts caused by lack of precision; performance will degrade
                Normals                     = vaTextureFormat::R8G8B8A8_UNORM;
                AOResult                    = vaTextureFormat::R8G8_UNORM;
#if SSAO_USE_SEPARATE_LOWQ_AORESULTS_TEXTURE
                AOResultLowQ                = vaTextureFormat::R8_UNORM;
#endif
                ImportanceMap               = vaTextureFormat::R8_UNORM;
            }
        };

    protected:
        string                                      m_debugInfo;
        mutable int                                 m_prevQualityLevel;
        mutable bool                                m_debugShowNormals;
        mutable bool                                m_debugShowEdges;
        mutable bool                                m_debugShowSampleHeatmap;
        mutable float                               m_debugRefSamplesDistribution;
        mutable bool                                m_debugExperimentalFullscreenReferencePath;

#ifdef SSAO_ALLOW_INTERNAL_SHADER_DEBUGGING
        mutable bool                                m_debugShowSamplesAtCursor;
        vaVector2i                                  m_debugShowSamplesAtCursorPos;
#endif

        BufferFormats                               m_formats;

        vaVector2i                                  m_size;
        vaVector2i                                  m_halfSize;
        vaVector2i                                  m_quarterSize;

        vaVector4i                                  m_fullResOutScissorRect;
        vaVector4i                                  m_halfResOutScissorRect;

        shared_ptr<vaTexture>                       m_halfDepths[4];
        shared_ptr<vaTexture>                       m_halfDepthsMipViews[4][SSAO_DEPTH_MIP_LEVELS];
        shared_ptr<vaTexture>                       m_pingPongHalfResultA;
        shared_ptr<vaTexture>                       m_pingPongHalfResultB;
        shared_ptr<vaTexture>                       m_finalResults;
        shared_ptr<vaTexture>                       m_finalResultsArrayViews[4];
        shared_ptr<vaTexture>                       m_normals;
        shared_ptr<vaTexture>                       m_loadCounter;
        shared_ptr<vaTexture>                       m_importanceMap;
        shared_ptr<vaTexture>                       m_importanceMapPong;

        shared_ptr<vaTexture>                       m_debuggingOutput;

        mutable Settings                            m_settings;

        pair< string, string >                      m_specialShaderMacro;

        bool                                        m_autoPatternGenerateModeEnabled;
        vaVector4                                   m_autoPatternGenerateModeData[SSAO_MAX_TAPS];
        int                                         m_autoPatternGenerateModeCurrentCount;

    protected:
        vaASSAO( );
    public:
        virtual ~vaASSAO( ) { }

    public:
        virtual void                                Draw( vaDrawContext & drawContext, const vaMatrix4x4 & projMatrix, vaTexture & depthTexture, bool blend, vaTexture * normalmapTexture = nullptr, const vaVector4i & scissorRect = vaVector4i( 0, 0, 0, 0 ) ) = 0;

    public:
        Settings &                                  GetSettings( ) { return m_settings; }

        void                                        SetAutoPatternGenerateModeSettings( bool enabled, vaVector4 data[], int currentCount );

        // used for debugging & optimization tests - just sets a single shader macro for all shaders (and triggers a shader recompile)
        void                                        SetSpecialShaderMacro( const pair< string, string > & ssm ) { m_specialShaderMacro = ssm; }

        //void                                        GetRotatedSamples( const vaVector2 * inSamples, int inCount, vaVector2 * outSamples, int outCountSize );

#ifdef SSAO_ALLOW_INTERNAL_SHADER_DEBUGGING
        void                                        SetDebugShowSamplesAtCursorPos( const vaVector2i & pos ) { m_debugShowSamplesAtCursorPos = pos; }
        bool                                        GetDebugShowSamplesAtCursorEnabled( ) { return m_debugShowSamplesAtCursor; }
        vaVector2i                                  GetDebugShowSamplesAtCursorPos( ) { return m_debugShowSamplesAtCursorPos; }

        void                                        SetDebugShowSamplesDistribution( float debugRefSamplesDistribution )     { m_debugRefSamplesDistribution = debugRefSamplesDistribution; }
        float                                       GetDebugShowSamplesDistribution( ) const                                 { return m_debugRefSamplesDistribution; }
#endif

        bool &                                      DebugShowNormals()                                                      { return m_debugShowNormals;        }
        bool &                                      DebugShowEdges()                                                        { return m_debugShowEdges;          }
        bool &                                      DebugShowSampleHeatmap()                                                { return m_debugShowSampleHeatmap;  }


    private:
        virtual string                              IHO_GetInstanceInfo( ) const { return "Intel ASSAO (development version)"; }
        virtual void                                IHO_Draw( );

    };

} // namespace VertexAsylum

