

#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "game/tile_map/tile_interaction_system.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "test_helpers/test_player_utils.hpp"

TEST_CASE("Pickup", "[TileInteractionSystem]")
{
    TileData pickupTileData, pickupTileData2;
    pickupTileData2.kind = pickupTileData.kind = TileKind::Pickup;
    pickupTileData2.pickupReplaceIndex = pickupTileData.pickupReplaceIndex = 1;
    pickupTileData.pickupScoreDelta = 100;
    TileMap tileMap = setupTileMap(
        10, 10, 16,
        {{2, pickupTileData},
         {1, TileData{TileKind::Empty}},
         {3, pickupTileData2}});
    tileMap.setTileIndex({1, 1}, 2);
    Player player = setupPlayer();
    player.setPosition(glm::vec2(1 * 16, 1 * 16));
    TileInteractionSystem system;

    SECTION("Replaces pickup tile with replacement index")
    {
        system.fixedUpdate(player, tileMap);
        REQUIRE(tileMap.tilePositionToTileIndex({1, 1}) == 1);
    }

    SECTION("Triggers onPickup")
    {
        int scoreDelta = 0;
        player.onPickup.connect([&](int delta)
                                { scoreDelta = delta; });
        system.fixedUpdate(player, tileMap);
        REQUIRE(scoreDelta == 100);
        tileMap.setTileIndex({1, 1}, 3);
        scoreDelta = -1;
        system.fixedUpdate(player, tileMap);
        REQUIRE(scoreDelta == 0);
    }
}

TEST_CASE("Spikes", "[TileInteractionSystem]")
{
    TileMap tileMap = setupTileMap(10, 10, 16, {{3, TileData{TileKind::Spikes}}});
    tileMap.setTileIndex({1, 1}, 3);
    Player player = setupPlayer();
    player.setPosition(glm::vec2(1 * 16, 1 * 16));
    TileInteractionSystem system;

    SECTION("Triggers onDeath")
    {
        bool died = false;
        player.onDeath.connect([&]()
                               { died = true; });
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
    TileMap tileMap = setupTileMap(10, 10, 16, {{0, TileData{TileKind::Empty}}});
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
    TileMap tileMap = setupTileMap(10, 10, 16, {{4, TileData{TileKind::Portal}}});
    tileMap.setTileIndex({1, 1}, 4);
    Player player = setupPlayer();
    player.setPosition(glm::vec2(1 * 16, 1 * 16));
    TileInteractionSystem system;

    SECTION("Triggers onLevelComplete")
    {
        bool completed = false;
        player.onLevelComplete.connect([&]
                                       { completed = true; });
        system.fixedUpdate(player, tileMap);
        REQUIRE(completed);
    }

    SECTION("Does not replace")
    {
        system.fixedUpdate(player, tileMap);
        REQUIRE(tileMap.tilePositionToTileIndex(glm::ivec2(1, 1)) == 4);
    }
}