

#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_interaction_system.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "test_helpers/test_player_utils.hpp"

TEST_CASE("Spikes", "[TileInteractionSystem]")
{
    TileData spikeTileData;
    spikeTileData.deadly = true;
    TileMap tileMap = setupTileMap(10, 10, 16, paletteOf({{3, spikeTileData}}));
    tileMap.setTileIndex({1, 1}, 3);
    Player player = setupPlayer();
    player.setPosition(glm::vec2(1 * 16, 1 * 16));
    TileInteractionSystem system;

    SECTION("Triggers onDeath")
    {
        bool died = false;
        player.onDeath.connect([&]() { died = true; });
        system.fixedUpdate(player, tileMap);
        REQUIRE(died);
    }

    SECTION("Does not replace")
    {
        system.fixedUpdate(player, tileMap);
        REQUIRE(tileMap.tilePositionToTileIndex(glm::ivec2(1, 1)) == 3);
    }
}

TEST_CASE("Empty", "[TileInteractionSystem]")
{
    TileData emptyTileData;
    TileMap tileMap = setupTileMap(10, 10, 16, paletteOf({{0, emptyTileData}}));
    tileMap.setTileIndex({1, 1}, 0);
    Player player = setupPlayer();
    player.setPosition(glm::vec2(1 * 16, 1 * 16));
    TileInteractionSystem system;

    SECTION("Does not replace")
    {
        system.fixedUpdate(player, tileMap);
        REQUIRE(tileMap.tilePositionToTileIndex(glm::ivec2(1, 1)) == 0);
    }
}

TEST_CASE("Portal", "[TileInteractionSystem]")
{
    TileData portalTileData;
    portalTileData.portal = true;
    TileMap tileMap = setupTileMap(10, 10, 16, paletteOf({{4, portalTileData}}));
    tileMap.setTileIndex({1, 1}, 4);
    Player player = setupPlayer();
    player.setPosition(glm::vec2(1 * 16, 1 * 16));
    TileInteractionSystem system;

    SECTION("Triggers onLevelComplete")
    {
        bool completed = false;
        player.onLevelComplete.connect([&] { completed = true; });
        system.fixedUpdate(player, tileMap);
        REQUIRE(completed);
    }

    SECTION("Does not replace")
    {
        system.fixedUpdate(player, tileMap);
        REQUIRE(tileMap.tilePositionToTileIndex(glm::ivec2(1, 1)) == 4);
    }
}