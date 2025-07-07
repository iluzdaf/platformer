#include <cassert>
#include "rendering/tile_map_renderer.hpp"
#include "rendering/texture2d.hpp"
#include "game/tile_map/tile_map.hpp"

TileMapRenderer::TileMapRenderer(const SpriteRenderer &spriteRenderer)
    : spriteRenderer(spriteRenderer)
{
}

void TileMapRenderer::draw(
    const TileMap &tileMap,
    const glm::mat4 &projection,
    const Shader &tileSetShader,
    const Texture2D &tileSet)
{
    assert(tileSet.getWidth() == tileSet.getHeight() && "TileSet texture must be square");

    int tileSize = tileMap.getTileSize();
    for (int tileY = 0; tileY < tileMap.getHeight(); ++tileY)
    {
        for (int tileX = 0; tileX < tileMap.getWidth(); ++tileX)
        {
            int tileIndex = tileMap.tilePositionToTileIndex(glm::ivec2(tileX, tileY));
            const Tile &tile = tileMap.getTile(tileIndex);
            int frameIndex = tile.getCurrentFrame();
            auto [uvStart, uvEnd] = tileSet.getUVRange(frameIndex, tileSize);
            glm::vec2 position = tileMap.tileToWorldPosition(glm::ivec2(tileX, tileY));
            glm::vec2 size = glm::vec2(static_cast<float>(tileSize));

            spriteRenderer.drawWithUV(tileSetShader, tileSet, projection, position, size, uvStart, uvEnd);
        }
    }
}