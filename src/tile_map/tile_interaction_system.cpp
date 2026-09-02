#include <optional>
#include "tile_map/tile_interaction_system.hpp"
#include "physics/aabb.hpp"
#include "player/player.hpp"
#include "tile_map/tile.hpp"
#include "tile_map/tile_map.hpp"
#include <cassert>

void TileInteractionSystem::fixedUpdate(Player &player, TileMap &tileMap)
{
    AABB playerAABB = player.getPhysicsBody().getAABB();
    auto tilePositions = tileMap.tilesOverlapping(playerAABB.position, playerAABB.size);

    for (const auto &tilePosition : tilePositions)
    {
        if (!tileMap.validTilePosition(tilePosition))
            continue;

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