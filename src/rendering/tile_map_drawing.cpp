#include "rendering/tile_map_drawing.hpp"
#include "rendering/sprite_renderer.hpp"
#include "rendering/texture2d.hpp"
#include "tile_map/tile.hpp"
#include "tile_map/tile_map.hpp"

void drawTileMap(
    const SpriteRenderer &spriteRenderer,
    const TileMap &tileMap,
    const glm::mat4 &projection,
    const Shader &tileSetShader,
    const Texture2D &tileSet)
{
    int tileSize = tileMap.getTileSize();
    int cellSize = tileMap.getTileSet().cellSize.x;
    for (int tileY = 0; tileY < tileMap.getHeight(); ++tileY)
    {
        for (int tileX = 0; tileX < tileMap.getWidth(); ++tileX)
        {
            int tileIndex = tileMap.tilePositionToTileIndex(glm::ivec2(tileX, tileY));
            const Tile &tile = tileMap.getTile(tileIndex);
            int frameIndex = tile.animatingTo().value_or(tileIndex);
            auto [uvStart, uvEnd] = tileSet.getUVRange(frameIndex, cellSize);
            glm::vec2 position = tileMap.topLeftOfTile(glm::ivec2(tileX, tileY));
            glm::vec2 size = glm::vec2(static_cast<float>(tileSize));

            spriteRenderer.drawWithUV(
                tileSetShader, tileSet, projection, position, size, uvStart, uvEnd);
        }
    }
}