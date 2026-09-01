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

TEST_CASE("A palette nobody changed can be saved", "[TilePalettesUi]")
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

    REQUIRE_FALSE(tilePalettesUi.cannotSave(palettes, textures).has_value());
}

TEST_CASE("A tile set of the same grid can be saved", "[TilePalettesUi]")
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

    palettes["default"].tileSet.texture = std::string(assets::TileSetTexture);
    palettes["default"].tileSet.tileSize = 16;

    REQUIRE_FALSE(tilePalettesUi.cannotSave(palettes, textures).has_value());
}

TEST_CASE("A tile set that would move every tile cannot be saved", "[TilePalettesUi]")
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

    palettes["default"].tileSet.texture = std::string(assets::PlayerTexture);

    std::optional<std::string> reason = tilePalettesUi.cannotSave(palettes, textures);

    REQUIRE(reason.has_value());
    REQUIRE_THAT(*reason, Catch::Matchers::ContainsSubstring("7 across becomes 6"));
    REQUIRE_THAT(*reason, Catch::Matchers::ContainsSubstring("default"));
}

TEST_CASE("A tile set nobody has loaded cannot be compared against", "[TilePalettesUi]")
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

    palettes["default"].tileSet.texture = "textures/not_loaded.png";

    REQUIRE(tilePalettesUi.cannotSave(palettes, textures).has_value());
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

#endif // SKIP_OPENGL_TESTS
