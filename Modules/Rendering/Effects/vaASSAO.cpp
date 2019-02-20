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

#include "vaASSAO.h"

#ifdef SSAO_ALLOW_INTERNAL_SHADER_DEBUGGING
#include "Rendering/vaRenderingGlobals.h"
#endif

using namespace VertexAsylum;

vaASSAO::vaASSAO( ) 
{ 
    m_size = m_halfSize = m_quarterSize = vaVector2i( 0, 0 ); 
    m_prevQualityLevel              = -1;
    m_debugShowNormals              = false; 
    m_debugShowEdges                = false;
    m_debugShowSampleHeatmap        = false;
    //m_debugOpaqueOutput             = false;
#ifdef SSAO_ALLOW_INTERNAL_SHADER_DEBUGGING
    m_debugShowSamplesAtCursor      = false;
    m_debugShowSamplesAtCursorPos   = vaVector2i( 0, 0 );
#endif
    m_debugRefSamplesDistribution   = 1.7f;
    m_debugExperimentalFullscreenReferencePath = false;
}

void vaASSAO::IHO_Draw( )
{
#ifdef VA_IMGUI_INTEGRATION_ENABLED

    // Keyboard input (but let the ImgUI controls have input priority)
    
    // extension for "Lowest"
    int qualityLevelUI = m_settings.QualityLevel;

    if( !ImGui::GetIO( ).WantCaptureKeyboard )
    {
        if( ( vaInputKeyboardBase::GetCurrent( ) != nullptr ) && vaInputKeyboardBase::GetCurrent( )->IsKeyClicked( KK_OEM_4 ) )
            qualityLevelUI--;
        if( ( vaInputKeyboardBase::GetCurrent( ) != nullptr ) && vaInputKeyboardBase::GetCurrent( )->IsKeyClicked( KK_OEM_6 ) )
            qualityLevelUI++;
        qualityLevelUI = vaMath::Clamp( qualityLevelUI, SSAO_QUALITY_LEVEL_LOWEST, SSAO_QUALITY_LEVEL_REFERENCE );
        if( ( vaInputKeyboardBase::GetCurrent( ) != nullptr ) && vaInputKeyboardBase::GetCurrent( )->IsKeyClicked( KK_OEM_1 ) )
            m_settings.AdaptiveQualityLimit -= 0.025f;
        if( ( vaInputKeyboardBase::GetCurrent( ) != nullptr ) && vaInputKeyboardBase::GetCurrent( )->IsKeyClicked( KK_OEM_7 ) )
            m_settings.AdaptiveQualityLimit += 0.025f;
        m_settings.AdaptiveQualityLimit = vaMath::Clamp( m_settings.AdaptiveQualityLimit, 0.0f, 1.0f );
    }

    ImGui::PushItemWidth( 180.0f );

    ImGui::Text( "Performance/quality settings:" );
	ImGui::Combo("ASSAO Mode", &m_settings.ASSAOMode, "Pixel Shader\0Compute Shader");
    m_settings.ASSAOMode = vaMath::Clamp(m_settings.ASSAOMode, SSAO_MODE_PIXEL_SHADER, SSAO_MODE_COMPUTE_SHADER);

	ImGui::PushItemWidth(120.0f);
    ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 0.8f, 0.8f, 1.0f ) );
    ImGui::Combo( "Quality level", &qualityLevelUI, "Lowest\0Low\0Medium\0High\0Highest\0Reference\0\0" );  // Combo using values packed in a single constant string (for really quick combo)

    m_settings.QualityLevel = qualityLevelUI;

    if( ImGui::IsItemHovered() ) ImGui::SetTooltip( "Each quality level is roughly 2x more costly than the previous." );
    ImGui::PopStyleColor( 1 );

    ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );

    if( m_settings.QualityLevel == SSAO_QUALITY_LEVEL_REFERENCE )
    {
        ImGui::InputFloat( "Reference Sample Distribution POW", &m_debugRefSamplesDistribution, 0.05f, 0.0f, 2 );
        m_debugRefSamplesDistribution = vaMath::Clamp( m_debugRefSamplesDistribution, 0.5f, 2.0f );
    }

    if( m_settings.QualityLevel <= SSAO_QUALITY_LEVEL_LOW )
    {
        ImGui::InputInt( "Simple blur passes (0-1)", &m_settings.BlurPassCount );
        if( ImGui::IsItemHovered( ) ) ImGui::SetTooltip( "For Low quality, only one optional simple blur pass can be applied (recommended); settings above 1 are ignored" );
    }
    else
    {
        ImGui::InputInt( "Smart blur passes (0-6)", &m_settings.BlurPassCount );
        if( ImGui::IsItemHovered( ) ) ImGui::SetTooltip( "The amount of edge-aware smart blur; each additional pass increases blur effect but adds to the cost" );
    }
    m_settings.BlurPassCount = vaMath::Clamp( m_settings.BlurPassCount, 1, 6 );

    ImGui::Separator();
    ImGui::Text( "Visual settings:" );
    ImGui::InputFloat( "Effect radius",                     &m_settings.Radius                          , 0.05f, 0.0f, 2 );
    if( ImGui::IsItemHovered( ) ) ImGui::SetTooltip( "World (viewspace) effect radius" );
    ImGui::InputFloat( "Occlusion multiplier",              &m_settings.ShadowMultiplier                , 0.05f, 0.0f, 2 );
    if( ImGui::IsItemHovered( ) ) ImGui::SetTooltip( "Effect strength" );
    ImGui::InputFloat( "Occlusion power curve",             &m_settings.ShadowPower                     , 0.05f, 0.0f, 2 );
    if( ImGui::IsItemHovered( ) ) ImGui::SetTooltip( "occlusion = pow( occlusion, value ) - changes the occlusion curve" );
    ImGui::InputFloat( "Fadeout distance from",             &m_settings.FadeOutFrom                     , 1.0f , 0.0f, 1 );
    if( ImGui::IsItemHovered( ) ) ImGui::SetTooltip( "Distance at which to start fading out the effect" );
    ImGui::InputFloat( "Fadeout distance to",               &m_settings.FadeOutTo                       , 1.0f , 0.0f, 1 );
    if( ImGui::IsItemHovered( ) ) ImGui::SetTooltip( "Distance at which to completely fade out the effect" );
    ImGui::InputFloat( "Sharpness",                         &m_settings.Sharpness                       , 0.01f, 0.0f, 2 );
    if( ImGui::IsItemHovered( ) ) ImGui::SetTooltip( "How much to bleed over edges; 1: not at all, 0.5: half-half; 0.0: completely ignore edges" );

    ImGui::Separator( );
    ImGui::Text( "Advanced visual settings:" );
    ImGui::InputFloat( "Detail occlusion multiplier",       &m_settings.DetailShadowStrength            , 0.05f, 0.0f, 2 );
    if( ImGui::IsItemHovered( ) ) ImGui::SetTooltip( "Additional small radius / high detail occlusion; too much will create aliasing & temporal instability" );
    ImGui::InputFloat( "Horizon angle threshold",           &m_settings.HorizonAngleThreshold           , 0.01f, 0.0f, 2 );
    if( ImGui::IsItemHovered( ) ) ImGui::SetTooltip( "Reduces precision and tessellation related unwanted occlusion" );
    ImGui::InputFloat( "Occlusion max clamp",               &m_settings.ShadowClamp                     , 0.01f, 0.0f, 2 );
    if( ImGui::IsItemHovered( ) ) ImGui::SetTooltip( "occlusion = min( occlusion, value ) - limits the occlusion maximum" );    
    //ImGui::InputFloat( "Radius distance-based modifier",    &m_settings.RadiusDistanceScalingFunction   , 0.05f, 0.0f, 2 );
    //if( ImGui::IsItemHovered( ) ) ImGui::SetTooltip( "Used to modify ""Effect radius"" based on distance from the camera; for 1.0, effect world radius is constant (default);\nfor values smaller than 1.0, the effect radius will grow the more distant from the camera it is; if changed, ""Effect Radius"" often needs to be rebalanced as well" );

    m_settings.Radius                           = vaMath::Clamp( m_settings.Radius                          , 0.0f, 100.0f      );
    m_settings.HorizonAngleThreshold            = vaMath::Clamp( m_settings.HorizonAngleThreshold           , 0.0f, 1.0f        );
    m_settings.ShadowMultiplier                 = vaMath::Clamp( m_settings.ShadowMultiplier                , 0.0f, 5.0f       );
    m_settings.ShadowPower                      = vaMath::Clamp( m_settings.ShadowPower                     , 0.5f, 5.0f        );
    m_settings.ShadowClamp                      = vaMath::Clamp( m_settings.ShadowClamp                     , 0.1f, 1.0f        );
    m_settings.FadeOutFrom                      = vaMath::Clamp( m_settings.FadeOutFrom                     , 0.0f, 1000000.0f  );
    m_settings.FadeOutTo                        = vaMath::Clamp( m_settings.FadeOutTo                       , 0.0f, 1000000.0f  );
    m_settings.Sharpness                        = vaMath::Clamp( m_settings.Sharpness                       , 0.0f, 1.0f        );
    m_settings.DetailShadowStrength             = vaMath::Clamp( m_settings.DetailShadowStrength            , 0.0f, 5.0f        );
    //m_settings.RadiusDistanceScalingFunction    = vaMath::Clamp( m_settings.RadiusDistanceScalingFunction   , 0.1f, 1.0f        );

    ImGui::Separator( );
    ImGui::Text( "Debugging and experimental switches:" );

    // ImGui::Checkbox( "Smoothen normals", &m_settings.SmoothenNormals );
    // if( ImGui::IsItemHovered( ) ) ImGui::SetTooltip( "Experimental - smoothens normals; fixes some aliasingartifacts, but makes some worse" );

    static int debugOption = -1;
    if(debugOption != -1 && ImGui::Button( "Clear currently selected debug option" ))
    {
        debugOption = -1;
    }

    ImGui::RadioButton( "Debug: Show normals", &debugOption, 0 ); //ImGui::SameLine();
    if( ImGui::IsItemHovered() ) ImGui::SetTooltip( "Show normals used forSSAO (either generated or provided, and after smoothing if selected)" );

    ImGui::RadioButton( "Debug: Show edges", &debugOption, 1 ); //ImGui::SameLine();
    if( ImGui::IsItemHovered() ) ImGui::SetTooltip( "Show edges used for edge-aware smart blur" );

#ifdef SSAO_ALLOW_INTERNAL_SHADER_DEBUGGING
    ImGui::RadioButton( "Debug: Show samples at cursor", &debugOption, 2 );
    if( ImGui::IsItemHovered() ) ImGui::SetTooltip( "Shows individual SSAO samples under the mouse cursor location" );
#endif
    ImGui::RadioButton( "Debug: Show sample heatmap", &debugOption, 3 );
    if( ImGui::IsItemHovered() ) ImGui::SetTooltip( "Number of samples per pixel; constant at Low/Medium/High, variable at Highest (Adaptable)" );

    m_debugShowNormals = (debugOption == 0);
    m_debugShowEdges = (debugOption == 1);
    m_debugShowSamplesAtCursor = (debugOption == 2);
    m_debugShowSampleHeatmap = (debugOption == 3);

    // ImGui::Checkbox( "Debug: Experimental Fullscreen Reference Path ", &m_debugExperimentalFullscreenReferencePath );
    // if( ImGui::IsItemHovered( ) ) ImGui::SetTooltip( "No interleaved rendering, 64 samples, other setting like quality level High, no depth preparation, etc - just one pass." );
    // if( m_debugExperimentalFullscreenReferencePath )
    // {
    //     m_debugShowSampleHeatmap    = false;
    //     m_debugShowSamplesAtCursor  = false;
    //     m_debugShowEdges            = false;
    //     m_debugShowNormals          = false;
    //     //m_settings.SmoothenNormals  = false;
    // }

    ImGui::PopStyleColor( 1 );

    ImGui::Separator( );
    ImGui::Text( m_debugInfo.c_str( ) );

    ImGui::PopItemWidth( );

#endif
}

void vaASSAO::SetAutoPatternGenerateModeSettings( bool enabled, vaVector4 data[], int currentCount )
{
    m_autoPatternGenerateModeEnabled = enabled;
    if( enabled )
    {
        assert( currentCount <= SSAO_MAX_TAPS );
        memcpy( m_autoPatternGenerateModeData, data, sizeof(vaVector4) * currentCount );
        m_autoPatternGenerateModeCurrentCount = currentCount;
        m_specialShaderMacro = std::make_pair("AUTO_PATTERN_GENERATE_MODE", "");
    }
    else
    {
        m_specialShaderMacro = std::make_pair("", "");
    }
}