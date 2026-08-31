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
    if (!ImGui::TreeNodeEx("Navigation", ImGuiTreeNodeFlags_DefaultOpen))
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

    if (selectedGraphIndex && *selectedGraphIndex >= graphs.size())
        forgetSelection();

    ImGui::TextUnformatted("graph");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo(
            "##graph", selectedGraphIndex ? graphs[*selectedGraphIndex].name.c_str() : "none"))
    {
        if (ImGui::Selectable("none", !selectedGraphIndex))
            forgetSelection();

        for (size_t index = 0; index < graphs.size(); ++index)
            if (ImGui::Selectable(graphs[index].name.c_str(), selectedGraphIndex == index))
            {
                forgetSelection();
                selectedGraphIndex = index;
            }

        ImGui::EndCombo();
    }

    const NavigationGraph *graph =
        selectedGraphIndex ? &graphs[*selectedGraphIndex].graph : nullptr;

    std::vector<int> nodeIds;
    if (graph)
        for (const auto &[nodeId, node] : graph->getNodes())
            nodeIds.push_back(nodeId);
    std::sort(nodeIds.begin(), nodeIds.end());

    std::string shownNode = "all";
    if (graph && selectedNodeId)
        shownNode = std::to_string(*selectedNodeId);

    ImGui::BeginDisabled(!graph);
    ImGui::TextUnformatted("node");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##node", shownNode.c_str()))
    {
        if (ImGui::Selectable("all", !selectedNodeId))
        {
            selectedNodeId.reset();
            shownEdge.reset();
        }

        for (int nodeId : nodeIds)
            if (ImGui::Selectable(std::to_string(nodeId).c_str(), selectedNodeId == nodeId))
            {
                selectedNodeId = nodeId;
                shownEdge.reset();
            }

        ImGui::EndCombo();
    }
    ImGui::EndDisabled();

    drawEdgeChooser(graph, selectedNodeId);
}

void NavigationUi::forgetSelection()
{
    selectedGraphIndex.reset();
    selectedNodeId.reset();
    shownEdge.reset();
}

void NavigationUi::drawOverlay(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const Level &level) const
{
    const std::vector<NamedNavigationGraph> &graphs = level.getGraphs();
    if (!selectedGraphIndex || *selectedGraphIndex >= graphs.size())
        return;

    drawNavigationGraph(
        imGuiManager,
        camera,
        graphs[*selectedGraphIndex].graph,
        selectedNodeId,
        shownEdge,
        drawTheFlightItself ? JumpsDrawnAs::TheFlightItself : JumpsDrawnAs::SmoothArc);
}

void NavigationUi::drawEdgeChooser(const NavigationGraph *graph, std::optional<int> nodeId)
{
    std::vector<std::pair<NavigationEdge, bool>> edges;
    if (graph && nodeId)
        edges = edgesOf(*graph, *nodeId);

    std::string chosen = "all";
    for (const auto &[edge, leaving] : edges)
        if (shownEdge == std::pair<int, int>{edge.fromId, edge.toId})
            chosen = labelOf(edge);

    ImGui::BeginDisabled(edges.empty());
    ImGui::TextUnformatted("edge");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##edge", chosen.c_str()))
    {
        if (ImGui::Selectable("all", !shownEdge))
            shownEdge.reset();

        for (const auto &[edge, leaving] : edges)
        {
            std::pair<int, int> ends{edge.fromId, edge.toId};
            if (ImGui::Selectable(labelOf(edge).c_str(), shownEdge == ends))
                shownEdge = ends;
        }

        ImGui::EndCombo();
    }
    ImGui::EndDisabled();
}
