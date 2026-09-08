#include "helpers/tiles.hpp"
#include "game/levels_data.hpp"
#include "game/level_data.hpp"
#include <cstddef>
#include <optional>
#include <tuple>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "actor/actor_state.hpp"
#include "cameras/camera2d.hpp"
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "game/level_data_file.hpp"
#include "npc/npc_spawn_data.hpp"
#include "rendering/texture_cache.hpp"
#include "helpers/asset_path.hpp"
#include "ui/editor_section.hpp"
#include "ui/editor_ui.hpp"

namespace
{
    LevelData level6Placing(const std::vector<NpcSpawnData> &extra)
    {
        LevelData levelData = readLevelData(assetPath("levels/level6.json"));
        for (const NpcSpawnData &spawn : extra)
            levelData.npcs.push_back(spawn);

        return levelData;
    }

    NpcSpawnData strandedVillager()
    {
        NpcSpawnData stranded{"rat", feetOf(glm::ivec2(2, 8)), std::nullopt};
        stranded.patrol = beatOf(glm::ivec2(2, 8), glm::ivec2(2, 1));
        return stranded;
    }

    struct Editing
    {
        explicit Editing(const std::vector<NpcSpawnData> &extra = {})
            : levelData(level6Placing(extra))
        {
        }

        GameData gameData = loadGameData();
        LevelsData levels = gameData.levels;
        std::string levelPath = assetPath("levels/level6.json");
        LevelData levelData;
        Level level{
            levelData,
            gameData.tilePalettes,
            gameData.playerData,
            gameData.npcData,
            gameData.pickupData};
        TextureCache textures;
        Decided decided;
        Observed observed;
        ActorState playerState;
        Camera2D camera{gameData.cameraData, 800, 600};

        EditorSubject subject()
        {
            return EditorSubject{
                gameData,
                level,
                levelData,
                levelPath,
                textures,
                levels,
                decided,
                observed,
                level.getTileMap().feetOnTile(glm::ivec2(1, 1)),
                playerState,
                camera,
                false};
        }
    };
}

TEST_CASE("A section with nothing changed offers no save", "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;

    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Runtime, editing.subject()).unsaved);
}

TEST_CASE("Every section that saves a file has a save to press", "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;

    for (EditorSection listed :
         {EditorSection::Game, EditorSection::Runtime, EditorSection::Cast, EditorSection::Level})
        REQUIRE(editorUi.savingIn(listed, editing.subject()).save != nullptr);
}

TEST_CASE("Palettes with every level readable have nothing said against a save", "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;

    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Level, editing.subject()).cannotBecause);
}

TEST_CASE("The level section is unsaved when its palettes are", "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;
    EditorSubject subject = editing.subject();
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Level, subject).unsaved);

    editing.gameData.tilePalettes.begin()->second.tiles[0].solid = true;

    REQUIRE(editorUi.savingIn(EditorSection::Level, subject).unsaved);
}

TEST_CASE("The level section is unsaved when its level list is", "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;
    EditorSubject subject = editing.subject();
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Level, subject).unsaved);

    editing.levels.first = "levels/level6.json";

    REQUIRE(editorUi.savingIn(EditorSection::Level, subject).unsaved);
}

TEST_CASE(
    "Reverting the level section puts back its palettes and list and reloads the level",
    "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;
    EditorSubject subject = editing.subject();
    std::optional<std::string> reloaded;
    std::ignore =
        editorUi.commands.onLoadLevel.connect([&](const std::string &path) { reloaded = path; });
    std::string firstWas = editing.levels.first;
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Level, subject).unsaved);
    editing.gameData.tilePalettes.begin()->second.tiles[0].solid = true;
    editing.levels.first = "levels/level6.json";

    editorUi.savingIn(EditorSection::Level, subject).revert();
    editorUi.commands.drain();

    REQUIRE_FALSE(editing.gameData.tilePalettes.begin()->second.tiles[0].solid);
    REQUIRE(editing.levels.first == firstWas);
    REQUIRE(reloaded == editing.levelPath);
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Level, subject).unsaved);
}

TEST_CASE(
    "A reload keeps the sections with unsaved edits and follows the disk for the rest",
    "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;
    EditorSubject subject = editing.subject();
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Cast, subject).unsaved);
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Runtime, subject).unsaved);

    editing.gameData.playerData.fallFromHeightThreshold += 100.0f;
    float edited = editing.gameData.playerData.fallFromHeightThreshold;

    GameData onDisk = loadGameData();
    onDisk.cameraData.zoom += 1.0f;
    editorUi.reloaded(editing.gameData, onDisk);

    REQUIRE(editing.gameData.playerData.fallFromHeightThreshold == edited);
    REQUIRE(editing.gameData.cameraData.zoom == onDisk.cameraData.zoom);
    REQUIRE(editorUi.savingIn(EditorSection::Cast, subject).unsaved);
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Runtime, subject).unsaved);
}

TEST_CASE("Reverting a section kept through a reload takes what is on disk now", "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;
    EditorSubject subject = editing.subject();
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Cast, subject).unsaved);

    editing.gameData.playerData.fallFromHeightThreshold += 100.0f;

    GameData onDisk = loadGameData();
    onDisk.playerData.fallFromHeightThreshold += 50.0f;
    editorUi.reloaded(editing.gameData, onDisk);
    editorUi.savingIn(EditorSection::Cast, subject).revert();

    REQUIRE(
        editing.gameData.playerData.fallFromHeightThreshold ==
        onDisk.playerData.fallFromHeightThreshold);
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Cast, subject).unsaved);
}

TEST_CASE("The level being played follows the disk only while it is clean", "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;
    EditorSubject subject = editing.subject();
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Level, subject).unsaved);

    REQUIRE(editorUi.levelFollowsTheDisk(editing.levelData, editing.levelPath));

    editing.levelData.playerFeet.x += 16.0f;

    REQUIRE_FALSE(editorUi.levelFollowsTheDisk(editing.levelData, editing.levelPath));
    REQUIRE(editorUi.savingIn(EditorSection::Level, subject).unsaved);
}

TEST_CASE("The runtime section saves the camera, which playback has no part in", "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;
    EditorSubject subject = editing.subject();
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Runtime, subject).unsaved);

    editing.gameData.cameraData.zoom += 1.0f;

    REQUIRE(editorUi.savingIn(EditorSection::Runtime, subject).unsaved);
    REQUIRE(editorUi.savingIn(EditorSection::Runtime, subject).save != nullptr);
}

TEST_CASE(
    "Reverting the runtime section puts the camera back and says the camera changed",
    "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;
    EditorSubject subject = editing.subject();
    bool cameraChanged = false;
    std::ignore = editorUi.commands.onCameraChanged.connect([&] { cameraChanged = true; });
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Runtime, subject).unsaved);
    float zoomWas = editing.gameData.cameraData.zoom;
    editing.gameData.cameraData.zoom += 1.0f;

    editorUi.savingIn(EditorSection::Runtime, subject).revert();
    editorUi.commands.drain();

    REQUIRE(editing.gameData.cameraData.zoom == zoomWas);
    REQUIRE(cameraChanged);
}

TEST_CASE("A level that strands an npc says so where its save would be", "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing({strandedVillager()});

    SectionSaving saving = editorUi.savingIn(EditorSection::Level, editing.subject());

    REQUIRE(saving.cannotBecause.has_value());
    REQUIRE_THAT(*saving.cannotBecause, Catch::Matchers::ContainsSubstring("cannot get back"));
}

TEST_CASE("The cast section is unsaved when its types are", "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;
    EditorSubject subject = editing.subject();
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Cast, subject).unsaved);

    editing.gameData.pickupData.begin()->second.scoreDelta += 1;

    REQUIRE(editorUi.savingIn(EditorSection::Cast, subject).unsaved);
}

TEST_CASE("Reverting the cast section puts back the player and the types", "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;
    EditorSubject subject = editing.subject();
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Cast, subject).unsaved);
    float thresholdWas = editing.gameData.playerData.fallFromHeightThreshold;
    int scoreWas = editing.gameData.pickupData.begin()->second.scoreDelta;
    editing.gameData.playerData.fallFromHeightThreshold += 100.0f;
    editing.gameData.pickupData.begin()->second.scoreDelta += 1;

    editorUi.savingIn(EditorSection::Cast, subject).revert();

    REQUIRE(editing.gameData.playerData.fallFromHeightThreshold == thresholdWas);
    REQUIRE(editing.gameData.pickupData.begin()->second.scoreDelta == scoreWas);
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Cast, subject).unsaved);
}
