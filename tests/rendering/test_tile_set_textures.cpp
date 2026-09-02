#include <cmath>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include "rendering/texture2d.hpp"
#include "rendering/tile_set_textures.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_palette.hpp"

TEST_CASE("A tile set covering every tile a palette declares fits", "[TileSet]")
{
    TilePalette palette = paletteOf({{0, TileData{}}, {48, TileData{}}});

    REQUIRE_NOTHROW(checkTileSetFits(palette, "default", 112, 112));
}

TEST_CASE("A tile past the end of its tile set is refused", "[TileSet]")
{
    TilePalette palette = paletteOf({{0, TileData{}}, {49, TileData{}}});

    REQUIRE_THROWS_WITH(
        checkTileSetFits(palette, "default", 112, 112),
        Catch::Matchers::ContainsSubstring("Tile 49") &&
            Catch::Matchers::ContainsSubstring("49 tiles") &&
            Catch::Matchers::ContainsSubstring("default"));
}

TEST_CASE("A taller sheet keeps every tile where it was and holds more", "[TileSet]")
{
    TilePalette palette = paletteOf({{0, TileData{}}, {55, TileData{}}});

    REQUIRE_THROWS(checkTileSetFits(palette, "default", 112, 112));
    REQUIRE_NOTHROW(checkTileSetFits(palette, "default", 112, 128));
}

TEST_CASE("A sheet holding no whole tile is refused", "[TileSet]")
{
    TilePalette palette = paletteOf({{0, TileData{}}});

    REQUIRE_THROWS_WITH(
        checkTileSetFits(palette, "default", 8, 8),
        Catch::Matchers::ContainsSubstring("no whole tiles"));
}

TEST_CASE("A palette naming no tile set texture is refused", "[TileSet]")
{
    TilePalette palette = paletteOf({{0, TileData{}}});
    palette.tileSet.texture.clear();

    REQUIRE_THROWS_WITH(
        checkTileSetFits(palette, "ice", 112, 112),
        Catch::Matchers::ContainsSubstring("No tile set texture") &&
            Catch::Matchers::ContainsSubstring("ice"));
}

TEST_CASE("A tile set cell narrower than a pixel is refused", "[TileSet]")
{
    TilePalette palette = paletteOf({{0, TileData{}}});
    palette.tileSet.tileSize = 0;

    REQUIRE_THROWS_WITH(
        checkTileSetFits(palette, "default", 112, 112),
        Catch::Matchers::ContainsSubstring("cell must be wider than 0"));
}

TEST_CASE("A wider sheet holds more tiles", "[TileSet]")
{
    TilePalette palette = paletteOf({{0, TileData{}}, {63, TileData{}}});

    REQUIRE_THROWS(checkTileSetFits(palette, "default", 112, 112));
    REQUIRE_NOTHROW(checkTileSetFits(palette, "default", 128, 128));
}

TEST_CASE("Every shipped palette fits the tile set it names", "[TileSet]")
{
    for (const auto &[name, palette] : shippedPalettes())
        REQUIRE_NOTHROW(checkTileSetFits(palette, name, 112, 112));
}

TEST_CASE("A tile keeps its cell when the sheet grows taller", "[TileSet]")
{
    for (int index : {0, 6, 7, 13, 48})
    {
        auto [wasStart, wasEnd] = uvRangeIn(112, 112, index, 16);
        auto [nowStart, nowEnd] = uvRangeIn(112, 128, index, 16);

        REQUIRE(wasStart.x == nowStart.x);
        REQUIRE(wasEnd.x == nowEnd.x);
        REQUIRE(std::lround(nowStart.y * 128.0f) == std::lround(wasStart.y * 112.0f));
    }
}

TEST_CASE("A square sheet reads exactly as it did", "[TileSet]")
{
    for (int index : {0, 1, 7, 42, 48})
    {
        auto [start, end] = uvRangeIn(112, 112, index, 16);
        float cell = 16.0f / 112.0f;

        REQUIRE(start.x == (index % 7) * cell);
        REQUIRE(start.y == (index / 7) * cell);
        REQUIRE(end.x == ((index % 7) + 1) * cell);
        REQUIRE(end.y == ((index / 7) + 1) * cell);
    }
}

TEST_CASE("A tall sheet divides each axis by its own size", "[TileSet]")
{
    auto [start, end] = uvRangeIn(112, 224, 7, 16);

    REQUIRE(start.x == 0.0f);
    REQUIRE(end.x == 16.0f / 112.0f);
    REQUIRE(start.y == 16.0f / 224.0f);
    REQUIRE(end.y == 32.0f / 224.0f);
}

#ifndef SKIP_OPENGL_TESTS

#include "assets/asset_paths.hpp"
#include "rendering/texture2d.hpp"
#include "rendering/texture_cache.hpp"

TEST_CASE("Two palettes naming two tile sets get two textures", "[TileSet]")
{
    TilePalettes palettes;
    palettes["default"] = paletteOf({{0, TileData{}}});
    palettes["other"] = paletteOf({{0, TileData{}}});
    palettes["other"].tileSet.texture = std::string(assets::PlayerTexture);

    TextureCache textures;
    warmTileSets(textures, palettes);

    const Texture2D &first = textures.get(palettes["default"].tileSet.texture);
    const Texture2D &second = textures.get(palettes["other"].tileSet.texture);

    REQUIRE(&first != &second);
    REQUIRE(first.getWidth() == 112);
    REQUIRE(second.getWidth() == 96);
}

TEST_CASE("Two palettes sharing a tile set load it once", "[TileSet]")
{
    TilePalettes palettes;
    palettes["default"] = paletteOf({{0, TileData{}}});
    palettes["same"] = paletteOf({{0, TileData{}}});

    TextureCache textures;
    warmTileSets(textures, palettes);

    REQUIRE(
        &textures.get(palettes["default"].tileSet.texture) ==
        &textures.get(palettes["same"].tileSet.texture));
}

TEST_CASE("A palette whose tile set is not on disk says so", "[TileSet]")
{
    TilePalettes palettes;
    palettes["ice"] = paletteOf({{0, TileData{}}});
    palettes["ice"].tileSet.texture = "textures/nothing_here.png";

    TextureCache textures;

    REQUIRE_THROWS_WITH(
        warmTileSets(textures, palettes),
        Catch::Matchers::ContainsSubstring("textures/nothing_here.png"));
}

#endif // SKIP_OPENGL_TESTS
