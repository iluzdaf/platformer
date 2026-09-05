#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include "test_helpers/headless_imgui.hpp"

template <class Draw> bool drawsAPictureWide(HeadlessImGui &gui, float width, Draw &&draw)
{
    bool drawn = false;
    gui.frame(draw);
    gui.frame(
        [&]
        {
            ImDrawList *drawList = ImGui::GetWindowDrawList();
            int before = drawList->VtxBuffer.Size;
            draw();
            for (int at = before; at + 1 < drawList->VtxBuffer.Size; ++at)
            {
                const ImDrawVert &corner = drawList->VtxBuffer[at];
                const ImDrawVert &next = drawList->VtxBuffer[at + 1];
                if (corner.pos.y == next.pos.y && next.pos.x - corner.pos.x == width)
                    drawn = true;
            }
        });

    return drawn;
}
