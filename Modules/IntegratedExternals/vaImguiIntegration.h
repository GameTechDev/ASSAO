#pragma once

#include "Core/vaCoreIncludes.h"

#ifdef VA_IMGUI_INTEGRATION_ENABLED
#include "IntegratedExternals\imgui\imgui.h"
#include "IntegratedExternals\imgui\imgui_internal.h"
#endif

namespace VertexAsylum
{
    class vaImguiHierarchyObject
    {
    private:
        static int                                              s_lastID;
        string                                                  m_persistentObjectID;

    protected:
        vaImguiHierarchyObject( )                               { m_persistentObjectID = vaStringTools::Format( "IHO%d", s_lastID ); s_lastID++; }  // create unique string id for each new object
        virtual ~vaImguiHierarchyObject( )                      { }

    private:
        // this is just so that the title of the collapsing header can be the same for multiple different objects, as ImGui tracks them by string id which defaults to the title
        virtual string                                          IHO_GetPersistentObjectID( ) const  { return m_persistentObjectID.c_str(); }

    public:
        // this string can change at runtime!
        virtual string                                          IHO_GetInstanceInfo( ) const        { return "Unnamed"; }
        // draw your own IMGUI stuff here, and call IHO_DrawIfOpen on sub-elements
        virtual void                                            IHO_Draw( )                         { }

    public:
        static void                                             DrawCollapsable( vaImguiHierarchyObject & obj, bool display_frame = true, bool default_open = false, bool indent = true );
    };

#ifdef VA_IMGUI_INTEGRATION_ENABLED
    inline ImVec4       ImFromVA( const vaVector4 &  v )            { return ImVec4( v.x, v.y, v.z, v.w); }
    inline vaVector4    VAFromIm( const ImVec4 &  v )               { return vaVector4( v.x, v.y, v.z, v.w); }

    // Two big TTF fonts for big.. things? Created in vaApplicationBase
    ImFont *                                                    ImGetBigClearSansRegular( );
    ImFont *                                                    ImGetBigClearSansBold( );
#endif
}

