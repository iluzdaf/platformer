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
#include "ui/state_machine_graph.hpp"
#include "ui/state_machine_shown.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"

namespace
{
    constexpr float GraphHeight = 240.0f;
    constexpr float NodeHeight = 38.0f;
    constexpr float NodeMinWidth = 72.0f;
    constexpr float NodePadding = 8.0f;
    constexpr float NodeRounding = 5.0f;
    constexpr float RingMargin = 64.0f;
    constexpr float Bend = 22.0f;
    constexpr float LoopRadius = 16.0f;
    constexpr float ArrowLength = 8.0f;
    constexpr float ArrowWidth = 5.0f;
    constexpr float LabelPadding = 3.0f;
    constexpr ImU32 BackgroundColour = IM_COL32(24, 24, 28, 255);
    constexpr ImU32 FrameColour = IM_COL32(70, 70, 80, 255);
    constexpr ImU32 NodeFillColour = IM_COL32(48, 48, 56, 255);
    constexpr ImU32 NodeLitFillColour = IM_COL32(60, 120, 60, 255);
    constexpr ImU32 NodeBorderColour = IM_COL32(150, 150, 165, 255);
    constexpr ImU32 NodeLitBorderColour = IM_COL32(140, 255, 140, 255);
    constexpr ImU32 NameColour = IM_COL32(235, 235, 235, 255);
    constexpr ImU32 BehaviourColour = IM_COL32(170, 170, 180, 255);
    constexpr ImU32 EdgeColour = IM_COL32(150, 150, 165, 255);
    constexpr ImU32 EdgeLitColour = IM_COL32(140, 255, 140, 255);
    constexpr ImU32 LabelColour = IM_COL32(215, 215, 225, 255);
    constexpr ImU32 MissingColour = IM_COL32(255, 100, 100, 255);

    ImVec2 screen(glm::vec2 position)
    {
        return ImVec2(position.x, position.y);
    }

    struct Node
    {
        glm::vec2 centre;
        glm::vec2 half;
    };

    glm::vec2 halfOf(const BehaviorStateData &state)
    {
        float nameWidth = ImGui::CalcTextSize(state.name.c_str()).x;
        float behaviourWidth = ImGui::CalcTextSize(behaviourOf(state).c_str()).x;
        float width = std::max(NodeMinWidth, std::max(nameWidth, behaviourWidth) + 2 * NodePadding);
        return glm::vec2(width * 0.5f, NodeHeight * 0.5f);
    }

    glm::vec2 whereItLeaves(const Node &node, glm::vec2 direction)
    {
        float alongX = direction.x != 0.0f ? node.half.x / std::abs(direction.x) : 1e9f;
        float alongY = direction.y != 0.0f ? node.half.y / std::abs(direction.y) : 1e9f;
        return node.centre + direction * std::min(alongX, alongY);
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

    void drawLabel(ImDrawList *drawList, glm::vec2 at, const std::string &text)
    {
        ImVec2 size = ImGui::CalcTextSize(text.c_str());
        ImVec2 low(at.x - size.x * 0.5f - LabelPadding, at.y - size.y * 0.5f - LabelPadding);
        ImVec2 high(at.x + size.x * 0.5f + LabelPadding, at.y + size.y * 0.5f + LabelPadding);
        drawList->AddRectFilled(low, high, BackgroundColour, 2.0f);
        drawList->AddText(
            ImVec2(low.x + LabelPadding, low.y + LabelPadding), LabelColour, text.c_str());
    }

    void drawLoop(ImDrawList *drawList, const Node &node, ImU32 colour, const std::string &label)
    {
        glm::vec2 top = node.centre - glm::vec2(0.0f, node.half.y);
        ImVec2 centre = screen(top - glm::vec2(0.0f, LoopRadius));
        drawList->AddCircle(centre, LoopRadius, colour, 0, 1.5f);
        drawArrowHead(drawList, top + glm::vec2(LoopRadius, -2.0f), glm::vec2(0.0f, 1.0f), colour);
        drawLabel(drawList, top - glm::vec2(0.0f, 2.0f * LoopRadius + 8.0f), label);
    }

    void drawEdge(
        ImDrawList *drawList,
        const Node &from,
        const Node &to,
        bool bent,
        ImU32 colour,
        const std::string &label)
    {
        glm::vec2 between = to.centre - from.centre;
        if (glm::length(between) < 1.0f)
            return;

        glm::vec2 direction = glm::normalize(between);
        glm::vec2 normal(-direction.y, direction.x);
        glm::vec2 control = (from.centre + to.centre) * 0.5f + normal * (bent ? Bend : 0.0f);

        glm::vec2 start = whereItLeaves(from, glm::normalize(control - from.centre));
        glm::vec2 end = whereItLeaves(to, glm::normalize(control - to.centre));
        glm::vec2 arrives = glm::normalize(end - control);

        drawList->PathLineTo(screen(start));
        drawList->PathBezierQuadraticCurveTo(screen(control), screen(end - arrives * ArrowLength));
        drawList->PathStroke(colour, ImDrawFlags_None, 1.5f);
        drawArrowHead(drawList, end, arrives, colour);

        glm::vec2 middle = start * 0.25f + control * 0.5f + end * 0.25f;
        drawLabel(drawList, middle + normal * (bent ? 8.0f : 0.0f), label);
    }

    void drawNode(ImDrawList *drawList, const Node &node, const BehaviorStateData &state, bool lit)
    {
        ImVec2 low = screen(node.centre - node.half);
        ImVec2 high = screen(node.centre + node.half);
        drawList->AddRectFilled(low, high, lit ? NodeLitFillColour : NodeFillColour, NodeRounding);
        drawList->AddRect(
            low,
            high,
            lit ? NodeLitBorderColour : NodeBorderColour,
            NodeRounding,
            0,
            lit ? 2.0f : 1.0f);

        ImVec2 nameSize = ImGui::CalcTextSize(state.name.c_str());
        drawList->AddText(
            ImVec2(node.centre.x - nameSize.x * 0.5f, low.y + 4.0f),
            NameColour,
            state.name.c_str());

        std::string behaviour = behaviourOf(state);
        ImVec2 behaviourSize = ImGui::CalcTextSize(behaviour.c_str());
        drawList->AddText(
            ImVec2(node.centre.x - behaviourSize.x * 0.5f, high.y - behaviourSize.y - 3.0f),
            BehaviourColour,
            behaviour.c_str());
    }
}

void drawStateMachineGraph(
    const StateMachineBehaviorData &machine,
    const std::set<std::string> &litStates)
{
    if (machine.states.empty())
    {
        ImGui::TextDisabled("no states");
        return;
    }

    ImVec2 size(std::max(ImGui::GetContentRegionAvail().x, 1.0f), GraphHeight);
    ImVec2 at = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("##stateMachineGraph", size);

    ImDrawList *drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(at, ImVec2(at.x + size.x, at.y + size.y), BackgroundColour);
    drawList->AddRect(at, ImVec2(at.x + size.x, at.y + size.y), FrameColour);

    glm::vec2 centre(at.x + size.x * 0.5f, at.y + size.y * 0.5f);
    float radius = std::max(std::min(size.x, size.y) * 0.5f - RingMargin, 0.0f);
    std::vector<glm::vec2> ring = aRingOf(machine.states.size(), centre, radius);

    std::vector<Node> nodes;
    nodes.reserve(machine.states.size());
    for (std::size_t index = 0; index < machine.states.size(); ++index)
        nodes.push_back(Node{ring[index], halfOf(machine.states[index])});

    std::vector<std::string> missing;
    for (const BehaviorTransitionData &transition : machine.transitions)
    {
        std::optional<std::size_t> from = indexOfState(machine, transition.from);
        std::optional<std::size_t> to = indexOfState(machine, transition.to);
        if (!from || !to)
        {
            missing.push_back(transition.from + " -> " + transition.to);
            continue;
        }

        ImU32 colour = litStates.contains(transition.from) ? EdgeLitColour : EdgeColour;
        if (*from == *to)
            drawLoop(drawList, nodes[*from], colour, whenOf(transition));
        else
            drawEdge(
                drawList,
                nodes[*from],
                nodes[*to],
                goesBothWays(machine, transition),
                colour,
                whenOf(transition));
    }

    for (std::size_t index = 0; index < machine.states.size(); ++index)
        drawNode(
            drawList,
            nodes[index],
            machine.states[index],
            litStates.contains(machine.states[index].name));

    for (const std::string &edge : missing)
        ImGui::TextColored(
            ImGui::ColorConvertU32ToFloat4(MissingColour),
            "%s names a state that does not exist",
            edge.c_str());
}
