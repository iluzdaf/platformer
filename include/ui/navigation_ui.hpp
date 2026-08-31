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
    void drawOverlayToggles();
    void draw(const Level &level);
    void drawOverlay(const ImGuiManager &imGuiManager, const Camera2D &camera, const Level &level)
        const;

private:
    bool drawTheFlightItself = false;
    std::optional<size_t> selectedGraphIndex;
    std::optional<int> selectedNodeId;
    std::optional<std::pair<int, int>> shownEdge;

    void drawGraphs(const Level &level);
    void drawEdgeChooser(const NavigationGraph *graph, std::optional<int> nodeId);
    void forgetSelection();
};
