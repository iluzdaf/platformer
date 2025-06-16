#include "game/tile_map/tile_interaction_system.hpp"
#include "game/player/player.hpp"
#include "game/tile_map/tile_map.hpp"
#include <iostream>

void TileInteractionSystem::fixedUpdate(Player &player, TileMap &tileMap)
{
    AABB playerAABB = player.getAABB();
    auto tilePositions = tileMap.worldToTilePositions(playerAABB.position, playerAABB.size);

    for (const auto &tilePosition : tilePositions)
    {
        if(!tileMap.validTilePosition(tilePosition))
            continue;
            
        const Tile &tile = tileMap.getTileAtTilePosition(tilePosition);
        glm::vec2 tileWorldPosition = tileMap.tileToWorldPosition(tilePosition);
        AABB tileAABB = tile.getAABBAt(tileWorldPosition);

        if (!playerAABB.intersects(tileAABB))
        {
            continue;
        }

        if (tile.isSpikes())
        {
            player.onDeath();
        }

        if (tile.isPickup())
        {
            auto replaceIndexOpt = tile.getPickupReplaceIndex();
            assert(replaceIndexOpt.has_value());
            tileMap.setTileIndex(tilePosition, replaceIndexOpt.value());
            player.onLevelComplete();
        }
    }
}