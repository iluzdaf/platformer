#include <algorithm>
#include <set>
#include <string>
#include "rendering/ui/navigation_overlay.hpp"
#include "rendering/ui/imgui_manager.hpp"
#include "navigation/navigation_graph.hpp"
#include "cameras/camera2d.hpp"

namespace
{
    constexpr unsigned int NodeColour = IM_COL32(0, 255, 0, 255);
    constexpr unsigned int OriginColour = IM_COL32(0, 255, 0, 255);
    constexpr unsigned int DestinationColour = IM_COL32(0, 200, 255, 255);

    void drawNode(
        const ImGuiManager &imGuiManager,
        const Camera2D &camera,
        const NavigationNode &node,
        int id,
        unsigned int colour)
    {
        ImVec2 position = imGuiManager.worldToScreen(
            node.position,
            camera.getZoom(),
            camera.getTopLeftPosition());
        ImDrawList *drawList = imGuiManager.getDrawList();
        drawList->AddCircleFilled(position, 5, colour, 16);

        std::string label = std::to_string(id);
        ImVec2 labelSize = ImGui::CalcTextSize(label.c_str());
        drawList->AddText(
            ImVec2(position.x - labelSize.x * 0.5f, position.y - 6 - labelSize.y),
            colour,
            label.c_str());
    }

    void drawEdge(
        const ImGuiManager &imGuiManager,
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

        NavigationNode takeOff = navigationGraph.getNode(edge.fromId);
        NavigationNode landing = navigationGraph.getNode(edge.toId);

        if (!edge.path.empty())
        {
            float chordY = (takeOff.position.y + landing.position.y) * 0.5f;
            float apexY = chordY;
            for (const glm::vec2 &position : edge.path)
                apexY = std::min(apexY, position.y);

            glm::vec2 control(
                (takeOff.position.x + landing.position.x) * 0.5f,
                chordY - 2.0f * (chordY - apexY));

            auto screen = [&](glm::vec2 world)
            { return imGuiManager.worldToScreen(world, camera.getZoom(), camera.getTopLeftPosition()); };

            drawList->PathLineTo(screen(takeOff.position));
            drawList->PathBezierQuadraticCurveTo(screen(control), screen(landing.position));
            drawList->PathStroke(color, ImDrawFlags_None, 1.5f);
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
}

void drawNavigationGraph(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const NavigationGraph &navigationGraph,
    std::optional<int> selectedNodeId,
    std::optional<std::pair<int, int>> selectedEdge)
{
    auto edgeShown = [&](const NavigationEdge &edge)
    {
        if (selectedEdge)
            return std::pair<int, int>{edge.fromId, edge.toId} == *selectedEdge;
        if (selectedNodeId)
            return edge.fromId == *selectedNodeId || edge.toId == *selectedNodeId;
        return true;
    };

    std::optional<int> origin = selectedNodeId;
    if (selectedEdge)
        origin = selectedEdge->first;

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
            drawNode(imGuiManager, camera, node, id, NodeColour);
        else if (id == *origin)
            drawNode(imGuiManager, camera, node, id, OriginColour);
        else if (otherEnds.contains(id))
            drawNode(imGuiManager, camera, node, id, DestinationColour);
    }

    for (const auto &edge : navigationGraph.getEdges())
        if (edgeShown(edge))
            drawEdge(imGuiManager, navigationGraph, camera, edge);
}
