#pragma once

#include <optional>
#include <utility>

class ImGuiManager;
class Camera2D;
class NavigationGraph;

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
    std::optional<std::pair<int, int>> selectedEdge,
    JumpsDrawnAs jumpsDrawnAs = JumpsDrawnAs::SmoothArc);
