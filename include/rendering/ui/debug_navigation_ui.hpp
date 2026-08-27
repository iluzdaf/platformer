#pragma once

#include <optional>
#include <utility>

#include <glm/gtc/matrix_transform.hpp>

class ImGuiManager;
class NavigationGraph;
struct NavigationNode;
struct ImDrawList;
struct NavigationEdge;
class Camera2D;

class DebugNavigationUi
{
public:
    struct Selection
    {
        std::optional<int> nodeId;
        std::optional<std::pair<int, int>> edge;
    };

    void draw(
        ImGuiManager &imGuiManager,
        const NavigationGraph &navigationGraph,
        const Camera2D &camera,
        const Selection &selection = {});

private:
    void drawNode(
        ImGuiManager &imGuiManager,
        const Camera2D &camera,
        const NavigationNode &node);
    void drawEdge(
        ImGuiManager &imGuiManager,
        const NavigationGraph &navigationGraph,
        const Camera2D &camera,
        const NavigationEdge &edge);
};