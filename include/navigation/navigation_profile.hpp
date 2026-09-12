#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "actor/abilities/abilities_data.hpp"
#include "physics/physics_body_data.hpp"
#include "navigation/jump_arc.hpp"

struct NavigationProfile
{
    std::vector<JumpArc> jumpArcs;
    AbilitiesData abilities;
    PhysicsBodyData physicsBodyData;

    bool falls() const
    {
        return abilities.gravity.has_value();
    }

    bool climbs() const
    {
        return abilities.wallHang.has_value() && abilities.wallClimb.has_value();
    }

    bool operator==(const NavigationProfile &) const = default;
};
