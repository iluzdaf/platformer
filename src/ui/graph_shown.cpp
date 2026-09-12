#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numbers>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "ui/graph_shown.hpp"
#include "ui/state_machine_shown.hpp"
#include "animations/animation_ladder_data.hpp"
#include "animations/animator_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "animations/animator_data.hpp"
#include "animations/animator_facts.hpp"
#include "conditions/asked.hpp"
#include "conditions/fact_rows.hpp"
#include "animations/frame_animation_data.hpp"

namespace
{
    constexpr float LeastGraphHeight = 220.0f;
    constexpr float RoomPerNode = 44.0f;

    std::string wordsOf(const FrameAnimationData &clip)
    {
        return std::format(
            "{} frame{}, {} s each{}",
            clip.frames.size(),
            clip.frames.size() == 1 ? "" : "s",
            clip.frameDuration,
            clip.loops ? "" : ", once");
    }

    bool anyRungLeavesFromAnywhere(const AnimationLadderData &ladder)
    {
        for (const AnimationTransitionData &rung : ladder.transitions)
            if (rung.from.empty())
                return true;

        return false;
    }
}

std::optional<std::size_t> indexOfNode(const GraphShown &graph, std::string_view name)
{
    for (std::size_t index = 0; index < graph.nodes.size(); ++index)
        if (graph.nodes[index].name == name)
            return index;

    return std::nullopt;
}

bool goesBothWays(const GraphShown &graph, const GraphEdge &edge)
{
    for (const GraphEdge &other : graph.edges)
        if (other.from == edge.to && other.to == edge.from)
            return true;

    return false;
}

MachineShown stillAmong(MachineShown shown, const GraphShown &graph)
{
    if (shown.what == MachineShown::What::State && shown.index >= graph.nodes.size())
        return MachineShown{};

    if (shown.what == MachineShown::What::Transition && shown.index >= graph.edges.size())
        return MachineShown{};

    return shown;
}

std::size_t nodesAroundIn(const GraphShown &graph)
{
    std::size_t around = 0;
    for (const GraphNode &node : graph.nodes)
        if (!node.hub)
            ++around;

    return around;
}

float graphHeightFor(std::size_t ringCount)
{
    return std::max(LeastGraphHeight, RoomPerNode * static_cast<float>(ringCount));
}

std::vector<glm::vec2> placedAround(const GraphShown &graph, glm::vec2 centre, glm::vec2 radii)
{
    std::size_t around = nodesAroundIn(graph);
    std::vector<glm::vec2> positions(graph.nodes.size(), centre);
    std::size_t placed = 0;
    for (std::size_t index = 0; index < graph.nodes.size(); ++index)
    {
        if (graph.nodes[index].hub || around == 1)
            positions[index] = centre;
        else
        {
            float turn = static_cast<float>(placed++) / static_cast<float>(around);
            float angle =
                -std::numbers::pi_v<float> / 2.0f + turn * 2.0f * std::numbers::pi_v<float>;
            positions[index] = glm::vec2(
                centre.x + radii.x * std::cos(angle), centre.y + radii.y * std::sin(angle));
        }
    }

    return positions;
}

std::string whenOf(const AnimationWhenData &when)
{
    return whenOf(when, animatorRows());
}

GraphShown graphOf(const StateMachineBehaviorData &machine)
{
    GraphShown graph;
    graph.nodes.reserve(machine.states.size());
    for (const BehaviorStateData &state : machine.states)
        graph.nodes.push_back({state.name, behaviourOf(state)});

    graph.edges.reserve(machine.transitions.size());
    for (const BehaviorTransitionData &transition : machine.transitions)
        graph.edges.push_back({transition.from, transition.to, whenOf(transition)});

    return graph;
}

GraphShown graphOf(const AnimatorData &animations)
{
    GraphShown graph;
    for (const auto &[name, clip] : animations.clips)
        graph.nodes.push_back({name, wordsOf(clip), false, name == animations.startClip});

    if (anyRungLeavesFromAnywhere(animations.ladder))
        graph.nodes.push_back({std::string(AnyNode), "from whatever is playing", true});

    graph.edges.reserve(animations.ladder.transitions.size());
    for (const AnimationTransitionData &rung : animations.ladder.transitions)
        graph.edges.push_back(
            {rung.from.empty() ? std::string(AnyNode) : rung.from, rung.to, whenOf(rung.when)});

    return graph;
}
