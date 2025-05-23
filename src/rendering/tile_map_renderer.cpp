#include "rendering/tile_map_renderer.hpp"
#include <cassert>

TileMapRenderer::TileMapRenderer(
    const Texture2D &tileSet,
    const int tileSize,
    const SpriteRenderer &spriteRenderer) : tileSet(tileSet),
                                            tileSize(tileSize),
                                            spriteRenderer(spriteRenderer)
{
    assert(tileSet.getWidth() == tileSet.getHeight() && "TileSet texture must be square");
    assert(tileSize > 0 && "TileSize must be positive");
    assert(tileSet.getWidth() % tileSize == 0 && "TileSet width must be divisible by TileSize");
    assert(tileSet.getHeight() % tileSize == 0 && "TileSet height must be divisible by TileSize");

    tilesPerRow = tileSet.getWidth() / tileSize;
}

void TileMapRenderer::draw(const TileMap &map, const glm::mat4 &projection)
{
    for (int y = 0; y < map.getHeight(); ++y)
    {
        for (int x = 0; x < map.getWidth(); ++x)
        {
            int tileIndex = map.getTile(x, y);
            if (tileIndex < 0) continue;

            int tileX = tileIndex % tilesPerRow;
            int tileY = tileIndex / tilesPerRow;

            float tileUVSize = static_cast<float>(tileSize) / static_cast<float>(tileSet.getWidth());

            glm::vec2 uvStart(tileX * tileUVSize, tileY * tileUVSize);
            glm::vec2 uvEnd = uvStart + glm::vec2(tileUVSize);

            glm::vec2 position = glm::vec2(x * tileSize, y * tileSize);
            glm::vec2 size = glm::vec2(tileSize);

            spriteRenderer.drawWithUV(tileSet, projection, position, size, uvStart, uvEnd);
        }
    }
}