#include <algorithm>
#include <cstddef>
#include <optional>
#include <utility>
#include <string>
#include <string_view>
#include <vector>
#include <imgui.h>
#include "ui/navigation_ui.hpp"
#include "ui/navigation_overlay.hpp"
#include "navigation/navigation_edge.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/named_navigation_graph.hpp"
#include "game/level.hpp"

namespace
{
    std::string_view nameOf(EdgeType type)
    {
        switch (type)
        {
        case EdgeType::Walk:
            return "walk";
        case EdgeType::Jump:
            return "jump";
        case EdgeType::Fall:
            return "fall";
        case EdgeType::Climb:
            return "climb";
        }

        return "?";
    }
}

void NavigationUi::drawOverlayToggles()
{
    ImGui::Checkbox("Graph", &showNavigation);
    ImGui::SameLine();
    ImGui::Checkbox("Real Trajectory", &drawTheFlightItself);
}

void NavigationUi::draw(const Level &level)
{
    const std::vector<NamedNavigationGraph> &graphs = level.getGraphs();
    ImGui::Indent();

    if (graphs.empty())
    {
        ImGui::TextDisabled("none");
        ImGui::Unindent();
        return;
    }

    if (selectedGraphIndex >= graphs.size())
        selectedGraphIndex = 0;

    const NamedNavigationGraph &shown = graphs[selectedGraphIndex];

    ImGui::SetNextItemWidth(170.0f);
    bool choosingGraph = false;
    if (ImGui::BeginCombo("##graph", shown.name.c_str()))
    {
        for (size_t index = 0; index < graphs.size(); ++index)
            if (ImGui::Selectable(graphs[index].name.c_str(), index == selectedGraphIndex))
            {
                selectedGraphIndex = index;
                choosingGraph = true;
            }

        ImGui::EndCombo();
    }

    if (choosingGraph)
    {
        selectedNodeId.reset();
        selectedEdge.reset();
    }

    std::vector<int> nodeIds;
    for (const auto &[nodeId, node] : shown.graph.getNodes())
        nodeIds.push_back(nodeId);
    std::sort(nodeIds.begin(), nodeIds.end());

    for (int nodeId : nodeIds)
    {
        ImGui::PushID(nodeId);
        NavigationNode node = shown.graph.getNode(nodeId);
        std::string nodeLabel = std::to_string(nodeId) + " at " +
                                std::to_string(static_cast<int>(node.position.x)) + "," +
                                std::to_string(static_cast<int>(node.position.y));

        ImGuiTreeNodeFlags flags =
            ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        if (selectedNodeId == nodeId)
            flags |= ImGuiTreeNodeFlags_Selected;

        bool open = ImGui::TreeNodeEx(nodeLabel.c_str(), flags);
        if (!choosingGraph && ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        {
            bool wasSelected = selectedNodeId == nodeId;
            selectedNodeId.reset();
            selectedEdge.reset();
            if (!wasSelected)
                selectedNodeId = nodeId;
        }

        if (open)
        {
            drawEdgesOf(shown.graph, nodeId);
            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    ImGui::Unindent();
}

void NavigationUi::drawOverlay(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const Level &level) const
{
    const std::vector<NamedNavigationGraph> &graphs = level.getGraphs();
    if (!showNavigation || graphs.empty() || selectedGraphIndex >= graphs.size())
        return;

    drawNavigationGraph(
        imGuiManager,
        camera,
        graphs[selectedGraphIndex].graph,
        selectedNodeId,
        selectedEdge,
        drawTheFlightItself ? JumpsDrawnAs::TheFlightItself : JumpsDrawnAs::SmoothArc);
}

void NavigationUi::drawEdgesOf(const NavigationGraph &graph, int nodeId)
{
    std::vector<std::pair<NavigationEdge, bool>> edges;
    for (const NavigationEdge &edge : graph.getOutgoingEdges(nodeId))
        edges.emplace_back(edge, true);

    for (const NavigationEdge &edge : graph.getEdges())
        if (edge.toId == nodeId)
            edges.emplace_back(edge, false);

    std::ranges::sort(
        edges,
        [](const auto &left, const auto &right)
        {
            const NavigationEdge &leftEdge = left.first;
            const NavigationEdge &rightEdge = right.first;
            return std::tie(leftEdge.type, left.second, leftEdge.fromId, leftEdge.toId) <
                   std::tie(rightEdge.type, right.second, rightEdge.fromId, rightEdge.toId);
        });

    for (const auto &[edge, leaving] : edges)
    {
        ImGui::PushID(leaving ? edge.toId : -edge.fromId - 1);
        std::pair<int, int> ends{edge.fromId, edge.toId};
        std::string label = std::string(nameOf(edge.type)) + " " + std::to_string(edge.fromId) +
                            " to " + std::to_string(edge.toId);

        if (ImGui::Selectable(label.c_str(), selectedEdge == ends))
        {
            bool wasSelected = selectedEdge == ends;
            selectedEdge.reset();
            selectedNodeId.reset();
            if (!wasSelected)
                selectedEdge = ends;
        }
        ImGui::PopID();
    }
}
