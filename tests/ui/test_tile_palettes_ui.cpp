#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <string>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "assets/asset_paths.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_map.hpp"
#include "tile_map/tile_map_data.hpp"
#include "tile_map/tile_palette_data.hpp"
#include "game/level.hpp"
#include "game/level_data.hpp"
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

    std::filesystem::path aLevelNaming(const std::string &palette)
    {
        std::filesystem::path directory =
            std::filesystem::temp_directory_path() / "platformer_palette_levels";
        std::filesystem::remove_all(directory);
        std::filesystem::create_directories(directory);

        LevelData levelData = readLevelData(assetPath("levels/level1.json"));
        levelData.tileMapData.tilePalette = palette;
        writeLevelData(levelData, (directory / "level1.json").string());

        return directory;
    }

    std::string paletteNamedIn(const std::filesystem::path &directory)
    {
        return readLevelData((directory / "level1.json").string()).tileMapData.tilePalette;
    }

    TileMapData aTileMapOn(const std::string &palette)
    {
        TileMapData tileMapData;
        tileMapData.indices = std::vector<std::vector<int>>(2, std::vector<int>(2, 0));
        tileMapData.tilePalette = palette;
        return tileMapData;
    }
}

TEST_CASE("Removing a palette re-points its levels to the first that remains", "[TilePalettesUi]")
{
    std::filesystem::path directory = aLevelNaming("default");
    std::optional<TilePalettes> written;
    TilePalettesUi tilePalettesUi(
        directory.string(), [&](const TilePalettes &palettes) { written = palettes; });
    TilePalettes palettes = namedPalettes();
    REQUIRE_FALSE(tilePalettesUi.unsavedSince(palettes));

    tilePalettesUi.show("default");
    tilePalettesUi.remove(palettes);

    LevelData playing = readLevelData((directory / "level1.json").string());
    REQUIRE(tilePalettesUi.save(palettes, playing));

    REQUIRE(written.has_value());
    REQUIRE_FALSE(written->contains("default"));
    REQUIRE(paletteNamedIn(directory) == "other");
    REQUIRE(playing.tileMapData.tilePalette == "other");
    REQUIRE_NOTHROW(TileMap(playing.tileMapData, palettes));

    std::filesystem::remove_all(directory);
}

TEST_CASE("Removing the last palette leaves its levels naming it", "[TilePalettesUi]")
{
    std::filesystem::path directory = aLevelNaming("only");
    std::optional<TilePalettes> written;
    TilePalettesUi tilePalettesUi(
        directory.string(), [&](const TilePalettes &palettes) { written = palettes; });
    TilePalettes palettes;
    palettes["only"] = paletteOf({{0, TileData{}}});
    REQUIRE_FALSE(tilePalettesUi.unsavedSince(palettes));

    tilePalettesUi.show("only");
    tilePalettesUi.remove(palettes);
    REQUIRE(tilePalettesUi.unsavedSince(palettes));

    LevelData playing = readLevelData((directory / "level1.json").string());
    REQUIRE_FALSE(tilePalettesUi.save(palettes, playing));

    REQUIRE(written.has_value());
    REQUIRE(written->empty());
    REQUIRE(paletteNamedIn(directory) == "only");
    REQUIRE(playing.tileMapData.tilePalette == "only");
    REQUIRE_THROWS_WITH(
        TileMap(playing.tileMapData, palettes),
        Catch::Matchers::ContainsSubstring("Unknown tile palette"));

    std::filesystem::remove_all(directory);
}

TEST_CASE("A palette removed stays until the save and shows another", "[TilePalettesUi]")
{
    TilePalettesUi tilePalettesUi;
    TilePalettes palettes = namedPalettes();
    REQUIRE_FALSE(tilePalettesUi.unsavedSince(palettes));

    tilePalettesUi.show("other");
    tilePalettesUi.remove(palettes);

    REQUIRE(palettes.contains("other"));
    REQUIRE(tilePalettesUi.shownPalette() == "default");
    REQUIRE(tilePalettesUi.unsavedSince(palettes));
}

TEST_CASE("A level still builds while a palette removal waits to be saved", "[TilePalettesUi]")
{
    TilePalettesUi tilePalettesUi;
    TilePalettes palettes = namedPalettes();
    REQUIRE_FALSE(tilePalettesUi.unsavedSince(palettes));

    tilePalettesUi.show("other");
    tilePalettesUi.remove(palettes);

    REQUIRE_NOTHROW(TileMap(aTileMapOn("other"), palettes));
}

TEST_CASE("A palette added and removed before a save vanishes at once", "[TilePalettesUi]")
{
    TilePalettesUi tilePalettesUi;
    TilePalettes palettes = namedPalettes();
    REQUIRE_FALSE(tilePalettesUi.unsavedSince(palettes));

    tilePalettesUi.add(palettes);
    std::string added = tilePalettesUi.shownPalette();
    REQUIRE(palettes.contains(added));

    tilePalettesUi.remove(palettes);

    REQUIRE_FALSE(palettes.contains(added));
    REQUIRE_FALSE(tilePalettesUi.unsavedSince(palettes));
}

TEST_CASE("Reverting puts back a palette that was removed", "[TilePalettesUi]")
{
    TilePalettesUi tilePalettesUi;
    TilePalettes palettes = namedPalettes();
    REQUIRE_FALSE(tilePalettesUi.unsavedSince(palettes));

    tilePalettesUi.show("other");
    tilePalettesUi.remove(palettes);
    tilePalettesUi.revert(palettes);

    REQUIRE(palettes.contains("other"));
    REQUIRE_FALSE(tilePalettesUi.unsavedSince(palettes));

    tilePalettesUi.show("other");
    REQUIRE_NOTHROW(TileMap(aTileMapOn("other"), palettes));
}

TEST_CASE(
    "Saving a removal re-points the levels before the palettes are written",
    "[TilePalettesUi]")
{
    std::filesystem::path directory = aLevelNaming("other");
    std::optional<TilePalettes> written;
    TilePalettesUi tilePalettesUi(
        directory.string(),
        [&](const TilePalettes &palettes)
        {
            REQUIRE(paletteNamedIn(directory) == "default");
            written = palettes;
        });
    TilePalettes palettes = namedPalettes();
    REQUIRE_FALSE(tilePalettesUi.unsavedSince(palettes));

    tilePalettesUi.show("other");
    tilePalettesUi.remove(palettes);

    LevelData playing = readLevelData((directory / "level1.json").string());
    REQUIRE(tilePalettesUi.save(palettes, playing));

    REQUIRE(written.has_value());
    REQUIRE_FALSE(written->contains("other"));
    REQUIRE_FALSE(palettes.contains("other"));
    REQUIRE_FALSE(tilePalettesUi.unsavedSince(palettes));
    REQUIRE(playing.tileMapData.tilePalette == "default");

    std::filesystem::remove_all(directory);
}

TEST_CASE("Saving with nothing pending leaves the playing level alone", "[TilePalettesUi]")
{
    std::filesystem::path directory = aLevelNaming("other");
    bool wrote = false;
    TilePalettesUi tilePalettesUi(directory.string(), [&](const TilePalettes &) { wrote = true; });
    TilePalettes palettes = namedPalettes();
    REQUIRE_FALSE(tilePalettesUi.unsavedSince(palettes));

    LevelData playing = readLevelData((directory / "level1.json").string());
    REQUIRE_FALSE(tilePalettesUi.save(palettes, playing));

    REQUIRE(wrote);
    REQUIRE(playing.tileMapData.tilePalette == "other");

    std::filesystem::remove_all(directory);
}

TEST_CASE("A removal cannot be saved while a level cannot be read", "[TilePalettesUi]")
{
    std::filesystem::path directory = aLevelNaming("other");
    std::ofstream(directory / "broken.json") << "{";
    bool wrote = false;
    TilePalettesUi tilePalettesUi(directory.string(), [&](const TilePalettes &) { wrote = true; });
    TilePalettes palettes = namedPalettes();
    REQUIRE_FALSE(tilePalettesUi.unsavedSince(palettes));

    tilePalettesUi.show("other");
    tilePalettesUi.remove(palettes);

    REQUIRE(tilePalettesUi.cannotSaveBecause() == "broken cannot be read");

    LevelData playing = readLevelData((directory / "level1.json").string());
    REQUIRE_FALSE(tilePalettesUi.save(palettes, playing));

    REQUIRE_FALSE(wrote);
    REQUIRE(palettes.contains("other"));
    REQUIRE(playing.tileMapData.tilePalette == "other");
    REQUIRE(paletteNamedIn(directory) == "other");
    REQUIRE(tilePalettesUi.unsavedSince(palettes));

    std::filesystem::remove_all(directory);
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
#include <imgui_internal.h>
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

TEST_CASE("The palette editor says when its texture is not under textures", "[TilePalettesUi]")
{
    HeadlessImGui gui;
    TextureCache textures;
    std::optional<Armed> armed;
    EditorCommands commands;

    auto heightNaming = [&](const std::string &texture)
    {
        TilePalettesUi tilePalettesUi;
        TilePalettes palettes;
        palettes["ice"] = paletteOf({{0, TileData{}}});
        palettes["ice"].tileSet.texture = texture;

        float reached = 0.0f;
        gui.frame(
            [&]
            {
                tilePalettesUi.draw(palettes, textures, commands, armed);
                reached = ImGui::GetCurrentWindow()->DC.CursorPos.y;
            });

        return reached;
    };

    REQUIRE(heightNaming("textures/nowhere.png") > heightNaming("textures/coin.png"));
}

TEST_CASE("The palette editor says when its cells are not square", "[TilePalettesUi]")
{
    HeadlessImGui gui;
    TextureCache textures;
    std::optional<Armed> armed;
    EditorCommands commands;

    auto heightWithCells = [&](glm::ivec2 cellSize)
    {
        TilePalettesUi tilePalettesUi;
        TilePalettes palettes;
        palettes["ice"] = paletteOf({{0, TileData{}}});
        palettes["ice"].tileSet.cellSize = cellSize;

        float reached = 0.0f;
        gui.frame(
            [&]
            {
                tilePalettesUi.draw(palettes, textures, commands, armed);
                reached = ImGui::GetCurrentWindow()->DC.CursorPos.y;
            });

        return reached;
    };

    REQUIRE(heightWithCells(glm::ivec2(16, 24)) > heightWithCells(glm::ivec2(16)));
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
    TilePalettes palettes = twoPalettes();

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
    TilePalettes palettes = twoPalettes();

    TypingAName renaming(palettes);
    auto drawing = renaming.drawing(tilePalettesUi, palettes);

    gui.type("##name", "base", drawing);
    gui.pressEnter(drawing);

    REQUIRE(palettes.contains("default"));
    REQUIRE_FALSE(palettes.contains("base"));
}

TEST_CASE(
    "A removal falls back to the name the first remaining palette will have",
    "[TilePalettesUi]")
{
    HeadlessImGui gui;
    std::filesystem::path directory = aLevelNaming("default");
    bool wrote = false;
    TilePalettesUi tilePalettesUi(directory.string(), [&](const TilePalettes &) { wrote = true; });
    TilePalettes palettes = twoPalettes();
    REQUIRE_FALSE(tilePalettesUi.unsavedSince(palettes));

    TypingAName renaming(palettes);
    tilePalettesUi.show("other");
    auto drawing = renaming.drawing(tilePalettesUi, palettes);
    gui.type("##name", "base", drawing);
    gui.pressEnter(drawing);

    tilePalettesUi.show("default");
    tilePalettesUi.remove(palettes);

    LevelData playing = readLevelData((directory / "level1.json").string());
    REQUIRE(tilePalettesUi.save(palettes, playing));

    REQUIRE(wrote);
    REQUIRE(palettes.contains("base"));
    REQUIRE(palettes.size() == 1);
    REQUIRE(paletteNamedIn(directory) == "base");
    REQUIRE(playing.tileMapData.tilePalette == "base");

    std::filesystem::remove_all(directory);
}

TEST_CASE("A removed name taken by a new palette keeps the levels on it", "[TilePalettesUi]")
{
    HeadlessImGui gui;
    std::filesystem::path directory = aLevelNaming("other");
    bool wrote = false;
    TilePalettesUi tilePalettesUi(
        directory.string(),
        [&](const TilePalettes &)
        {
            REQUIRE(paletteNamedIn(directory) == "other");
            wrote = true;
        });
    TilePalettes palettes = twoPalettes();

    REQUIRE_FALSE(tilePalettesUi.unsavedSince(palettes));

    TypingAName renaming(palettes);
    tilePalettesUi.show("other");
    tilePalettesUi.remove(palettes);
    tilePalettesUi.add(palettes);
    auto drawing = renaming.drawing(tilePalettesUi, palettes);

    gui.type("##name", "other", drawing);
    gui.pressEnter(drawing);

    LevelData playing = readLevelData((directory / "level1.json").string());
    REQUIRE_FALSE(tilePalettesUi.save(palettes, playing));

    REQUIRE(wrote);
    REQUIRE(playing.tileMapData.tilePalette == "other");
    REQUIRE(palettes.size() == 2);
    REQUIRE(palettes.at("other").tiles.empty());
    REQUIRE(tilePalettesUi.shownPalette() == "other");

    std::filesystem::remove_all(directory);
}

TEST_CASE("A name already taken is not entered", "[TilePalettesUi]")
{
    HeadlessImGui gui;
    TilePalettesUi tilePalettesUi;
    TilePalettes palettes = twoPalettes();

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
    TilePalettes palettes = twoPalettes();

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
    TilePalettes palettes = twoPalettes();

    REQUIRE_FALSE(tilePalettesUi.unsavedSince(palettes));

    TypingAName renaming(palettes);
    auto drawing = renaming.drawing(tilePalettesUi, palettes);

    gui.type("##name", "base", drawing);
    gui.pressEnter(drawing);
    REQUIRE(tilePalettesUi.unsavedSince(palettes));

    tilePalettesUi.reloaded(palettes, palettes);

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
