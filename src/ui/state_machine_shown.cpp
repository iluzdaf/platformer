#include <cmath>
#include <cstddef>
#include <format>
#include <numbers>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>
#include <algorithm>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ui/state_machine_shown.hpp"
#include "actor/behaviors/attack_behavior_data.hpp"
#include "actor/behaviors/chase_behavior_data.hpp"
#include "actor/behaviors/flee_behavior_data.hpp"
#include "actor/behaviors/patrol_behavior_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "actor/behaviors/behavior_facts.hpp"
#include "conditions/fact_rows.hpp"

namespace
{
}

std::string behaviourOf(const BehaviorStateData &state)
{
    std::string does = std::visit(
        [](const auto &held) -> std::string
        {
            using Does = std::remove_cvref_t<decltype(held)>;
            if constexpr (std::is_same_v<Does, PatrolBehaviorData>)
                return "patrol";
            else if constexpr (std::is_same_v<Does, FleeBehaviorData>)
                return "flee";
            else if constexpr (std::is_same_v<Does, ChaseBehaviorData>)
                return held.standoff > 0.0f ? std::format("chase, standoff {}", held.standoff)
                                            : "chase";
            else if constexpr (std::is_same_v<Does, AttackBehaviorData>)
                return "attack with " + held.with;
            else
                return "does nothing";
        },
        state.does);

    if (state.cooldown > 0.0f)
        does += std::format(", cooldown {} s", state.cooldown);

    return does;
}

std::string whenOf(const BehaviorTransitionData &transition)
{
    std::string text = whenOf(transition.when, behaviorRows());
    if (transition.after <= 0.0f)
        return text;

    std::string after = std::format("after {} s", transition.after);
    return text == "always" ? after : text + ", " + after;
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

MachineShown showingState(std::size_t index)
{
    return MachineShown{MachineShown::What::State, index};
}

MachineShown showingTransition(std::size_t index)
{
    return MachineShown{MachineShown::What::Transition, index};
}

MachineShown stillAmong(MachineShown shown, const StateMachineBehaviorData &machine)
{
    if (shown.what == MachineShown::What::State && shown.index >= machine.states.size())
        return MachineShown{};

    if (shown.what == MachineShown::What::Transition && shown.index >= machine.transitions.size())
        return MachineShown{};

    return shown;
}

glm::vec2 onCurve(float along, glm::vec2 start, glm::vec2 control, glm::vec2 end)
{
    float left = 1.0f - along;
    return start * (left * left) + control * (2.0f * left * along) + end * (along * along);
}

float distanceToCurve(glm::vec2 point, glm::vec2 start, glm::vec2 control, glm::vec2 end)
{
    constexpr int Samples = 24;
    float nearest = glm::distance(point, start);
    for (int sample = 1; sample <= Samples; ++sample)
    {
        float along = static_cast<float>(sample) / static_cast<float>(Samples);
        nearest = std::min(nearest, glm::distance(point, onCurve(along, start, control, end)));
    }

    return nearest;
}
