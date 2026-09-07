#include <cmath>
#include <cstddef>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "game/level_resizing.hpp"
#include "game/level_data.hpp"
#include "npc/npc_spawn_data.hpp"
#include "pickups/pickup_spawn_data.hpp"
#include "tile_map/tile_map_data.hpp"

glm::vec2 shiftOf(Resize resize, int tileSize)
{
    float tile = static_cast<float>(resize.larger ? tileSize : -tileSize);
    switch (resize.side)
    {
    case Side::Left:
        return glm::vec2(tile, 0.0f);
    case Side::Above:
        return glm::vec2(0.0f, tile);
    case Side::Right:
    case Side::Below:
        break;
    }

    return glm::vec2(0.0f);
}

namespace
{
    void resizeRows(std::vector<std::vector<int>> &rows, Resize resize)
    {
        std::size_t width = rows.empty() ? 0 : rows.front().size();
        switch (resize.side)
        {
        case Side::Left:
            for (std::vector<int> &row : rows)
                if (resize.larger)
                    row.insert(row.begin(), 0);
                else if (!row.empty())
                    row.erase(row.begin());
            break;
        case Side::Right:
            for (std::vector<int> &row : rows)
                if (resize.larger)
                    row.push_back(0);
                else if (!row.empty())
                    row.pop_back();
            break;
        case Side::Above:
            if (resize.larger)
                rows.insert(rows.begin(), std::vector<int>(width, 0));
            else if (!rows.empty())
                rows.erase(rows.begin());
            break;
        case Side::Below:
            if (resize.larger)
                rows.emplace_back(width, 0);
            else if (!rows.empty())
                rows.pop_back();
            break;
        }
    }

    struct Tiles
    {
        int width, height, tileSize;

        bool holdsColumn(float x) const
        {
            int column = static_cast<int>(std::floor(x / static_cast<float>(tileSize)));
            return column >= 0 && column < width;
        }

        bool holdsFeet(glm::vec2 feet) const
        {
            int row = static_cast<int>(std::round(feet.y / static_cast<float>(tileSize))) - 1;
            return holdsColumn(feet.x) && row >= 0 && row < height;
        }

        bool holdsPoint(glm::vec2 point) const
        {
            int row = static_cast<int>(std::floor(point.y / static_cast<float>(tileSize)));
            return holdsColumn(point.x) && row >= 0 && row < height;
        }
    };
}

LevelData resizedBy(Resize resize, const LevelData &level, int tileSize)
{
    LevelData resized = level;
    std::vector<std::vector<int>> &rows = resized.tileMapData.indices;
    resizeRows(rows, resize);

    glm::vec2 shift = shiftOf(resize, tileSize);
    resized.playerFeet += shift;
    for (NpcSpawnData &npc : resized.npcs)
    {
        npc.feet += shift;
        if (npc.patrol)
        {
            npc.patrol->from += shift;
            npc.patrol->to += shift;
        }
    }
    for (PickupSpawnData &pickup : resized.pickups)
        pickup.feet += shift;

    if (resize.larger)
        return resized;

    Tiles left{
        static_cast<int>(rows.empty() ? 0 : rows.front().size()),
        static_cast<int>(rows.size()),
        tileSize};
    std::erase_if(resized.npcs, [&](const NpcSpawnData &npc) { return !left.holdsFeet(npc.feet); });
    for (NpcSpawnData &npc : resized.npcs)
        if (npc.patrol && !(left.holdsFeet(npc.patrol->from) && left.holdsFeet(npc.patrol->to)))
            npc.patrol.reset();
    std::erase_if(
        resized.pickups,
        [&](const PickupSpawnData &pickup) { return !left.holdsPoint(pickup.feet); });

    return resized;
}
