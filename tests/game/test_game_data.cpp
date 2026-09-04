#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "game/game_data.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_palette_data.hpp"

TEST_CASE("Game data without a default palette is refused", "[GameData]")
{
    TilePalettes palettes;
    palettes["ice"] = paletteOf({{0, TileData{}}});

    REQUIRE_THROWS_WITH(
        requireDefaultPalette(palettes), Catch::Matchers::ContainsSubstring("default"));
}

TEST_CASE("Every level counts on the palette named default", "[GameData]")
{
    REQUIRE_NOTHROW(requireDefaultPalette(shippedPalettes()));
}
