#pragma once

#include <set>
#include <imgui.h>
#include <imgui_internal.h>
#include "helpers/headless_imgui.hpp"

template <class Draw> std::set<ImTextureID> sheetsDrawnWhile(HeadlessImGui &gui, Draw &&draw)
{
    std::set<ImTextureID> drawn;
    gui.frame(draw);
    gui.frame(
        [&]
        {
            ImDrawList *drawList = ImGui::GetWindowDrawList();
            draw();
            for (const ImDrawCmd &command : drawList->CmdBuffer)
                drawn.insert(command.GetTexID());
        });

    return drawn;
}

template <class Draw> int picturesWideDrawn(HeadlessImGui &gui, float width, Draw &&draw)
{
    int drawn = 0;
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
                    ++drawn;
            }
        });

    return drawn;
}

template <class Draw> bool drawsAPictureWide(HeadlessImGui &gui, float width, Draw &&draw)
{
    return picturesWideDrawn(gui, width, draw) > 0;
}
