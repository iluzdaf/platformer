#ifndef SKIP_OPENGL_TESTS

#include <optional>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include "assets/asset_paths.hpp"
#include "rendering/texture_cache.hpp"
#include "test_helpers/headless_imgui.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_palette.hpp"
#include "ui/armed.hpp"
#include "ui/tile_palettes_ui.hpp"

namespace
{
    TilePalettes twoPalettes()
    {
        TilePalettes palettes;
        palettes["default"] = paletteOf({{0, TileData{}}, {1, TileData{}}});
        palettes["other"] = paletteOf({{0, TileData{}}});
        palettes["other"].tileSet.texture = std::string(assets::PlayerTexture);
        return palettes;
    }
}

TEST_CASE("The palette editor draws a palette no level is using", "[TilePalettesUi]")
{
    HeadlessImGui gui;
    TilePalettesUi tilePalettesUi;
    TilePalettes palettes = twoPalettes();

    TextureCache textures;
    textures.warm(palettes["default"].tileSet.texture);
    textures.warm(palettes["other"].tileSet.texture);

    std::optional<Armed> armed;

    REQUIRE_NOTHROW(gui.frame([&] { tilePalettesUi.draw(palettes, textures, armed); }));
}

TEST_CASE("The palette editor survives a tile set that is not loaded", "[TilePalettesUi]")
{
    HeadlessImGui gui;
    TilePalettesUi tilePalettesUi;
    TilePalettes palettes;
    palettes["half typed"] = paletteOf({{0, TileData{}}});
    palettes["half typed"].tileSet.texture = "textures/tile_se";

    TextureCache textures;
    std::optional<Armed> armed;

    REQUIRE_NOTHROW(gui.frame([&] { tilePalettesUi.draw(palettes, textures, armed); }));
}

TEST_CASE("The palette editor reports unsaved once a tile set changes", "[TilePalettesUi]")
{
    HeadlessImGui gui;
    TilePalettesUi tilePalettesUi;
    TilePalettes palettes = twoPalettes();

    TextureCache textures;
    textures.warm(palettes["default"].tileSet.texture);
    textures.warm(palettes["other"].tileSet.texture);

    std::optional<Armed> armed;
    gui.frame([&] { tilePalettesUi.draw(palettes, textures, armed); });

    REQUIRE_FALSE(tilePalettesUi.hasUnsavedChanges(palettes));

    palettes["default"].tileSet.texture = std::string(assets::PlayerTexture);

    REQUIRE(tilePalettesUi.hasUnsavedChanges(palettes));
}

#endif // SKIP_OPENGL_TESTS
