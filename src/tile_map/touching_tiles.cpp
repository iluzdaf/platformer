#include <glm/gtc/matrix_transform.hpp>
#include "tile_map/touching_tiles.hpp"
#include "player/player.hpp"
#include "combat/hit.hpp"
#include "tile_map/tile.hpp"
#include "tile_map/tile_map.hpp"

void touchTiles(Player &player, const TileMap &tileMap)
{
    for (glm::ivec2 tilePosition : tileMap.tilesTouching(player.body().touchBox()))
    {
        const Tile &tile = tileMap.getTileAtTilePosition(tilePosition);
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
