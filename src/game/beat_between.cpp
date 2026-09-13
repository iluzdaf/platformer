#include <stdexcept>
#include <utility>
#include "game/beat_between.hpp"
#include "tile_map/tile_map.hpp"
#include "actor/behaviors/patrol_data.hpp"

PatrolData beatBetween(glm::ivec2 fromTile, glm::ivec2 toTile, int tileSize)
{
    glm::vec2 from = feetOnTile(fromTile, tileSize);
    glm::vec2 to = feetOnTile(toTile, tileSize);
    float outwards = static_cast<float>(tileSize) * 0.5f;

    if (from.x <= to.x)
    {
        from.x -= outwards;
        to.x += outwards;
    }
    else
    {
        from.x += outwards;
        to.x -= outwards;
    }

    return PatrolData{from, to};
}

PatrolData beatBetween(const TileMap &tileMap, glm::ivec2 fromTile, glm::ivec2 toTile)
{
    if (!tileMap.validTilePosition(fromTile) || !tileMap.validTilePosition(toTile))
        throw std::runtime_error("Tile coordinates out of bounds");

    return beatBetween(fromTile, toTile, tileMap.getTileSize());
}

std::pair<glm::ivec2, glm::ivec2> tilesOfBeat(const TileMap &tileMap, const PatrolData &beat)
{
    glm::vec2 from = beat.from;
    glm::vec2 to = beat.to;
    float inwards = static_cast<float>(tileMap.getTileSize()) * 0.5f;

    if (from.x <= to.x)
    {
        from.x += inwards;
        to.x -= inwards;
    }
    else
    {
        from.x -= inwards;
        to.x += inwards;
    }

    return std::pair(tileMap.tileUnderFeet(from), tileMap.tileUnderFeet(to));
}
