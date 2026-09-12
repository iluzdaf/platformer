#pragma once

#include <imgui.h>
#include "helpers/headless_imgui.hpp"

template <class Draw> bool drawnIn(HeadlessImGui &gui, ImVec4 colour, Draw &&draw)
{
    bool found = false;
    gui.frame(draw);
    gui.frame(
        [&]
        {
            ImDrawList *drawList = ImGui::GetWindowDrawList();
            int before = drawList->VtxBuffer.Size;
            ImU32 wanted = ImGui::GetColorU32(colour);
            draw();
            for (int at = before; at < drawList->VtxBuffer.Size; ++at)
                if (drawList->VtxBuffer[at].col == wanted)
                    found = true;
        });

    return found;
}
