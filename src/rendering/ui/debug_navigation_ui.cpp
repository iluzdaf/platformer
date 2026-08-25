#include "rendering/ui/debug_navigation_ui.hpp"
#include "rendering/ui/imgui_manager.hpp"
#include "navigation/navigation_graph.hpp"
#include "cameras/camera2d.hpp"

void DebugNavigationUi::draw(
    ImGuiManager &imGuiManager,
    const NavigationGraph &navigationGraph,
    const Camera2D &camera)
{
    imGuiManager.setNextFullscreenWindow();

    for (const auto &[id, node] : navigationGraph.getNodes())
        drawNode(imGuiManager, camera, node);

    for (auto edge : navigationGraph.getEdges())
        drawEdge(imGuiManager, navigationGraph, camera, edge);
}

void DebugNavigationUi::drawNode(
    ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const NavigationNode &node)
{
    ImVec2 position = imGuiManager.worldToScreen(
        node.position,
        camera.getZoom(),
        camera.getTopLeftPosition());
    ImDrawList *drawList = imGuiManager.getDrawList();
    drawList->AddCircle(position, 5, IM_COL32(0, 255, 0, 255), 16);
}

void DebugNavigationUi::drawEdge(
    ImGuiManager &imGuiManager,
    const NavigationGraph &navigationGraph,
    const Camera2D &camera,
    const NavigationEdge &edge)
{
    ImU32 color = IM_COL32(255, 255, 255, 255);
    switch (edge.type)
    {
    case EdgeType::Walk:
        color = IM_COL32(255, 255, 255, 255);
        break;
    case EdgeType::Jump:
        color = IM_COL32(255, 200, 0, 255);
        break;
    case EdgeType::Fall:
        color = IM_COL32(100, 100, 255, 255);
        break;
    case EdgeType::Climb:
        color = IM_COL32(0, 255, 255, 255);
        break;
    }

    NavigationNode fromNode = navigationGraph.getNode(edge.fromId);
    ImVec2 fromPosition = imGuiManager.worldToScreen(
        fromNode.position,
        camera.getZoom(),
        camera.getTopLeftPosition());
    NavigationNode toNode = navigationGraph.getNode(edge.toId);
    ImVec2 toPosition = imGuiManager.worldToScreen(
        toNode.position,
        camera.getZoom(),
        camera.getTopLeftPosition());
    ImDrawList *drawList = imGuiManager.getDrawList();
    drawList->AddLine(
        fromPosition,
        toPosition,
        color,
        1.0f);
}