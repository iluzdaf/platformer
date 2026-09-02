#ifndef SKIP_OPENGL_TESTS

#include <optional>
#include <tuple>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "assets/asset_paths.hpp"
#include "rendering/texture_cache.hpp"
#include "test_helpers/headless_imgui.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_palette.hpp"
#include "ui/armed.hpp"
#include "ui/editor_commands.hpp"
#include "ui/tile_palettes_ui.hpp"
#include "ui/tile_picker.hpp"

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
    EditorCommands commands;

    REQUIRE_NOTHROW(gui.frame([&] { tilePalettesUi.draw(palettes, textures, commands, armed); }));
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
    EditorCommands commands;

    REQUIRE_NOTHROW(gui.frame([&] { tilePalettesUi.draw(palettes, textures, commands, armed); }));
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
    EditorCommands commands;
    gui.frame([&] { tilePalettesUi.draw(palettes, textures, commands, armed); });

    REQUIRE_FALSE(tilePalettesUi.hasUnsavedChanges(palettes));

    palettes["default"].tileSet.texture = std::string(assets::PlayerTexture);

    REQUIRE(tilePalettesUi.hasUnsavedChanges(palettes));
}

TEST_CASE("A tile set nobody loaded is asked for once", "[TilePalettesUi]")
{
    HeadlessImGui gui;
    TilePalettesUi tilePalettesUi;
    TilePalettes palettes;
    palettes["ice"] = paletteOf({{0, TileData{}}});
    palettes["ice"].tileSet.texture = "textures/not_loaded.png";

    TextureCache textures;
    std::optional<Armed> armed;
    EditorCommands commands;

    int asked = 0;
    std::ignore = commands.onWarmTexture.connect([&](const std::string &) { ++asked; });

    for (int frame = 0; frame < 3; ++frame)
    {
        gui.frame([&] { tilePalettesUi.draw(palettes, textures, commands, armed); });
        commands.drain();
    }

    REQUIRE(asked == 1);
}

TEST_CASE("The palette editor offers every cell of its sheet", "[TilePalettesUi]")
{
    HeadlessImGui gui;
    TilePalettesUi tilePalettesUi;
    TilePalettes palettes;
    palettes["default"] = paletteOf({{0, TileData{}}});

    TextureCache textures;
    textures.warm(palettes["default"].tileSet.texture);

    std::optional<Armed> armed;
    EditorCommands commands;

    gui.frame([&] { tilePalettesUi.draw(palettes, textures, commands, armed); });

    REQUIRE(tilesToPickFrom(textures.get(palettes["default"].tileSet.texture), 16).size() == 49);
    REQUIRE(palettes["default"].tiles.size() == 1);
}

TEST_CASE("Looking at a cell does not give it settings", "[TilePalettesUi]")
{
    HeadlessImGui gui;
    TilePalettesUi tilePalettesUi;
    TilePalettes palettes;
    palettes["default"] = paletteOf({{0, TileData{}}});

    TextureCache textures;
    textures.warm(palettes["default"].tileSet.texture);

    std::optional<Armed> armed = PaintTile{30};
    EditorCommands commands;

    for (int frame = 0; frame < 3; ++frame)
        gui.frame([&] { tilePalettesUi.draw(palettes, textures, commands, armed); });

    REQUIRE_FALSE(palettes["default"].tiles.contains(30));
    REQUIRE(palettes["default"].tiles.size() == 1);
}

#endif // SKIP_OPENGL_TESTS
