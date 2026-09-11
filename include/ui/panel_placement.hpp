#pragma once

#include <imgui.h>

struct PanelPlacement
{
    ImVec2 position;
    ImVec2 smallest;
    ImVec2 largest;
};

inline constexpr float NarrowestPanel = 200.0f;

PanelPlacement panelPinnedRight(ImVec2 displaySize, float width);
