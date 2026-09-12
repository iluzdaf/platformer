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
#include "cameras/camera2d.hpp"
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "game/level_data_file.hpp"
#include "npc/npc_spawn_data.hpp"
#include "pickups/pickup_data.hpp"
#include "ui/editor_section.hpp"
#include "ui/editor_ui.hpp"
#include <string>
#include <exception>
#include "npc/npc.hpp"
#include "npc/npc_data.hpp"

#include "helpers/small_game.hpp"

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

    for (EditorSection listed : {EditorSection::Game, EditorSection::Runtime, EditorSection::Level})
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

    editing.levels.first.path = "levels/elsewhere.json";

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
    std::string firstWas = editing.levels.first.path;
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Level, subject).unsaved);
    editing.gameData.tilePalettes.begin()->second.tiles[0].solid = true;
    editing.levels.first.path = "levels/elsewhere.json";

    editorUi.savingIn(EditorSection::Level, subject).revert();
    editorUi.commands.drain();

    REQUIRE_FALSE(editing.gameData.tilePalettes.begin()->second.tiles[0].solid);
    REQUIRE(editing.levels.first.path == firstWas);
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
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Level, subject).unsaved);
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Runtime, subject).unsaved);

    GameData onDisk = editing.gameData;
    editing.gameData.playerData.fallFromHeightThreshold += 100.0f;
    float edited = editing.gameData.playerData.fallFromHeightThreshold;

    onDisk.cameraData.zoom += 1.0f;
    editorUi.reloaded(editing.gameData, onDisk);

    REQUIRE(editing.gameData.playerData.fallFromHeightThreshold == edited);
    REQUIRE(editing.gameData.cameraData.zoom == onDisk.cameraData.zoom);
    REQUIRE(editorUi.savingIn(EditorSection::Level, subject).unsaved);
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Runtime, subject).unsaved);
}

TEST_CASE("Reverting a section kept through a reload takes what is on disk now", "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;
    EditorSubject subject = editing.subject();
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Level, subject).unsaved);

    GameData onDisk = editing.gameData;
    editing.gameData.playerData.fallFromHeightThreshold += 100.0f;

    onDisk.playerData.fallFromHeightThreshold += 50.0f;
    editorUi.reloaded(editing.gameData, onDisk);
    editorUi.savingIn(EditorSection::Level, subject).revert();

    REQUIRE(
        editing.gameData.playerData.fallFromHeightThreshold ==
        onDisk.playerData.fallFromHeightThreshold);
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Level, subject).unsaved);
}

TEST_CASE(
    "The level being played takes the disk only while it is clean and the file changed",
    "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;
    EditorSubject subject = editing.subject();
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Level, subject).unsaved);
    REQUIRE_FALSE(editorUi.levelTakesTheDisk(editing.levelData, editing.levelPath));

    LevelData changedOnDisk = editing.levelData;
    changedOnDisk.playerFeet.x += 16.0f;
    writeLevelData(changedOnDisk, editing.levelPath);
    REQUIRE(editorUi.levelTakesTheDisk(editing.levelData, editing.levelPath));

    editing.levelData.playerFeet.x += 32.0f;

    REQUIRE_FALSE(editorUi.levelTakesTheDisk(editing.levelData, editing.levelPath));
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

TEST_CASE("The game section refuses a window nobody can see, as the window would", "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;
    EditorSubject subject = editing.subject();
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Game, subject).cannotBecause.has_value());

    editing.gameData.settings.windowHeight = 0;

    SectionSaving saving = editorUi.savingIn(EditorSection::Game, subject);
    REQUIRE(saving.cannotBecause.has_value());
    REQUIRE_THAT(*saving.cannotBecause, Catch::Matchers::ContainsSubstring("nobody can see"));
}

TEST_CASE("The runtime section refuses what the camera refuses", "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;
    EditorSubject subject = editing.subject();
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Runtime, subject).cannotBecause.has_value());

    editing.gameData.cameraData.zoom = 0.0f;
    std::string cameraSays;
    try
    {
        Camera2D built(editing.gameData.cameraData, 1, 1);
    }
    catch (const std::exception &e)
    {
        cameraSays = e.what();
    }

    SectionSaving saving = editorUi.savingIn(EditorSection::Runtime, subject);
    REQUIRE(saving.cannotBecause == cameraSays);
    REQUIRE_FALSE(cameraSays.empty());
}

TEST_CASE("The cast section refuses what a creature refuses when it is built", "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;
    EditorSubject subject = editing.subject();
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Level, subject).cannotBecause.has_value());

    NpcData &rat = editing.gameData.npcData.at("rat");
    rat.actorData.abilities.pounce.reset();
    std::string creatureSays;
    try
    {
        Npc built(NpcSpawnData{"rat", glm::vec2(0.0f), std::nullopt}, rat);
    }
    catch (const std::exception &e)
    {
        creatureSays = e.what();
    }

    SectionSaving saving = editorUi.savingIn(EditorSection::Level, subject);
    REQUIRE(saving.cannotBecause == "rat " + creatureSays);
    REQUIRE_THAT(creatureSays, Catch::Matchers::ContainsSubstring("no such ability"));
}

TEST_CASE("The level section refuses a first level that cannot be read", "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;
    EditorSubject subject = editing.subject();
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Level, subject).cannotBecause.has_value());

    editing.levels.first.path = "levels/nowhere.json";

    SectionSaving saving = editorUi.savingIn(EditorSection::Level, subject);
    REQUIRE(saving.cannotBecause.has_value());
    REQUIRE_THAT(*saving.cannotBecause, Catch::Matchers::ContainsSubstring("nowhere"));
    REQUIRE_THAT(*saving.cannotBecause, Catch::Matchers::ContainsSubstring("cannot be read"));
}

TEST_CASE("The cast section is unsaved when its types are", "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;
    EditorSubject subject = editing.subject();
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Level, subject).unsaved);

    editing.gameData.pickupData.begin()->second.scoreDelta += 1;

    REQUIRE(editorUi.savingIn(EditorSection::Level, subject).unsaved);
}

TEST_CASE("Reverting the cast section puts back the player and the types", "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;
    EditorSubject subject = editing.subject();
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Level, subject).unsaved);
    float thresholdWas = editing.gameData.playerData.fallFromHeightThreshold;
    int scoreWas = editing.gameData.pickupData.begin()->second.scoreDelta;
    editing.gameData.playerData.fallFromHeightThreshold += 100.0f;
    editing.gameData.pickupData.begin()->second.scoreDelta += 1;

    editorUi.savingIn(EditorSection::Level, subject).revert();

    REQUIRE(editing.gameData.playerData.fallFromHeightThreshold == thresholdWas);
    REQUIRE(editing.gameData.pickupData.begin()->second.scoreDelta == scoreWas);
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Level, subject).unsaved);
}
