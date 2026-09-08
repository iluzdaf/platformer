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
    std::optional<glm::vec2> size;
    std::optional<glm::vec2> colliderSize;
    glm::vec2 colliderOffset = glm::vec2(0, 0);
    int scoreDelta = 0;
};

inline glm::vec2 drawnSizeOf(const PickupData &pickupData)
{
    return pickupData.size.value_or(glm::vec2(pickupData.sheet.cellSize));
}
