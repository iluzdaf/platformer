#include <catch2/catch_test_macros.hpp>
#include <optional>
#include "ui/tile_map_shown.hpp"

TEST_CASE("A tile map overlay nobody asked for is shown while something is armed", "[TileMapShown]")
{
    TileMapShown tileMap = whileArmed(TileMapShown{false, std::nullopt}, true);

    REQUIRE(tileMap.showing);
}

TEST_CASE("A tile map overlay shown only for arming goes away again", "[TileMapShown]")
{
    TileMapShown tileMap = whileArmed(TileMapShown{false, std::nullopt}, true);

    REQUIRE(whileArmed(tileMap, false) == TileMapShown{false, std::nullopt});
}

TEST_CASE("A tile map overlay asked for stays after arming ends", "[TileMapShown]")
{
    TileMapShown tileMap = whileArmed(TileMapShown{true, std::nullopt}, true);

    REQUIRE(tileMap.showing);
    REQUIRE(whileArmed(tileMap, false) == TileMapShown{true, std::nullopt});
}

TEST_CASE("Staying armed remembers only what was there first", "[TileMapShown]")
{
    TileMapShown tileMap = whileArmed(TileMapShown{false, std::nullopt}, true);
    tileMap.showing = false;
    tileMap = whileArmed(tileMap, true);
    tileMap = whileArmed(tileMap, true);

    REQUIRE(tileMap.beforeArming == false);
    REQUIRE(whileArmed(tileMap, false) == TileMapShown{false, std::nullopt});
}

TEST_CASE("Staying disarmed leaves the tile map overlay alone", "[TileMapShown]")
{
    REQUIRE(
        whileArmed(TileMapShown{false, std::nullopt}, false) == TileMapShown{false, std::nullopt});
    REQUIRE(
        whileArmed(TileMapShown{true, std::nullopt}, false) == TileMapShown{true, std::nullopt});
}

TEST_CASE("A tile map overlay put away by hand stays away after arming ends", "[TileMapShown]")
{
    TileMapShown tileMap = whileArmed(TileMapShown{true, std::nullopt}, true);
    tileMap.showing = false;

    REQUIRE(whileArmed(tileMap, false) == TileMapShown{false, std::nullopt});
}

TEST_CASE("A tile map overlay put away and brought back by hand is left showing", "[TileMapShown]")
{
    TileMapShown tileMap = whileArmed(TileMapShown{true, std::nullopt}, true);
    tileMap.showing = false;
    tileMap.showing = true;

    REQUIRE(whileArmed(tileMap, false) == TileMapShown{true, std::nullopt});
}
