#include <cstddef>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include "ui/graph_shown.hpp"
#include "ui/state_machine_shown.hpp"
#include "actor/actor_animation_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "animations/animator_data.hpp"
#include "animations/frame_animation_data.hpp"

namespace
{
    void say(
        std::vector<std::string> &parts,
        std::optional<bool> asked,
        const char *yes,
        const char *no)
    {
        if (asked)
            parts.emplace_back(*asked ? yes : no);
    }

    std::string joined(const std::vector<std::string> &parts, std::string_view none)
    {
        if (parts.empty())
            return std::string(none);

        std::string text = parts.front();
        for (std::size_t index = 1; index < parts.size(); ++index)
            text += ", " + parts[index];

        return text;
    }

    std::string wordsOf(const FrameAnimationData &clip)
    {
        return std::format(
            "{} frame{}, {} s each{}",
            clip.frames.size(),
            clip.frames.size() == 1 ? "" : "s",
            clip.frameDuration,
            clip.loops ? "" : ", once");
    }

    bool anyRungLeavesFromAnywhere(const AnimatorData &ladder)
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

std::string whenOf(const AnimationWhen &when)
{
    std::vector<std::string> parts;
    say(parts, when.alive, "alive", "dead");
    say(parts, when.knockback, "knocked back", "not knocked back");
    say(parts, when.swinging, "swinging", "not swinging");
    say(parts, when.dashing, "dashing", "not dashing");
    say(parts, when.onGround, "on ground", "in the air");
    say(parts, when.climbing, "climbing", "not climbing");
    say(parts, when.onWall, "on a wall", "off the wall");
    say(parts, when.rising, "rising", "not rising");
    say(parts, when.falling, "falling", "not falling");
    say(parts, when.moving, "moving", "still");
    say(parts, when.finished, "clip finished", "clip playing");
    return joined(parts, "always");
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

GraphShown graphOf(const ActorAnimationData &animations)
{
    GraphShown graph;
    if (const FrameAnimationData *idle = clipNamed(animations, IdleClip))
        graph.nodes.push_back({std::string(IdleClip), wordsOf(*idle)});

    for (const auto &[name, clip] : animations.clips)
        if (name != IdleClip)
            graph.nodes.push_back({name, wordsOf(clip)});

    if (anyRungLeavesFromAnywhere(animations.ladder))
        graph.nodes.push_back({std::string(AnyNode), "from whatever is playing"});

    graph.edges.reserve(animations.ladder.transitions.size());
    for (const AnimationTransitionData &rung : animations.ladder.transitions)
        graph.edges.push_back(
            {rung.from.empty() ? std::string(AnyNode) : rung.from, rung.to, whenOf(rung.when)});

    return graph;
}
