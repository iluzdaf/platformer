#include <optional>
#include "tile_map/touching_tiles.hpp"
#include "physics/aabb.hpp"
#include "player/player.hpp"
#include "combat/hit.hpp"
#include "tile_map/tile.hpp"
#include "tile_map/tile_map.hpp"

void touchTiles(Player &player, const TileMap &tileMap)
{
    AABB touching = player.body().touchBox();
    auto tilePositions = tileMap.tilesOverlapping(touching.position, touching.size);

    for (const auto &tilePosition : tilePositions)
    {
        const Tile &tile = tileMap.getTileAtTilePosition(tilePosition);
        glm::vec2 tileWorldPosition = tileMap.topLeftOfTile(tilePosition);
        std::optional<AABB> tileAABB = tile.getAABBAt(tileWorldPosition);
        if (!tileAABB || !touching.intersects(*tileAABB))
            continue;

        if (tile.isDeadly())
        {
            player.takeHit(lethalHit());
            break;
        }

        if (tile.isPortal())
        {
            player.completeLevel();
            break;
        }
    }
}