#pragma once

#include <cstddef>
#include "ui/navigation_shown.hpp"

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
    NavigationShown shown;

    void drawGraphs(const Level &level);
    void drawEdgeChooser(const NavigationGraph *graph);
};
