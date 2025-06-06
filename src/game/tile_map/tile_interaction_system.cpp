#include "game/tile_map/tile_interaction_system.hpp"
#include "game/player/player.hpp"
#include "game/tile_map/tile_map.hpp"

void TileInteractionSystem::fixedUpdate(Player &player, TileMap &tileMap)
{
    auto aabb = player.getAABB();
    auto tilePositions = tileMap.getTilePositionsAt(aabb.position, aabb.size);

    for (const auto &tilePos : tilePositions)
    {
        const Tile &tile = tileMap.getTile(tilePos);

        if (tile.isSpikes())
        {
            player.onDeath();
        }

        if (tile.isPickup())
        {
            auto replaceIndexOpt = tile.getPickupReplaceIndex();
            assert(replaceIndexOpt.has_value());
            tileMap.setTileIndex(tilePos, replaceIndexOpt.value());
            player.onLevelComplete();
        }
    }
}