#include <array>
#include <cstddef>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "rendering/tile_map_vertices.hpp"
#include "rendering/texture2d.hpp"
#include "tile_map/tile.hpp"
#include "tile_map/tile_map.hpp"
#include "tile_map/tile_set.hpp"

namespace
{
    struct Corner
    {
        glm::vec2 position, texture;
    };

    constexpr std::array<Corner, VerticesPerTile> Quad{
        Corner{{0.0f, 1.0f}, {0.0f, 0.0f}},
        Corner{{1.0f, 0.0f}, {1.0f, 1.0f}},
        Corner{{0.0f, 0.0f}, {0.0f, 1.0f}},
        Corner{{0.0f, 1.0f}, {0.0f, 0.0f}},
        Corner{{1.0f, 1.0f}, {1.0f, 0.0f}},
        Corner{{1.0f, 0.0f}, {1.0f, 1.0f}}};
}

void appendTileMapVertices(
    std::vector<float> &into,
    const TileMap &tileMap,
    int sheetWidth,
    int sheetHeight)
{
    float tileSize = static_cast<float>(tileMap.getTileSize());
    int cellSize = tileMap.getTileSet().tileSize;

    into.reserve(
        into.size() + static_cast<std::size_t>(tileMap.getWidth()) * tileMap.getHeight() *
                          VerticesPerTile * FloatsPerVertex);

    for (int tileY = 0; tileY < tileMap.getHeight(); ++tileY)
    {
        for (int tileX = 0; tileX < tileMap.getWidth(); ++tileX)
        {
            glm::ivec2 tilePosition(tileX, tileY);
            int tileIndex = tileMap.tilePositionToTileIndex(tilePosition);
            const Tile &tile = tileMap.getTile(tileIndex);
            int frameIndex = tile.animatingTo().value_or(tileIndex);

            auto [uvStart, uvEnd] = uvRangeIn(sheetWidth, sheetHeight, frameIndex, cellSize);
            glm::vec2 topLeft = tileMap.topLeftOfTile(tilePosition);

            for (const Corner &corner : Quad)
            {
                glm::vec2 position = topLeft + corner.position * tileSize;
                glm::vec2 texture = glm::mix(uvStart, uvEnd, corner.texture);

                into.push_back(position.x);
                into.push_back(position.y);
                into.push_back(texture.x);
                into.push_back(texture.y);
            }
        }
    }
}
