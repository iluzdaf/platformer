#include <optional>
#include "tile_map/touching_tiles.hpp"
#include "physics/aabb.hpp"
#include "player/player.hpp"
#include "tile_map/tile.hpp"
#include "tile_map/tile_map.hpp"

void touchTiles(Player &player, const TileMap &tileMap)
{
    AABB playerAABB = player.getPhysicsBody().getAABB();
    auto tilePositions = tileMap.tilesOverlapping(playerAABB.position, playerAABB.size);

    for (const auto &tilePosition : tilePositions)
    {
        const Tile &tile = tileMap.getTileAtTilePosition(tilePosition);
        glm::vec2 tileWorldPosition = tileMap.topLeftOfTile(tilePosition);
        std::optional<AABB> tileAABB = tile.getAABBAt(tileWorldPosition);
        if (!tileAABB || !playerAABB.intersects(*tileAABB))
            continue;

        if (tile.isDeadly())
        {
            player.onDeath();
            break;
        }

        if (tile.isPortal())
        {
            player.onLevelComplete();
            break;
        }
    }
}