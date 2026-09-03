#include <utility>
#include "game/beat_between.hpp"
#include "npc/npc_spawn_data.hpp"
#include "tile_map/tile_map.hpp"

PatrolData beatBetween(const TileMap &tileMap, glm::ivec2 fromTile, glm::ivec2 toTile)
{
    glm::vec2 from = tileMap.feetOnTile(fromTile);
    glm::vec2 to = tileMap.feetOnTile(toTile);
    float outwards = static_cast<float>(tileMap.getTileSize()) * 0.5f;

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
