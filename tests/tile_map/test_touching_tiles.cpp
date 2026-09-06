

#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "tile_map/tile_data.hpp"
#include "tile_map/touching_tiles.hpp"
#include "helpers/palettes.hpp"
#include "helpers/tiles.hpp"
#include "helpers/actors.hpp"

TEST_CASE("Spikes", "[TouchingTiles]")
{
    TileData spikeTileData;
    spikeTileData.deadly = true;
    TileMap tileMap = aTileMapWith({{{1, 1}, 3}}, 10, 10, 16, paletteOf({{3, spikeTileData}}));
    Player player = aPlayerWithEveryAbility();
    player.setPosition(glm::vec2(1 * 16, 1 * 16));

    SECTION("Triggers onDeath")
    {
        bool died = false;
        player.onDeath.connect([&]() { died = true; });
        touchTiles(player, tileMap);
        REQUIRE(died);
    }

    SECTION("Does not replace")
    {
        touchTiles(player, tileMap);
        REQUIRE(tileMap.tilePositionToTileIndex(glm::ivec2(1, 1)) == 3);
    }
}

TEST_CASE("Empty", "[TouchingTiles]")
{
    TileData emptyTileData;
    TileMap tileMap = aTileMapWith({{{1, 1}, 0}}, 10, 10, 16, paletteOf({{0, emptyTileData}}));
    Player player = aPlayerWithEveryAbility();
    player.setPosition(glm::vec2(1 * 16, 1 * 16));

    SECTION("Does not replace")
    {
        touchTiles(player, tileMap);
        REQUIRE(tileMap.tilePositionToTileIndex(glm::ivec2(1, 1)) == 0);
    }
}

TEST_CASE("Portal", "[TouchingTiles]")
{
    TileData portalTileData;
    portalTileData.portal = true;
    TileMap tileMap = aTileMapWith({{{1, 1}, 4}}, 10, 10, 16, paletteOf({{4, portalTileData}}));
    Player player = aPlayerWithEveryAbility();
    player.setPosition(glm::vec2(1 * 16, 1 * 16));

    SECTION("Triggers onLevelComplete")
    {
        bool completed = false;
        player.onLevelComplete.connect([&] { completed = true; });
        touchTiles(player, tileMap);
        REQUIRE(completed);
    }

    SECTION("Does not replace")
    {
        touchTiles(player, tileMap);
        REQUIRE(tileMap.tilePositionToTileIndex(glm::ivec2(1, 1)) == 4);
    }
}