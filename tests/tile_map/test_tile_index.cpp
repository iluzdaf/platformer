#include <string>
#include <type_traits>
#include <catch2/catch_test_macros.hpp>
#include <glaze/glaze.hpp>
#include "tile_map/tile_index.hpp"
#include "tile_map/tile_pickup_data.hpp"

TEST_CASE("A tile index is a tile index and not any other number", "[TileIndex]")
{
    STATIC_REQUIRE(std::is_convertible_v<int, TileIndex>);
    STATIC_REQUIRE_FALSE(std::is_convertible_v<TileIndex, int>);
}

TEST_CASE("A tile index is written as the number it is", "[TileIndex]")
{
    TilePickupData pickup;
    pickup.replaceIndex = 7;

    std::string json;
    REQUIRE_FALSE(glz::write_json(pickup, json));

    REQUIRE(json == R"({"replaceIndex":7})");
}

TEST_CASE("A tile index is read from the number it was", "[TileIndex]")
{
    TilePickupData pickup;

    REQUIRE_FALSE(glz::read_json(pickup, R"({"replaceIndex":42,"scoreDelta":5})"));

    REQUIRE(pickup.replaceIndex == TileIndex{42});
    REQUIRE(pickup.replaceIndex.value == 42);
    REQUIRE(pickup.scoreDelta == 5);
}

TEST_CASE("A pickup still reads the way it is written on disk", "[TileIndex]")
{
    TilePickupData pickup;
    REQUIRE_FALSE(glz::read_json(pickup, R"({"replaceIndex":0})"));

    std::string json;
    REQUIRE_FALSE(glz::write_json(pickup, json));

    REQUIRE(json == R"({"replaceIndex":0})");
}
