#include <utility>
#include "npc/walk_between.hpp"
#include "npc/npc_spawn_data.hpp"
#include "tile_map/tile_map.hpp"

std::pair<glm::vec2, glm::vec2> walkBetween(const TileMap &tileMap, const PatrolData &patrol)
{
    glm::vec2 from = tileMap.feetOnTile(patrol.from);
    glm::vec2 to = tileMap.feetOnTile(patrol.to);
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

    return std::pair(from, to);
}
