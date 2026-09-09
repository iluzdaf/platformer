#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>

struct BehaviorStateData;
struct BehaviorTransitionData;
struct StateMachineBehaviorData;

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

std::string whenOf(const BehaviorTransitionData &transition);

std::vector<glm::vec2> aRingOf(std::size_t count, glm::vec2 centre, float radius);

std::optional<std::size_t> indexOfState(
    const StateMachineBehaviorData &machine,
    std::string_view name);

bool goesBothWays(
    const StateMachineBehaviorData &machine,
    const BehaviorTransitionData &transition);
