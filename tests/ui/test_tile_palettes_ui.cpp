#include <map>
#include <optional>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "assets/asset_paths.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_palette.hpp"
#include "game/level.hpp"
#include "ui/palette_renamed.hpp"
#include "ui/tile_palettes_ui.hpp"

namespace
{
    TilePalettes namedPalettes()
    {
        TilePalettes palettes;
        palettes["default"] = paletteOf({{0, TileData{}}});
        palettes["other"] = paletteOf({{0, TileData{}}});
        return palettes;
    }
}

TEST_CASE("A rename needs a name nobody has taken", "[TilePalettesUi]")
{
    TilePalettes palettes = namedPalettes();

    REQUIRE(whyNotARename(palettes, "default", "").has_value());
    REQUIRE_FALSE(whyNotARename(palettes, "default", "base").has_value());
}

TEST_CASE("A rename to the name it already has is allowed and does nothing", "[TilePalettesUi]")
{
    TilePalettes palettes = namedPalettes();

    REQUIRE_FALSE(whyNotARename(palettes, "default", "default").has_value());
}

TEST_CASE("A rename cannot take a name already taken", "[TilePalettesUi]")
{
    TilePalettes palettes = namedPalettes();

    std::optional<std::string> why = whyNotARename(palettes, "default", "other");

    REQUIRE(why.has_value());
    REQUIRE_THAT(*why, Catch::Matchers::ContainsSubstring("already a palette"));
    REQUIRE_THAT(*why, Catch::Matchers::ContainsSubstring("other"));
}

TEST_CASE("A rename nobody's level uses rewrites nothing", "[TilePalettesUi]")
{
    REQUIRE(renamePaletteInLevels(std::string(assets::Levels), {{"nobody", "somebody"}}) == 0);
}

TEST_CASE("Renaming twice remembers the name that is on disk", "[TilePalettesUi]")
{
    std::map<std::string, std::string> renames;

    rememberRename(renames, "default", "base");
    REQUIRE(renames == std::map<std::string, std::string>{{"default", "base"}});

    rememberRename(renames, "base", "ground");
    REQUIRE(renames == std::map<std::string, std::string>{{"default", "ground"}});
}

TEST_CASE("Renaming back to the name on disk remembers nothing", "[TilePalettesUi]")
{
    std::map<std::string, std::string> renames;

    rememberRename(renames, "default", "base");
    rememberRename(renames, "base", "default");

    REQUIRE(renames.empty());
}

TEST_CASE("An added palette gets a name nobody has taken", "[TilePalettesUi]")
{
    TilePalettes palettes = namedPalettes();

    std::string name = aNameNobodyHasTaken(palettes);

    REQUIRE_FALSE(name.empty());
    REQUIRE_FALSE(palettes.contains(name));

    palettes[name] = paletteOf({{0, TileData{}}});
    REQUIRE(aNameNobodyHasTaken(palettes) != name);
}

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
#include "game/level.hpp"
#include "ui/palette_renamed.hpp"
#include "ui/tile_palettes_ui.hpp"
#include "rendering/tile_set_textures.hpp"

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

    REQUIRE_FALSE(tilePalettesUi.unsavedSince(palettes));

    palettes["default"].tileSet.texture = std::string(assets::PlayerTexture);

    REQUIRE(tilePalettesUi.unsavedSince(palettes));
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

    REQUIRE(tilesInSheet(112, 112, 16) == 49);
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

TEST_CASE(
    "A palette made from an image starts with nothing said about its tiles",
    "[TilePalettesUi]")
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
    REQUIRE_FALSE(tilePalettesUi.unsavedSince(palettes));

    TilePalette made;
    made.tileSet = TileSet{std::string(assets::TileSetTexture), 16};
    palettes.insert({"ice", made});

    REQUIRE(palettes["ice"].tiles.empty());
    REQUIRE(tilesInSheet(112, 112, palettes["ice"].tileSet.tileSize) == 49);
    REQUIRE(tilePalettesUi.unsavedSince(palettes));
}

TEST_CASE("A rename is handed back so the level can be told", "[TilePalettesUi]")
{
    HeadlessImGui gui;
    TilePalettesUi tilePalettesUi;
    TilePalettes palettes;
    palettes["default"] = paletteOf({{0, TileData{}}});

    TextureCache textures;
    textures.warm(palettes["default"].tileSet.texture);

    std::optional<Armed> armed;
    EditorCommands commands;

    std::optional<PaletteRenamed> renamed;
    gui.frame([&] { renamed = tilePalettesUi.draw(palettes, textures, commands, armed); });

    REQUIRE_FALSE(renamed.has_value());
    REQUIRE(palettes.contains("default"));
}

#endif // SKIP_OPENGL_TESTS
