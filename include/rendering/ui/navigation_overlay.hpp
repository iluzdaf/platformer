#pragma once

#include <optional>
#include <utility>

class ImGuiManager;
class Camera2D;
class NavigationGraph;

// A jump can be drawn as the flight it was simulated as, or as a curve fitted
// to it, which reads more easily and hides how coarse the steps are.
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
