#include <string>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers.hpp>
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

TEST_CASE("A tile set that is not square is refused", "[TileSet]")
{
    TilePalette palette = paletteOf({{0, TileData{}}});

    REQUIRE_THROWS_WITH(
        checkTileSetFits(palette, "default", 128, 112),
        Catch::Matchers::ContainsSubstring("square") &&
            Catch::Matchers::ContainsSubstring("128 by 112"));
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

TEST_CASE("A tile set of the same grid is a reskin", "[TileSet]")
{
    TileSet saved{"textures/tile_set.png", 16};
    TileSet edited{"textures/night.png", 16};

    REQUIRE_FALSE(whatMovingToWouldBreak(saved, 112, edited, 112, "default").has_value());
}

TEST_CASE("A wider sheet moves every tile", "[TileSet]")
{
    TileSet saved{"textures/tile_set.png", 16};
    TileSet edited{"textures/bigger.png", 16};

    auto broken = whatMovingToWouldBreak(saved, 112, edited, 128, "default");

    REQUIRE(broken.has_value());
    REQUIRE_THAT(*broken, Catch::Matchers::ContainsSubstring("7 across becomes 8"));
    REQUIRE_THAT(*broken, Catch::Matchers::ContainsSubstring("default"));
}

TEST_CASE("A smaller cell moves every tile too", "[TileSet]")
{
    TileSet saved{"textures/tile_set.png", 16};
    TileSet edited{"textures/tile_set.png", 8};

    auto broken = whatMovingToWouldBreak(saved, 112, edited, 112, "default");

    REQUIRE(broken.has_value());
    REQUIRE_THAT(*broken, Catch::Matchers::ContainsSubstring("7 across becomes 14"));
}

TEST_CASE("A tile set nobody changed breaks nothing", "[TileSet]")
{
    TileSet same{"textures/tile_set.png", 16};

    REQUIRE_FALSE(whatMovingToWouldBreak(same, 112, same, 112, "default").has_value());
}

TEST_CASE("Tiles across is what the sheet divides into", "[TileSet]")
{
    REQUIRE(tilesAcross(112, 16) == 7);
    REQUIRE(tilesAcross(128, 16) == 8);
    REQUIRE(tilesAcross(112, 8) == 14);
    REQUIRE(tilesAcross(112, 0) == 0);
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
