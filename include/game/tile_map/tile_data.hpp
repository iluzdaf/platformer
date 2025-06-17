#pragma once
#include <optional>
#include "serialization/glm_vec2_meta.hpp"
#include "game/tile_map/tile_kind.hpp"
#include "animations/tile_animation_data.hpp"

struct TileData
{
    TileKind kind;
    std::optional<TileAnimationData> animationData;
    std::optional<int> pickupReplaceIndex, pickupScoreDelta;
    glm::vec2 colliderOffset = glm::vec2(0.0f, 0.0f),
              colliderSize = glm::vec2(16.0f, 16.0f);
};