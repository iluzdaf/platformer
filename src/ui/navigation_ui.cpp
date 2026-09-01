#include <algorithm>
#include <cfloat>
#include <cstddef>
#include <optional>
#include <utility>
#include <string>
#include <string_view>
#include <vector>
#include <imgui.h>
#include "ui/navigation_ui.hpp"
#include "ui/navigation_shown.hpp"
#include "ui/navigation_overlay.hpp"
#include "navigation/navigation_edge.hpp"
#include "navigation/navigation_graph.hpp"
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

    std::string labelOf(const NavigationEdge &edge)
    {
        return std::string(nameOf(edge.type)) + " " + std::to_string(edge.fromId) + " to " +
               std::to_string(edge.toId);
    }

    std::vector<std::pair<NavigationEdge, bool>> edgesOf(const NavigationGraph &graph, int nodeId)
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

        return edges;
    }

}

void NavigationUi::drawOverlayToggles()
{
    ImGui::Checkbox("Real Trajectory", &drawTheFlightItself);
}

void NavigationUi::draw(const Level &level)
{
    if (!ImGui::TreeNodeEx("Navigation"))
        return;

    drawGraphs(level);

    ImGui::TreePop();
}

void NavigationUi::drawGraphs(const Level &level)
{
    const std::vector<NamedNavigationGraph> &graphs = level.getGraphs();
    if (graphs.empty())
    {
        ImGui::TextDisabled("no graphs");
        return;
    }

    shown = stillAmong(shown, graphs.size());

    ImGui::TextUnformatted("graph");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo(
            "##graph", shown.graphIndex ? graphs[*shown.graphIndex].name.c_str() : "none"))
    {
        if (ImGui::Selectable("none", !shown.graphIndex))
            shown = showingGraph(std::nullopt);

        for (size_t index = 0; index < graphs.size(); ++index)
            if (ImGui::Selectable(graphs[index].name.c_str(), shown.graphIndex == index))
                shown = showingGraph(index);

        ImGui::EndCombo();
    }

    const NavigationGraph *graph = shown.graphIndex ? &graphs[*shown.graphIndex].graph : nullptr;

    std::vector<int> nodeIds;
    if (graph)
        for (const auto &[nodeId, node] : graph->getNodes())
            nodeIds.push_back(nodeId);
    std::sort(nodeIds.begin(), nodeIds.end());

    std::string shownNode = "all";
    if (graph && shown.nodeId)
        shownNode = std::to_string(*shown.nodeId);

    ImGui::BeginDisabled(!graph);
    ImGui::TextUnformatted("node");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##node", shownNode.c_str()))
    {
        if (ImGui::Selectable("all", !shown.nodeId))
            shown = showingNode(shown, std::nullopt);

        for (int nodeId : nodeIds)
            if (ImGui::Selectable(std::to_string(nodeId).c_str(), shown.nodeId == nodeId))
                shown = showingNode(shown, nodeId);

        ImGui::EndCombo();
    }
    ImGui::EndDisabled();

    drawEdgeChooser(graph);
}

void NavigationUi::drawOverlay(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const Level &level) const
{
    const std::vector<NamedNavigationGraph> &graphs = level.getGraphs();
    if (!shown.graphIndex || *shown.graphIndex >= graphs.size())
        return;

    drawNavigationGraph(
        imGuiManager,
        camera,
        graphs[*shown.graphIndex].graph,
        shown.nodeId,
        shown.edge,
        drawTheFlightItself ? JumpsDrawnAs::TheFlightItself : JumpsDrawnAs::SmoothArc);
}

void NavigationUi::drawEdgeChooser(const NavigationGraph *graph)
{
    std::vector<std::pair<NavigationEdge, bool>> edges;
    if (graph && shown.nodeId)
        edges = edgesOf(*graph, *shown.nodeId);

    std::string chosen = "all";
    for (const auto &[edge, leaving] : edges)
        if (shown.edge == std::pair<int, int>{edge.fromId, edge.toId})
            chosen = labelOf(edge);

    ImGui::BeginDisabled(edges.empty());
    ImGui::TextUnformatted("edge");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##edge", chosen.c_str()))
    {
        if (ImGui::Selectable("all", !shown.edge))
            shown = showingEdge(shown, std::nullopt);

        for (const auto &[edge, leaving] : edges)
        {
            std::pair<int, int> ends{edge.fromId, edge.toId};
            if (ImGui::Selectable(labelOf(edge).c_str(), shown.edge == ends))
                shown = showingEdge(shown, ends);
        }

        ImGui::EndCombo();
    }
    ImGui::EndDisabled();
}
