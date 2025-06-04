#include <cassert>
#include "rendering/tile_map_renderer.hpp"
#include "game/tile_map/tile_map.hpp"

TileMapRenderer::TileMapRenderer(
    const Texture2D &tileSet,
    const SpriteRenderer &spriteRenderer) : tileSet(tileSet),
                                            spriteRenderer(spriteRenderer)
{
    assert(tileSet.getWidth() == tileSet.getHeight() && "TileSet texture must be square");
}

void TileMapRenderer::draw(const TileMap &tileMap, const glm::mat4 &projection)
{
    int tileSize = tileMap.getTileSize();
    int tileSetWidth = tileSet.getWidth();
    int tilesPerRow = tileSetWidth / tileSize;

    for (int tileY = 0; tileY < tileMap.getHeight(); ++tileY)
    {
        for (int tileX = 0; tileX < tileMap.getWidth(); ++tileX)
        {
            int tileIndex = tileMap.getTileIndex(glm::ivec2(tileX, tileY));
            const Tile &tile = tileMap.getTile(tileIndex);
            int frameIndex = tile.isAnimated()
                                 ? tile.getCurrentFrame()
                                 : tileIndex;

            int tileSetX = frameIndex % tilesPerRow;
            int tileSetY = frameIndex / tilesPerRow;
            float tileUVSize = static_cast<float>(tileSize) / static_cast<float>(tileSetWidth);
            glm::vec2 uvStart(tileSetX * tileUVSize, tileSetY * tileUVSize);
            glm::vec2 uvEnd = uvStart + glm::vec2(tileUVSize);
            glm::vec2 position = glm::vec2(tileX * tileSize, tileY * tileSize);
            glm::vec2 size = glm::vec2(tileSize);

            spriteRenderer.drawWithUV(tileSet, projection, position, size, uvStart, uvEnd);
        }
    }
}