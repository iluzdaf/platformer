#pragma once

#include <optional>
#include "combat/hurting.hpp"
#include "physics/aabb.hpp"

struct AbilityStates;

std::optional<Hurting> hurtingFrom(const AbilityStates &states, const AABB &body);
