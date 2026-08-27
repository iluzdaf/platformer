#include "rendering/ui/debug_navigation_ui.hpp"
#include "rendering/ui/imgui_manager.hpp"
#include "navigation/navigation_graph.hpp"
#include "cameras/camera2d.hpp"

void DebugNavigationUi::draw(
    ImGuiManager &imGuiManager,
    const NavigationGraph &navigationGraph,
    const Camera2D &camera,
    const Selection &selection)
{
    imGuiManager.setNextFullscreenWindow();

    bool selecting = selection.nodeId.has_value() || selection.edge.has_value();

    auto edgeShown = [&](const NavigationEdge &edge)
    {
        if (selection.edge)
            return std::pair<int, int>{edge.fromId, edge.toId} == *selection.edge;
        if (selection.nodeId)
            return edge.fromId == *selection.nodeId || edge.toId == *selection.nodeId;
        return true;
    };

    auto nodeShown = [&](int id)
    {
        if (selection.edge)
            return id == selection.edge->first || id == selection.edge->second;
        if (selection.nodeId)
            return id == *selection.nodeId;
        return true;
    };

    for (const auto &[id, node] : navigationGraph.getNodes())
        if (!selecting || nodeShown(id))
            drawNode(imGuiManager, camera, node);

    for (const auto &edge : navigationGraph.getEdges())
        if (edgeShown(edge))
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

    ImDrawList *drawList = imGuiManager.getDrawList();

    if (!edge.path.empty())
    {
        for (const glm::vec2 &position : edge.path)
            drawList->PathLineTo(imGuiManager.worldToScreen(
                position,
                camera.getZoom(),
                camera.getTopLeftPosition()));
        drawList->PathStroke(color, ImDrawFlags_None, 1.0f);
        return;
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
    drawList->AddLine(
        fromPosition,
        toPosition,
        color,
        1.0f);
}