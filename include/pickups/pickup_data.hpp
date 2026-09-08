#pragma once

#include <optional>
#include <glm/gtc/matrix_transform.hpp>
#include "serialization/glm_vec2_meta.hpp" // IWYU pragma: keep
#include "assets/sheet_data.hpp"
#include "animations/frame_animation_data.hpp"

struct PickupData
{
    SheetData sheet;
    FrameAnimationData animationData;
    glm::vec2 size = glm::vec2(16, 16);
    std::optional<glm::vec2> colliderSize;
    glm::vec2 colliderOffset = glm::vec2(0, 0);
    int scoreDelta = 0;
};
