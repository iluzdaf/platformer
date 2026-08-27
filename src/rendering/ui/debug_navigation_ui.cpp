#include <set>
#include "rendering/ui/debug_navigation_ui.hpp"
#include "rendering/ui/imgui_manager.hpp"
#include "navigation/navigation_graph.hpp"
#include "cameras/camera2d.hpp"

namespace
{
    constexpr unsigned int NodeColour = IM_COL32(0, 255, 0, 255);
    constexpr unsigned int OriginColour = IM_COL32(0, 255, 0, 255);
    constexpr unsigned int DestinationColour = IM_COL32(0, 200, 255, 255);
}

void DebugNavigationUi::draw(
    ImGuiManager &imGuiManager,
    const NavigationGraph &navigationGraph,
    const Camera2D &camera,
    const Selection &selection)
{
    imGuiManager.setNextFullscreenWindow();

    auto edgeShown = [&](const NavigationEdge &edge)
    {
        if (selection.edge)
            return std::pair<int, int>{edge.fromId, edge.toId} == *selection.edge;
        if (selection.nodeId)
            return edge.fromId == *selection.nodeId || edge.toId == *selection.nodeId;
        return true;
    };

    std::optional<int> origin = selection.nodeId;
    if (selection.edge)
        origin = selection.edge->first;

    std::set<int> otherEnds;
    if (origin)
        for (const auto &edge : navigationGraph.getEdges())
            if (edgeShown(edge))
            {
                if (edge.fromId != *origin)
                    otherEnds.insert(edge.fromId);
                if (edge.toId != *origin)
                    otherEnds.insert(edge.toId);
            }

    for (const auto &[id, node] : navigationGraph.getNodes())
    {
        if (!origin)
            drawNode(imGuiManager, camera, node, NodeColour);
        else if (id == *origin)
            drawNode(imGuiManager, camera, node, OriginColour);
        else if (otherEnds.contains(id))
            drawNode(imGuiManager, camera, node, DestinationColour);
    }

    for (const auto &edge : navigationGraph.getEdges())
        if (edgeShown(edge))
            drawEdge(imGuiManager, navigationGraph, camera, edge);
}

void DebugNavigationUi::drawNode(
    ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const NavigationNode &node,
    unsigned int colour)
{
    ImVec2 position = imGuiManager.worldToScreen(
        node.position,
        camera.getZoom(),
        camera.getTopLeftPosition());
    ImDrawList *drawList = imGuiManager.getDrawList();
    drawList->AddCircleFilled(position, 5, colour, 16);
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