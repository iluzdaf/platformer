#pragma once
class ImGuiManager;
class NavigationGraph;
struct NavigationNode;
struct ImDrawList;
struct NavigationEdge;
class Camera2D;

class DebugNavigationUi
{
public:
    void draw(
        ImGuiManager &imGuiManager,
        const NavigationGraph &navigationGraph,
        const Camera2D &camera);

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