#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "actor/actor_motion_data.hpp"
#include "physics/physics_body_data.hpp"
#include "navigation/jump_arc.hpp"

struct NavigationProfile
{
    glm::vec2 colliderSize = glm::vec2(8.0f, 13.0f);
    std::vector<JumpArc> jumpArcs;

    std::optional<ActorMotionData> motionData;
    std::optional<PhysicsBodyData> physicsBodyData;

    bool falls() const
    {
        return motionData && motionData->gravityAbilityData.has_value();
    }

    bool operator==(const NavigationProfile &) const = default;
};
