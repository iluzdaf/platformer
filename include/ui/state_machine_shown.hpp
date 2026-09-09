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

std::string behaviourOf(const BehaviorStateData &state);

std::string whenOf(const BehaviorTransitionData &transition);

std::vector<glm::vec2> aRingOf(std::size_t count, glm::vec2 centre, float radius);

std::optional<std::size_t> indexOfState(
    const StateMachineBehaviorData &machine,
    std::string_view name);

bool goesBothWays(
    const StateMachineBehaviorData &machine,
    const BehaviorTransitionData &transition);
