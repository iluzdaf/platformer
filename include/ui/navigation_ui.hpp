#pragma once

#include <cstddef>
#include <optional>
#include <utility>

class Camera2D;
class ImGuiManager;
class Level;
class NavigationGraph;

class NavigationUi
{
public:
    void draw(const Level &level);
    void drawOverlay(const ImGuiManager &imGuiManager, const Camera2D &camera, const Level &level)
        const;

private:
    bool showNavigation = true, drawTheFlightItself = false;
    size_t selectedGraphIndex = 0;
    std::optional<int> selectedNodeId;
    std::optional<std::pair<int, int>> selectedEdge;

    void drawEdgesOf(const NavigationGraph &graph, int nodeId);
};
