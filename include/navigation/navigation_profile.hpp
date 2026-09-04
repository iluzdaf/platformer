#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "actor/actor_motion_data.hpp"
#include "physics/physics_body_data.hpp"
#include "navigation/jump_arc.hpp"

struct NavigationProfile
{
    std::vector<JumpArc> jumpArcs;
    ActorMotionData motionData;
    PhysicsBodyData physicsBodyData;

    bool falls() const
    {
        return motionData.gravityAbilityData.has_value();
    }

    bool climbs() const
    {
        return motionData.wallHangAbilityData.has_value() &&
               motionData.wallClimbAbilityData.has_value();
    }

    bool operator==(const NavigationProfile &) const = default;
};
