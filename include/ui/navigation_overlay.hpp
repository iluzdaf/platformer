#pragma once

#include <optional>
#include <utility>
#include <imgui.h>

class ImGuiManager;
class Camera2D;
class NavigationGraph;

inline constexpr unsigned int OriginColour = IM_COL32(0, 255, 0, 255);
inline constexpr unsigned int DestinationColour = IM_COL32(0, 200, 255, 255);

enum class JumpsDrawnAs
{
    SmoothArc,
    TheFlightItself
};

void drawNavigationGraph(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const NavigationGraph &navigationGraph,
    std::optional<int> selectedNodeId,
    std::optional<std::pair<int, int>> shownEdge,
    JumpsDrawnAs jumpsDrawnAs = JumpsDrawnAs::SmoothArc);
