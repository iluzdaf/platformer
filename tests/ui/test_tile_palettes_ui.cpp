#include <map>
#include <optional>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "assets/asset_paths.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_palette_data.hpp"
#include "game/level.hpp"
#include "game/level_data_file.hpp"
#include "game/game_data.hpp"
#include "test_helpers/asset_path.hpp"
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
#include <cstddef>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "assets/asset_paths.hpp"
#include "rendering/texture_cache.hpp"
#include "test_helpers/headless_imgui.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_palette_data.hpp"
#include "ui/armed.hpp"
#include "ui/editor_commands.hpp"
#include "game/level.hpp"
#include "game/game_data.hpp"
#include "test_helpers/asset_path.hpp"
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

    struct TypingAName
    {
        TextureCache textures;
        std::optional<Armed> armed;
        EditorCommands commands;

        explicit TypingAName(const TilePalettes &palettes)
        {
            textures.warm(palettes.begin()->second.tileSet.texture);
        }

        auto drawing(TilePalettesUi &tilePalettesUi, TilePalettes &palettes)
        {
            return [&] { tilePalettesUi.draw(palettes, textures, commands, armed); };
        }
    };
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

    TilePaletteData made;
    made.tileSet = SheetData{std::string(assets::TileSetTexture), glm::ivec2(16)};
    palettes.insert({"ice", made});

    REQUIRE(palettes["ice"].tiles.empty());
    REQUIRE(tilePalettesUi.unsavedSince(palettes));
}

TEST_CASE("A name typed is not a rename until it is entered", "[TilePalettesUi]")
{
    HeadlessImGui gui;
    TilePalettesUi tilePalettesUi;
    TilePalettes palettes;
    palettes["default"] = paletteOf({{0, TileData{}}});

    REQUIRE_FALSE(tilePalettesUi.unsavedSince(palettes));

    TypingAName renaming(palettes);
    auto drawing = renaming.drawing(tilePalettesUi, palettes);

    gui.type("##name", "base", drawing);

    REQUIRE_FALSE(tilePalettesUi.unsavedSince(palettes));

    gui.pressEnter(drawing);

    REQUIRE(tilePalettesUi.unsavedSince(palettes));
}

TEST_CASE("A palette keeps the name the levels know until it is saved", "[TilePalettesUi]")
{
    HeadlessImGui gui;
    TilePalettesUi tilePalettesUi;
    TilePalettes palettes;
    palettes["default"] = paletteOf({{0, TileData{}}});

    TypingAName renaming(palettes);
    auto drawing = renaming.drawing(tilePalettesUi, palettes);

    gui.type("##name", "base", drawing);
    gui.pressEnter(drawing);

    REQUIRE(palettes.contains("default"));
    REQUIRE_FALSE(palettes.contains("base"));
}

TEST_CASE("A name already taken is not entered", "[TilePalettesUi]")
{
    HeadlessImGui gui;
    TilePalettesUi tilePalettesUi;
    TilePalettes palettes;
    palettes["default"] = paletteOf({{0, TileData{}}});
    palettes["other"] = paletteOf({{0, TileData{}}});

    REQUIRE_FALSE(tilePalettesUi.unsavedSince(palettes));

    TypingAName renaming(palettes);
    auto drawing = renaming.drawing(tilePalettesUi, palettes);

    gui.type("##name", "other", drawing);
    gui.pressEnter(drawing);

    REQUIRE_FALSE(tilePalettesUi.unsavedSince(palettes));
}

TEST_CASE("Reverting takes back a rename that was never saved", "[TilePalettesUi]")
{
    HeadlessImGui gui;
    TilePalettesUi tilePalettesUi;
    TilePalettes palettes;
    palettes["default"] = paletteOf({{0, TileData{}}});

    REQUIRE_FALSE(tilePalettesUi.unsavedSince(palettes));

    TypingAName renaming(palettes);
    auto drawing = renaming.drawing(tilePalettesUi, palettes);

    gui.type("##name", "base", drawing);
    gui.pressEnter(drawing);
    REQUIRE(tilePalettesUi.unsavedSince(palettes));

    tilePalettesUi.revert(palettes);

    REQUIRE_FALSE(tilePalettesUi.unsavedSince(palettes));
    REQUIRE(palettes.contains("default"));
}

TEST_CASE("A level still loads while a palette rename waits to be saved", "[TilePalettesUi]")
{
    HeadlessImGui gui;
    TilePalettesUi tilePalettesUi;
    GameData gameData = loadGameData();

    TypingAName renaming(gameData.tilePalettes);
    auto drawing = renaming.drawing(tilePalettesUi, gameData.tilePalettes);

    gui.type("##name", "default1", drawing);
    gui.pressEnter(drawing);

    REQUIRE(tilePalettesUi.unsavedSince(gameData.tilePalettes));
    REQUIRE_NOTHROW(Level(
        readLevelData(assetPath("levels/level1.json")),
        gameData.tilePalettes,
        gameData.playerData,
        gameData.npcData,
        gameData.pickupData));
}

TEST_CASE("A palette rename outlives a reload of the values it waits on", "[TilePalettesUi]")
{
    HeadlessImGui gui;
    TilePalettesUi tilePalettesUi;
    TilePalettes palettes;
    palettes["default"] = paletteOf({{0, TileData{}}});

    REQUIRE_FALSE(tilePalettesUi.unsavedSince(palettes));

    TypingAName renaming(palettes);
    auto drawing = renaming.drawing(tilePalettesUi, palettes);

    gui.type("##name", "base", drawing);
    gui.pressEnter(drawing);
    REQUIRE(tilePalettesUi.unsavedSince(palettes));

    tilePalettesUi.valuesReplaced();

    REQUIRE(tilePalettesUi.unsavedSince(palettes));
}

#endif // SKIP_OPENGL_TESTS

TEST_CASE("Reverting takes back a palette that was added", "[TilePalettesUi]")
{
    TilePalettesUi tilePalettesUi;
    TilePalettes palettes = shippedPalettes();
    std::size_t before = palettes.size();

    REQUIRE_FALSE(tilePalettesUi.unsavedSince(palettes));

    TilePaletteData made;
    made.tileSet = SheetData{std::string(assets::TileSetTexture), glm::ivec2(16)};
    palettes.insert({"ice", made});

    tilePalettesUi.revert(palettes);

    REQUIRE(palettes.size() == before);
    REQUIRE_FALSE(palettes.contains("ice"));
}
