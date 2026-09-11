#include <cstddef>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "game/empty_level.hpp"
#include "game/level_data.hpp"
#include "tile_map/tile_map.hpp"

LevelData anEmptyLevelLike(const LevelData &playing, int tileSize)
{
    LevelData made;
    made.tileMapData.tilePalette = playing.tileMapData.tilePalette;
    made.nextLevel = playing.nextLevel;

    std::size_t rows = playing.tileMapData.indices.size();
    std::size_t columns = rows == 0 ? 0 : playing.tileMapData.indices.front().size();
    made.tileMapData.indices.assign(rows, std::vector<int>(columns, 0));
    if (rows == 0 || columns == 0)
        return made;

    made.playerFeet = feetOnTile(glm::ivec2(0, static_cast<int>(rows) - 1), tileSize);

    return made;
}
