#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "tile_map/tile_map.hpp"

inline constexpr int TestTileSize = 16;

inline glm::vec2 topLeftOf(glm::ivec2 tile)
{
    return topLeftOfTile(tile, TestTileSize);
}

inline glm::vec2 feetOf(glm::ivec2 tile)
{
    return feetOnTile(tile, TestTileSize);
}

inline glm::vec2 middleOf(glm::ivec2 tile)
{
    return middleOfTile(tile, TestTileSize);
}

inline float surfaceOf(int row)
{
    return static_cast<float>(row * TestTileSize);
}
