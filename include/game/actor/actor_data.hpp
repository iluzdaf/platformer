#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "serialization/glm_vec2_meta.hpp" // IWYU pragma: keep
#include "game/actor/actor_motion_data.hpp"
#include "game/actor/actor_animation_data.hpp"
#include "physics/physics_body_data.hpp"

struct ActorData
{
    glm::vec2 size = glm::vec2(16, 16);
    PhysicsBodyData physicsBodyData;
    ActorMotionData motionData;
    ActorAnimationData animationData;
};
