#include <format>
#include <string>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <glm/glm.hpp>
#include "game/levels.hpp"
#include "test_helpers/asset_path.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "tile_map/tile.hpp"
#include "tile_map/tile_map.hpp"
#include "tile_map/tile_map_data.hpp"

namespace
{
    constexpr int LeftCap = 39;
    constexpr int Middle = 40;
    constexpr int RightCap = 41;

    bool isPlatform(int tileIndex)
    {
        return tileIndex == LeftCap || tileIndex == Middle || tileIndex == RightCap;
    }

    bool solidAt(const TileMap &tileMap, int x, int y)
    {
        glm::ivec2 tilePosition(x, y);

        return tileMap.validTilePosition(tilePosition) &&
               tileMap.getTileAtTilePosition(tilePosition).isSolid();
    }

    std::string capsWrongIn(const TileMap &tileMap, const std::string &name)
    {
        std::string wrong;

        auto expect = [&](int x, int y, int wanted)
        {
            int found = tileMap.tilePositionToTileIndex(glm::ivec2(x, y));
            if (found != wanted)
                wrong +=
                    std::format("{} at {},{}: expected {}, found {}\n", name, x, y, wanted, found);
        };

        for (int y = 0; y < tileMap.getHeight(); ++y)
            for (int x = 0; x < tileMap.getWidth(); ++x)
            {
                if (!isPlatform(tileMap.tilePositionToTileIndex(glm::ivec2(x, y))))
                    continue;

                int start = x;
                while (x < tileMap.getWidth() &&
                       isPlatform(tileMap.tilePositionToTileIndex(glm::ivec2(x, y))))
                    ++x;
                int last = x - 1;

                expect(start, y, solidAt(tileMap, start - 1, y) ? Middle : LeftCap);
                expect(last, y, solidAt(tileMap, last + 1, y) ? Middle : RightCap);
                for (int between = start + 1; between < last; ++between)
                    expect(between, y, Middle);
            }

        return wrong;
    }
}

TEST_CASE("Every shipped platform is capped where it meets open air", "[Levels]")
{
    std::vector<std::string> levelPaths = levelPathsIn(assetPath("levels"));
    REQUIRE(levelPaths.size() >= 6);

    std::string wrong;
    for (const std::string &levelPath : levelPaths)
        wrong += capsWrongIn(tilesOfLevel(assetPath(levelPath)), levelName(levelPath));

    INFO(wrong);
    REQUIRE(wrong.empty());
}

namespace
{
    TileMap mapOfRow(const std::vector<int> &row)
    {
        TileMapData tileMapData;
        tileMapData.size = 16;
        tileMapData.indices = std::vector<std::vector<int>>{row};

        return TileMap(tileMapData, shippedPalettes());
    }
}

TEST_CASE("A platform in open air is capped at both ends", "[Levels]")
{
    REQUIRE(capsWrongIn(mapOfRow({0, LeftCap, Middle, RightCap, 0}), "row").empty());
    REQUIRE_FALSE(capsWrongIn(mapOfRow({0, Middle, Middle, RightCap, 0}), "row").empty());
    REQUIRE_FALSE(capsWrongIn(mapOfRow({0, LeftCap, Middle, Middle, 0}), "row").empty());
}

TEST_CASE("A platform running into a wall is not capped against it", "[Levels]")
{
    constexpr int Wall = 14;

    REQUIRE(capsWrongIn(mapOfRow({Wall, Middle, Middle, RightCap, 0}), "row").empty());
    REQUIRE(capsWrongIn(mapOfRow({0, LeftCap, Middle, Middle, Wall}), "row").empty());
    REQUIRE_FALSE(capsWrongIn(mapOfRow({Wall, LeftCap, Middle, RightCap, 0}), "row").empty());
}

TEST_CASE("A platform is made of middles between its caps", "[Levels]")
{
    REQUIRE_FALSE(capsWrongIn(mapOfRow({0, LeftCap, LeftCap, RightCap, 0}), "row").empty());
    REQUIRE_FALSE(capsWrongIn(mapOfRow({0, LeftCap, RightCap, RightCap, 0}), "row").empty());
}
