#pragma once

#include "actor/behaviors/state_machine_behavior_data.hpp"
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "state_machines/state_machine_data.hpp"

struct MachineShown
{
    enum class What
    {
        Nothing,
        State,
        Transition
    };

    What what = What::Nothing;
    std::size_t index = 0;

    bool operator==(const MachineShown &) const = default;
};

MachineShown showingState(std::size_t index);

MachineShown showingTransition(std::size_t index);

MachineShown stillAmong(MachineShown shown, const StateMachineBehaviorData &machine);

glm::vec2 onCurve(float along, glm::vec2 start, glm::vec2 control, glm::vec2 end);

float distanceToCurve(glm::vec2 point, glm::vec2 start, glm::vec2 control, glm::vec2 end);

std::string behaviourOf(const BehaviorStateData &state);

std::string whenOf(const TransitionData &transition);

std::vector<glm::vec2> aRingOf(std::size_t count, glm::vec2 centre, float radius);

std::optional<std::size_t> indexOfState(
    const StateMachineBehaviorData &machine,
    std::string_view name);

bool goesBothWays(const StateMachineBehaviorData &machine, const TransitionData &transition);
