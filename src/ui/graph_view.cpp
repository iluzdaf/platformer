#include <algorithm>
#include <cmath>
#include <cstddef>
#include <optional>
#include <set>
#include <string>
#include <vector>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include "ui/graph_view.hpp"
#include "ui/graph_shown.hpp"
#include "ui/state_machine_shown.hpp"

namespace
{
    constexpr float GraphHeight = 220.0f;
    constexpr float NodeHeight = 26.0f;
    constexpr float NodeMinWidth = 64.0f;
    constexpr float NodePadding = 10.0f;
    constexpr float NodeRounding = 5.0f;
    constexpr float RingMargin = 48.0f;
    constexpr float Bend = 22.0f;
    constexpr float LoopRadius = 14.0f;
    constexpr float ArrowLength = 8.0f;
    constexpr float ArrowWidth = 5.0f;
    constexpr float HandleRadius = 4.0f;
    constexpr float PickWithin = 6.0f;
    constexpr ImU32 BackgroundColour = IM_COL32(24, 24, 28, 255);
    constexpr ImU32 FrameColour = IM_COL32(70, 70, 80, 255);
    constexpr ImU32 NodeFillColour = IM_COL32(48, 48, 56, 255);
    constexpr ImU32 NodeLitFillColour = IM_COL32(60, 120, 60, 255);
    constexpr ImU32 NodeBorderColour = IM_COL32(150, 150, 165, 255);
    constexpr ImU32 NodeLitBorderColour = IM_COL32(140, 255, 140, 255);
    constexpr ImU32 SelectedColour = IM_COL32(255, 210, 80, 255);
    constexpr ImU32 NameColour = IM_COL32(235, 235, 235, 255);
    constexpr ImU32 EdgeColour = IM_COL32(150, 150, 165, 255);
    constexpr ImU32 EdgeLitColour = IM_COL32(140, 255, 140, 255);
    constexpr ImU32 MissingColour = IM_COL32(255, 100, 100, 255);

    ImVec2 screen(glm::vec2 position)
    {
        return ImVec2(position.x, position.y);
    }

    struct Node
    {
        glm::vec2 centre;
        glm::vec2 half;

        bool contains(glm::vec2 point) const
        {
            return std::abs(point.x - centre.x) <= half.x && std::abs(point.y - centre.y) <= half.y;
        }
    };

    struct Curve
    {
        glm::vec2 start, control, end;
        std::optional<glm::vec2> loopCentre;

        float distanceTo(glm::vec2 point) const
        {
            if (loopCentre)
                return std::abs(glm::distance(point, *loopCentre) - LoopRadius);

            return distanceToCurve(point, start, control, end);
        }

        glm::vec2 handle() const
        {
            if (loopCentre)
                return *loopCentre - glm::vec2(0.0f, LoopRadius);

            return onCurve(0.5f, start, control, end);
        }
    };

    glm::vec2 halfOf(const GraphNode &node)
    {
        float width =
            std::max(NodeMinWidth, ImGui::CalcTextSize(node.name.c_str()).x + 2 * NodePadding);
        return glm::vec2(width * 0.5f, NodeHeight * 0.5f);
    }

    glm::vec2 whereItLeaves(const Node &node, glm::vec2 direction)
    {
        float alongX = direction.x != 0.0f ? node.half.x / std::abs(direction.x) : 1e9f;
        float alongY = direction.y != 0.0f ? node.half.y / std::abs(direction.y) : 1e9f;
        return node.centre + direction * std::min(alongX, alongY);
    }

    Curve curveBetween(const Node &from, const Node &to, bool bent)
    {
        glm::vec2 direction = glm::normalize(to.centre - from.centre);
        glm::vec2 normal(-direction.y, direction.x);
        glm::vec2 control = (from.centre + to.centre) * 0.5f + normal * (bent ? Bend : 0.0f);
        glm::vec2 start = whereItLeaves(from, glm::normalize(control - from.centre));
        glm::vec2 end = whereItLeaves(to, glm::normalize(control - to.centre));
        return Curve{start, control, end, std::nullopt};
    }

    Curve loopAbove(const Node &node)
    {
        glm::vec2 top = node.centre - glm::vec2(0.0f, node.half.y);
        return Curve{top, top, top, top - glm::vec2(0.0f, LoopRadius)};
    }

    Curve curveOf(
        const GraphShown &graph,
        const std::vector<Node> &nodes,
        const GraphEdge &edge,
        std::size_t from,
        std::size_t to)
    {
        if (from == to)
            return loopAbove(nodes[from]);

        return curveBetween(nodes[from], nodes[to], goesBothWays(graph, edge));
    }

    void drawArrowHead(ImDrawList *drawList, glm::vec2 tip, glm::vec2 direction, ImU32 colour)
    {
        glm::vec2 normal(-direction.y, direction.x);
        glm::vec2 base = tip - direction * ArrowLength;
        drawList->AddTriangleFilled(
            screen(tip),
            screen(base + normal * ArrowWidth),
            screen(base - normal * ArrowWidth),
            colour);
    }

    void drawCurve(ImDrawList *drawList, const Curve &curve, ImU32 colour, float thickness)
    {
        if (curve.loopCentre)
        {
            drawList->AddCircle(screen(*curve.loopCentre), LoopRadius, colour, 0, thickness);
            drawArrowHead(
                drawList,
                curve.start + glm::vec2(LoopRadius, -2.0f),
                glm::vec2(0.0f, 1.0f),
                colour);
        }
        else
        {
            glm::vec2 arrives = glm::normalize(curve.end - curve.control);
            drawList->PathLineTo(screen(curve.start));
            drawList->PathBezierQuadraticCurveTo(
                screen(curve.control), screen(curve.end - arrives * ArrowLength));
            drawList->PathStroke(colour, ImDrawFlags_None, thickness);
            drawArrowHead(drawList, curve.end, arrives, colour);
        }

        drawList->AddCircleFilled(screen(curve.handle()), HandleRadius, colour);
    }

    void drawNode(
        ImDrawList *drawList,
        const Node &node,
        const GraphNode &shown,
        bool lit,
        bool selected)
    {
        ImVec2 low = screen(node.centre - node.half);
        ImVec2 high = screen(node.centre + node.half);
        drawList->AddRectFilled(low, high, lit ? NodeLitFillColour : NodeFillColour, NodeRounding);
        ImU32 border = selected ? SelectedColour : lit ? NodeLitBorderColour : NodeBorderColour;
        drawList->AddRect(low, high, border, NodeRounding, 0, selected || lit ? 2.0f : 1.0f);

        ImVec2 nameSize = ImGui::CalcTextSize(shown.name.c_str());
        drawList->AddText(
            ImVec2(node.centre.x - nameSize.x * 0.5f, node.centre.y - nameSize.y * 0.5f),
            NameColour,
            shown.name.c_str());
    }

    struct Drawn
    {
        std::vector<Node> nodes;
        std::vector<std::optional<Curve>> curves;
        std::vector<std::string> missing;
    };

    Drawn layOut(const GraphShown &graph, glm::vec2 centre, float radius)
    {
        Drawn drawn;
        std::vector<glm::vec2> ring = aRingOf(graph.nodes.size(), centre, radius);
        drawn.nodes.reserve(graph.nodes.size());
        for (std::size_t index = 0; index < graph.nodes.size(); ++index)
            drawn.nodes.push_back(Node{ring[index], halfOf(graph.nodes[index])});

        drawn.curves.reserve(graph.edges.size());
        for (const GraphEdge &edge : graph.edges)
        {
            std::optional<std::size_t> from = indexOfNode(graph, edge.from);
            std::optional<std::size_t> to = indexOfNode(graph, edge.to);
            if (from && to)
                drawn.curves.push_back(curveOf(graph, drawn.nodes, edge, *from, *to));
            else
            {
                drawn.curves.push_back(std::nullopt);
                drawn.missing.push_back(edge.from + " -> " + edge.to);
            }
        }

        return drawn;
    }

    MachineShown whatIsAt(glm::vec2 point, const Drawn &drawn)
    {
        for (std::size_t index = 0; index < drawn.nodes.size(); ++index)
            if (drawn.nodes[index].contains(point))
                return showingState(index);

        for (std::size_t index = 0; index < drawn.curves.size(); ++index)
        {
            const std::optional<Curve> &curve = drawn.curves[index];
            if (curve.has_value() && curve.value().distanceTo(point) <= PickWithin)
                return showingTransition(index);
        }

        return MachineShown{};
    }

    void describe(const GraphShown &graph, MachineShown hovered)
    {
        if (hovered.what == MachineShown::What::State)
            ImGui::SetTooltip("%s", graph.nodes[hovered.index].words.c_str());
        else if (hovered.what == MachineShown::What::Transition)
            ImGui::SetTooltip("%s", graph.edges[hovered.index].words.c_str());
    }
}

MachineShown drawGraph(
    const GraphShown &graph,
    const std::set<std::string> &litNodes,
    MachineShown selected)
{
    if (graph.nodes.empty())
    {
        ImGui::TextDisabled("nothing to draw");
        return MachineShown{};
    }

    ImVec2 size(std::max(ImGui::GetContentRegionAvail().x, 1.0f), GraphHeight);
    ImVec2 at = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("##graph", size);
    bool hovered = ImGui::IsItemHovered();
    bool clicked = ImGui::IsItemClicked();
    glm::vec2 mouse(ImGui::GetMousePos().x, ImGui::GetMousePos().y);

    ImDrawList *drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(at, ImVec2(at.x + size.x, at.y + size.y), BackgroundColour);
    drawList->AddRect(at, ImVec2(at.x + size.x, at.y + size.y), FrameColour);

    glm::vec2 centre(at.x + size.x * 0.5f, at.y + size.y * 0.5f);
    float radius = std::max(std::min(size.x, size.y) * 0.5f - RingMargin, 0.0f);
    Drawn drawn = layOut(graph, centre, radius);

    if (clicked)
        selected = whatIsAt(mouse, drawn);

    for (std::size_t index = 0; index < drawn.curves.size(); ++index)
    {
        const std::optional<Curve> &curve = drawn.curves[index];
        if (!curve.has_value())
            continue;

        bool chosen = selected == showingTransition(index);
        bool lit = litNodes.contains(graph.edges[index].from);
        ImU32 colour = chosen ? SelectedColour : lit ? EdgeLitColour : EdgeColour;
        drawCurve(drawList, curve.value(), colour, chosen ? 2.5f : 1.5f);
    }

    for (std::size_t index = 0; index < drawn.nodes.size(); ++index)
        drawNode(
            drawList,
            drawn.nodes[index],
            graph.nodes[index],
            litNodes.contains(graph.nodes[index].name),
            selected == showingState(index));

    if (hovered)
        describe(graph, whatIsAt(mouse, drawn));

    for (std::size_t index = 0; index < drawn.curves.size(); ++index)
    {
        if (drawn.curves[index])
            continue;

        ImGui::PushStyleColor(ImGuiCol_Text, MissingColour);
        std::string label = graph.edges[index].from + " -> " + graph.edges[index].to +
                            " names a node that does not exist";
        if (ImGui::Selectable(label.c_str(), selected == showingTransition(index)))
            selected = showingTransition(index);

        ImGui::PopStyleColor();
    }

    return selected;
}
