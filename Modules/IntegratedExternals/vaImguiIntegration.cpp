#pragma once

#include "vaImguiIntegration.h"

using namespace VertexAsylum;

int vaImguiHierarchyObject::s_lastID = 0;

#ifdef VA_IMGUI_INTEGRATION_ENABLED
void vaImguiHierarchyObject::DrawCollapsable( vaImguiHierarchyObject & obj, bool display_frame, bool default_open, bool indent )
{
    // needed so that any controls in obj.IHO_Draw() are unique
    ImGui::PushID( obj.IHO_GetPersistentObjectID( ).c_str( ) );

    ImGuiTreeNodeFlags headerFlags = 0;
    headerFlags |= (display_frame)?(ImGuiTreeNodeFlags_Framed):(0);
    headerFlags |= (default_open)?(ImGuiTreeNodeFlags_DefaultOpen):(0);

    if( ImGui::CollapsingHeader( obj.IHO_GetInstanceInfo( ).c_str( ), headerFlags ) )
    {
        if( indent )
            ImGui::Indent();
        
        obj.IHO_Draw();
        
        if( indent )
            ImGui::Unindent();
    }

    ImGui::PopID();
}
#else
void vaImguiHierarchyObject::DrawCollapsable( vaImguiHierarchyObject & obj, bool display_frame, bool default_open, bool indent )
{
}
#endif

namespace VertexAsylum
{

    static ImFont *     s_bigClearSansRegular   = nullptr;
    static ImFont *     s_bigClearSansBold      = nullptr;

    ImFont *            ImGetBigClearSansRegular( )                 { return s_bigClearSansRegular; }
    ImFont *            ImGetBigClearSansBold( )                    { return s_bigClearSansBold;    }

    void                ImSetBigClearSansRegular( ImFont * font )   { s_bigClearSansRegular = font; }
    void                ImSetBigClearSansBold(    ImFont * font )   { s_bigClearSansBold    = font; }

}