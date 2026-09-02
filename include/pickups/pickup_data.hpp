#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "serialization/glm_vec2_meta.hpp" // IWYU pragma: keep
#include "assets/sheet.hpp"
#include "animations/frame_animation_data.hpp"

struct PickupData
{
    Sheet sheet;
    FrameAnimationData animationData;
    glm::vec2 size = glm::vec2(16, 16);
    int scoreDelta = 0;
};
