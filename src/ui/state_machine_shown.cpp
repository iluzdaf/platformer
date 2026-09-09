#include <cmath>
#include <cstddef>
#include <format>
#include <numbers>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "ui/state_machine_shown.hpp"
#include "actor/behaviors/attack_behavior_data.hpp"
#include "actor/behaviors/chase_behavior_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"

namespace
{
    std::string joined(const std::vector<std::string> &parts, std::string_view none)
    {
        if (parts.empty())
            return std::string(none);

        std::string text = parts.front();
        for (std::size_t index = 1; index < parts.size(); ++index)
            text += ", " + parts[index];

        return text;
    }
}

std::string behaviourOf(const BehaviorStateData &state)
{
    std::vector<std::string> parts;
    if (state.patrolBehaviorData)
        parts.push_back("patrol");

    if (state.fleeBehaviorData)
        parts.push_back("flee");

    if (state.chaseBehaviorData)
        parts.push_back(
            state.chaseBehaviorData->standoff > 0.0f
                ? std::format("chase, standoff {}", state.chaseBehaviorData->standoff)
                : "chase");

    if (state.attackBehaviorData)
        parts.push_back("attack with " + state.attackBehaviorData->with);

    if (state.cooldown > 0.0f)
        parts.push_back(std::format("cooldown {} s", state.cooldown));

    return joined(parts, "does nothing");
}

std::string whenOf(const BehaviorTransitionData &transition)
{
    std::vector<std::string> parts;
    if (transition.threatWithin)
        parts.push_back(std::format("threat within {}", *transition.threatWithin));

    if (transition.threatBeyond)
        parts.push_back(std::format("threat beyond {}", *transition.threatBeyond));

    if (transition.threatOnMySurface)
        parts.emplace_back(*transition.threatOnMySurface ? "on my surface" : "off my surface");

    if (transition.cornered)
        parts.emplace_back(*transition.cornered ? "cornered" : "not cornered");

    if (transition.onGround)
        parts.emplace_back(*transition.onGround ? "on ground" : "in the air");

    if (transition.after > 0.0f)
        parts.push_back(std::format("after {} s", transition.after));

    return joined(parts, "always");
}

std::vector<glm::vec2> aRingOf(std::size_t count, glm::vec2 centre, float radius)
{
    if (count == 1)
        return {centre};

    std::vector<glm::vec2> positions;
    for (std::size_t index = 0; index < count; ++index)
    {
        float turn = static_cast<float>(index) / static_cast<float>(count);
        float angle = -std::numbers::pi_v<float> / 2.0f + turn * 2.0f * std::numbers::pi_v<float>;
        positions.emplace_back(
            centre.x + radius * std::cos(angle), centre.y + radius * std::sin(angle));
    }

    return positions;
}

std::optional<std::size_t> indexOfState(
    const StateMachineBehaviorData &machine,
    std::string_view name)
{
    for (std::size_t index = 0; index < machine.states.size(); ++index)
        if (machine.states[index].name == name)
            return index;

    return std::nullopt;
}

bool goesBothWays(const StateMachineBehaviorData &machine, const BehaviorTransitionData &transition)
{
    for (const BehaviorTransitionData &other : machine.transitions)
        if (other.from == transition.to && other.to == transition.from)
            return true;

    return false;
}
