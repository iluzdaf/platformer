#include "rendering/tile_map_renderer.hpp"
#include <cassert>

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

    for (int y = 0; y < tileMap.getHeight(); ++y)
    {
        for (int x = 0; x < tileMap.getWidth(); ++x)
        {
            int tileIndex = tileMap.getTileIndex(x, y);
            if (tileIndex < 0)
                continue;

            auto tileOpt = tileMap.getTile(tileIndex);
            if (!tileOpt)
                continue;

            const Tile &tile = tileOpt->get();
            int frameIndex = tile.isAnimated()
                                 ? tile.getCurrentFrame()
                                 : tileIndex;

            int tileX = frameIndex % tilesPerRow;
            int tileY = frameIndex / tilesPerRow;
            float tileUVSize = static_cast<float>(tileSize) / static_cast<float>(tileSetWidth);
            glm::vec2 uvStart(tileX * tileUVSize, tileY * tileUVSize);
            glm::vec2 uvEnd = uvStart + glm::vec2(tileUVSize);
            glm::vec2 position = glm::vec2(x * tileSize, y * tileSize);
            glm::vec2 size = glm::vec2(tileSize);

            spriteRenderer.drawWithUV(tileSet, projection, position, size, uvStart, uvEnd);
        }
    }
}