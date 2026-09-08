#include <cmath>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include "rendering/texture2d.hpp"
#include "rendering/tile_set_fit.hpp"
#include "helpers/palettes.hpp"
#include "tile_map/tile_data.hpp"
#include "animations/frame_animation_data.hpp"
#include "tile_map/tile_palette_data.hpp"

TEST_CASE("A tile set covering every tile a palette declares fits", "[TileSet]")
{
    TilePaletteData palette = paletteOf({{0, TileData{}}, {48, TileData{}}});

    REQUIRE_NOTHROW(checkTileSetFits(palette, "default", 112, 112));
}

TEST_CASE("A tile past the end of its tile set is refused", "[TileSet]")
{
    TilePaletteData palette = paletteOf({{0, TileData{}}, {49, TileData{}}});

    REQUIRE_THROWS_WITH(
        checkTileSetFits(palette, "default", 112, 112),
        Catch::Matchers::ContainsSubstring("Tile 49") &&
            Catch::Matchers::ContainsSubstring("49 tiles") &&
            Catch::Matchers::ContainsSubstring("default"));
}

TEST_CASE("A tile animating inside its tile set fits", "[TileSet]")
{
    TileData animated;
    animated.animationData = FrameAnimationData{{0, 48}, 0.1f};
    TilePaletteData palette = paletteOf({{0, animated}});

    REQUIRE_NOTHROW(checkTileSetFits(palette, "default", 112, 112));
}

TEST_CASE("A tile animating past the end of its tile set is refused", "[TileSet]")
{
    TileData animated;
    animated.animationData = FrameAnimationData{{0, 49}, 0.1f};
    TilePaletteData palette = paletteOf({{3, animated}});

    REQUIRE_THROWS_WITH(
        checkTileSetFits(palette, "default", 112, 112),
        Catch::Matchers::ContainsSubstring("Tile 3") &&
            Catch::Matchers::ContainsSubstring("frame 49") &&
            Catch::Matchers::ContainsSubstring("holds 49") &&
            Catch::Matchers::ContainsSubstring("default"));
}

TEST_CASE("A taller sheet keeps every tile where it was and holds more", "[TileSet]")
{
    TilePaletteData palette = paletteOf({{0, TileData{}}, {55, TileData{}}});

    REQUIRE_THROWS(checkTileSetFits(palette, "default", 112, 112));
    REQUIRE_NOTHROW(checkTileSetFits(palette, "default", 112, 128));
}

TEST_CASE("A sheet holding no whole tile is refused", "[TileSet]")
{
    TilePaletteData palette = paletteOf({{0, TileData{}}});

    REQUIRE_THROWS_WITH(
        checkTileSetFits(palette, "default", 8, 8),
        Catch::Matchers::ContainsSubstring("no whole tiles"));
}

TEST_CASE("A tile set cell narrower than a pixel is refused", "[TileSet]")
{
    TilePaletteData palette = paletteOf({{0, TileData{}}});
    palette.tileSet.cellSize = glm::ivec2(0);

    REQUIRE_THROWS_WITH(
        checkTileSetFits(palette, "default", 112, 112),
        Catch::Matchers::ContainsSubstring("cell must be wider than 0"));
}

TEST_CASE("A wider sheet holds more tiles", "[TileSet]")
{
    TilePaletteData palette = paletteOf({{0, TileData{}}, {63, TileData{}}});

    REQUIRE_THROWS(checkTileSetFits(palette, "default", 112, 112));
    REQUIRE_NOTHROW(checkTileSetFits(palette, "default", 128, 128));
}

TEST_CASE("A tile stays where it is when the sheet grows a row taller", "[TileSet]")
{
    for (int index : {0, 6, 7, 13, 48})
    {
        auto [wasStart, wasEnd] = uvRangeIn(112, 112, index, 16);
        auto [nowStart, nowEnd] = uvRangeIn(112, 128, index, 16);

        REQUIRE(wasStart.x == nowStart.x);
        REQUIRE(wasEnd.x == nowEnd.x);
        REQUIRE(std::lround(128.0f - nowEnd.y * 128.0f) == std::lround(112.0f - wasEnd.y * 112.0f));
    }
}

TEST_CASE("A square sheet counts its rows down from the top", "[TileSet]")
{
    for (int index : {0, 1, 7, 42, 48})
    {
        auto [start, end] = uvRangeIn(112, 112, index, 16);
        float cell = 16.0f / 112.0f;
        int rowFromTheTop = index / 7;

        REQUIRE(start.x == (index % 7) * cell);
        REQUIRE(end.x == ((index % 7) + 1) * cell);
        REQUIRE(start.y == (7 - 1 - rowFromTheTop) * cell);
        REQUIRE(end.y == (7 - rowFromTheTop) * cell);
    }
}

TEST_CASE("A tall sheet divides each axis by its own size", "[TileSet]")
{
    auto [start, end] = uvRangeIn(112, 224, 7, 16);

    REQUIRE(start.x == 0.0f);
    REQUIRE(end.x == 16.0f / 112.0f);
    REQUIRE(start.y == Catch::Approx(192.0f / 224.0f));
    REQUIRE(end.y == Catch::Approx(208.0f / 224.0f));
}
