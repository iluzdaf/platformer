#pragma once

#include <string_view>
#include <vector>

struct AbilityStates;
struct Observed;

std::vector<std::string_view> cuesOf(
    const AbilityStates &states,
    const Observed &observed,
    float fallFromHeightThreshold);
