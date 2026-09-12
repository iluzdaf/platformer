#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <string>
#include <tuple>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/ability_states.hpp"
#include "actor/observed.hpp"
#include "actor/actor_state.hpp"
#include "cameras/camera2d.hpp"
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "game/level_data.hpp"
#include "game/levels_data.hpp"
#include "rendering/texture_cache.hpp"
#include "tile_map/tile_palette_data.hpp"
#include "helpers/floor_level.hpp"
#include "helpers/npc_fixtures.hpp"
#include "helpers/palettes.hpp"
#include "ui/editor_commands.hpp"
#include "tile_map/tile_map_data.hpp"
#include "ui/editor_history.hpp"
#include "ui/editor_section.hpp"
#include "ui/editor_ui.hpp"

namespace
{
    GameData aGameWithOneNpc()
    {
        GameData gameData;
        gameData.tilePalettes = theOnlyPalette(aPaletteWithASolidTile());
        gameData.npcData = {{"rat", setupNpcData()}};
        return gameData;
    }

    struct Editing
    {
        GameData gameData = aGameWithOneNpc();
        LevelData levelData = aFloorLevelPlacing({});
        LevelsData levels = gameData.levels;
        Level level{
            levelData,
            gameData.tilePalettes,
            gameData.playerData,
            gameData.npcData,
            gameData.pickupData};
        TextureCache textures;
        AbilityStates states;
        Observed observed;
        ActorState playerState;
        Camera2D camera{gameData.cameraData, 800, 600};
        std::string levelPath = "levels/being_edited.json";

        EditorSubject subject()
        {
            return EditorSubject{
                gameData,
                level,
                levelData,
                levelPath,
                textures,
                levels,
                states,
                observed,
                level.getTileMap().feetOnTile(glm::ivec2(1, FloorLevelStanding)),
                playerState,
                camera,
                false};
        }
    };

    struct Announced
    {
        int settings = 0, camera = 0, cast = 0, palettes = 0;
        std::optional<LevelData> levelBuiltAgain;
        std::optional<TileMapData> tilesPainted;

        explicit Announced(EditorCommands &commands)
        {
            std::ignore = commands.onSettingsChanged.connect([this] { ++settings; });
            std::ignore = commands.onCameraChanged.connect([this] { ++camera; });
            std::ignore = commands.onCastChanged.connect([this] { ++cast; });
            std::ignore = commands.onPalettesChanged.connect([this] { ++palettes; });
            std::ignore = commands.onLevelEdited.connect([this](const LevelData &now)
                                                         { levelBuiltAgain = now; });
            std::ignore = commands.onTilesChanged.connect([this](const TileMapData &now)
                                                          { tilesPainted = now; });
        }
    };

    EditorStep aLevelAsItWas(const LevelData &levelData)
    {
        return EditorStep{EditorSection::Level, std::nullopt, levelData, glm::vec2(0.0f)};
    }

    void looksAt(EditorUi &editorUi, Editing &editing)
    {
        editorUi.remembersWhatChanged(editing.subject(), false);
    }
}

TEST_CASE("An editor nobody has edited has nothing to undo", "[EditorUndo]")
{
    EditorUi editorUi;
    Editing editing;

    looksAt(editorUi, editing);
    looksAt(editorUi, editing);

    REQUIRE_FALSE(editorUi.anythingToUndo());
    REQUIRE_FALSE(editorUi.undo(editing.subject()));
}

TEST_CASE("A camera edit goes back, and the camera hears about it", "[EditorUndo]")
{
    EditorUi editorUi;
    Editing editing;
    Announced announced(editorUi.commands);
    float was = editing.gameData.cameraData.zoom;

    looksAt(editorUi, editing);
    editing.gameData.cameraData.zoom = was + 2.0f;
    looksAt(editorUi, editing);

    REQUIRE(editorUi.anythingToUndo());
    REQUIRE(editorUi.undo(editing.subject()));
    editorUi.commands.drain();

    REQUIRE(editing.gameData.cameraData.zoom == was);
    REQUIRE(announced.camera == 1);
    REQUIRE(announced.cast == 0);
    REQUIRE(announced.settings == 0);
    REQUIRE(announced.palettes == 0);
}

TEST_CASE("A cast edit goes back, and the cast is made again", "[EditorUndo]")
{
    EditorUi editorUi;
    Editing editing;
    Announced announced(editorUi.commands);

    looksAt(editorUi, editing);
    editing.gameData.npcData.at("rat").actorData.size = glm::vec2(24.0f);
    looksAt(editorUi, editing);

    REQUIRE(editorUi.undo(editing.subject()));
    editorUi.commands.drain();

    REQUIRE(editing.gameData.npcData.at("rat").actorData.size == glm::vec2(16.0f));
    REQUIRE(announced.cast == 1);
    REQUIRE(announced.camera == 0);
}

TEST_CASE("A palette edit goes back, and the level is built again", "[EditorUndo]")
{
    EditorUi editorUi;
    Editing editing;
    Announced announced(editorUi.commands);

    looksAt(editorUi, editing);
    editing.gameData.tilePalettes.at("default").tiles.at(1).deadly = true;
    looksAt(editorUi, editing);

    REQUIRE(editorUi.undo(editing.subject()));
    editorUi.commands.drain();

    REQUIRE_FALSE(editing.gameData.tilePalettes.at("default").tiles.at(1).deadly);
    REQUIRE(announced.palettes == 1);
    REQUIRE(announced.cast == 0);
}

TEST_CASE("A settings edit goes back, and the window hears about it", "[EditorUndo]")
{
    EditorUi editorUi;
    Editing editing;
    Announced announced(editorUi.commands);
    int was = editing.gameData.settings.windowWidth;

    looksAt(editorUi, editing);
    editing.gameData.settings.windowWidth = was + 320;
    looksAt(editorUi, editing);

    REQUIRE(editorUi.undo(editing.subject()));
    editorUi.commands.drain();

    REQUIRE(editing.gameData.settings.windowWidth == was);
    REQUIRE(announced.settings == 1);
}

TEST_CASE("An edit still under the mouse is not a step yet", "[EditorUndo]")
{
    EditorUi editorUi;
    Editing editing;

    looksAt(editorUi, editing);
    editing.gameData.cameraData.zoom += 2.0f;
    editorUi.remembersWhatChanged(editing.subject(), true);

    REQUIRE_FALSE(editorUi.anythingToUndo());

    looksAt(editorUi, editing);

    REQUIRE(editorUi.anythingToUndo());
}

TEST_CASE("Edits go back one at a time, newest first", "[EditorUndo]")
{
    EditorUi editorUi;
    Editing editing;
    float zoom = editing.gameData.cameraData.zoom;

    looksAt(editorUi, editing);
    editing.gameData.cameraData.zoom = zoom + 1.0f;
    looksAt(editorUi, editing);
    editing.gameData.settings.windowWidth = 1024;
    looksAt(editorUi, editing);

    REQUIRE(editorUi.undo(editing.subject()));
    REQUIRE(editing.gameData.settings.windowWidth == 800);
    REQUIRE(editing.gameData.cameraData.zoom == zoom + 1.0f);

    REQUIRE(editorUi.undo(editing.subject()));
    REQUIRE(editing.gameData.cameraData.zoom == zoom);

    REQUIRE_FALSE(editorUi.undo(editing.subject()));
}

TEST_CASE("Undoing is not itself an edit to undo", "[EditorUndo]")
{
    EditorUi editorUi;
    Editing editing;

    looksAt(editorUi, editing);
    editing.gameData.cameraData.zoom += 2.0f;
    looksAt(editorUi, editing);

    REQUIRE(editorUi.undo(editing.subject()));
    looksAt(editorUi, editing);

    REQUIRE_FALSE(editorUi.anythingToUndo());
}

TEST_CASE("Undo shows the section the edit was made in", "[EditorUndo]")
{
    EditorUi editorUi;
    Editing editing;

    editorUi.show(EditorSection::Level);
    looksAt(editorUi, editing);
    editing.gameData.npcData.at("rat").actorData.size = glm::vec2(24.0f);
    looksAt(editorUi, editing);

    editorUi.show(EditorSection::Runtime);
    REQUIRE(editorUi.undo(editing.subject()));

    REQUIRE(editorUi.shown() == EditorSection::Level);
}

TEST_CASE("Undoing a paint asks for the tiles, not for the level again", "[EditorUndo]")
{
    EditorUi editorUi;
    Editing editing;
    Announced announced(editorUi.commands);
    LevelData asItWas = editing.levelData;

    editing.levelData.tileMapData.indices[FloorLevelRow][2] = 1;
    editorUi.remembers(aLevelAsItWas(asItWas));

    REQUIRE(editorUi.undo(editing.subject()));
    editorUi.commands.drain();

    REQUIRE(announced.tilesPainted);
    REQUIRE(announced.tilesPainted->indices == asItWas.tileMapData.indices);
    REQUIRE_FALSE(announced.levelBuiltAgain);
}

TEST_CASE("Undoing anything else about a level asks for the level again", "[EditorUndo]")
{
    EditorUi editorUi;
    Editing editing;
    Announced announced(editorUi.commands);
    LevelData asItWas = editing.levelData;

    editing.levelData.playerFeet += glm::vec2(16.0f, 0.0f);
    editorUi.remembers(aLevelAsItWas(asItWas));

    REQUIRE(editorUi.undo(editing.subject()));
    editorUi.commands.drain();

    REQUIRE(announced.levelBuiltAgain);
    REQUIRE(announced.levelBuiltAgain->playerFeet == asItWas.playerFeet);
    REQUIRE_FALSE(announced.tilesPainted);
}
