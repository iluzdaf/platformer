#include <algorithm>
#include <imgui.h>
#include "ui/panel_placement.hpp"

PanelPlacement panelPinnedRight(ImVec2 displaySize, float width)
{
    float widest = std::max(NarrowestPanel, displaySize.x * 0.8f);
    float held = std::clamp(width, NarrowestPanel, widest);

    return PanelPlacement{
        ImVec2(std::max(0.0f, displaySize.x - held), 0.0f),
        ImVec2(NarrowestPanel, displaySize.y),
        ImVec2(widest, displaySize.y)};
}
