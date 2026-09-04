#include "test_helpers/test_tile_map_utils.hpp"
#include "game/levels_data.hpp"
#include "game/level_data.hpp"
#include <cstddef>
#include <optional>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/actor_motion_state.hpp"
#include "actor/actor_state.hpp"
#include "cameras/camera2d.hpp"
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "game/level_data_file.hpp"
#include "npc/npc_spawn_data.hpp"
#include "rendering/texture_cache.hpp"
#include "test_helpers/asset_path.hpp"
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
        NpcSpawnData stranded{"villager", feetOf(glm::ivec2(2, 8)), std::nullopt};
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
        ActorMotionState motion;
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
                motion,
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

    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Camera, editing.subject()).unsaved);
}

TEST_CASE("Every section that saves a file has a save to press", "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;

    for (EditorSection listed :
         {EditorSection::Game,
          EditorSection::Camera,
          EditorSection::Player,
          EditorSection::Levels,
          EditorSection::Level,
          EditorSection::Types,
          EditorSection::TilePalettes})
        REQUIRE(editorUi.savingIn(listed, editing.subject()).save != nullptr);
}

TEST_CASE("Palettes with every level readable have nothing said against a save", "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;

    REQUIRE_FALSE(editorUi.savingIn(EditorSection::TilePalettes, editing.subject()).cannotBecause);
}

TEST_CASE(
    "A reload keeps the sections with unsaved edits and follows the disk for the rest",
    "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;
    EditorSubject subject = editing.subject();
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Player, subject).unsaved);
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Camera, subject).unsaved);

    editing.gameData.playerData.fallFromHeightThreshold += 100.0f;
    float edited = editing.gameData.playerData.fallFromHeightThreshold;

    GameData onDisk = loadGameData();
    onDisk.cameraData.zoom += 1.0f;
    editorUi.reloaded(editing.gameData, onDisk);

    REQUIRE(editing.gameData.playerData.fallFromHeightThreshold == edited);
    REQUIRE(editing.gameData.cameraData.zoom == onDisk.cameraData.zoom);
    REQUIRE(editorUi.savingIn(EditorSection::Player, subject).unsaved);
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Camera, subject).unsaved);
}

TEST_CASE("Reverting a section kept through a reload takes what is on disk now", "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;
    EditorSubject subject = editing.subject();
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Player, subject).unsaved);

    editing.gameData.playerData.fallFromHeightThreshold += 100.0f;

    GameData onDisk = loadGameData();
    onDisk.playerData.fallFromHeightThreshold += 50.0f;
    editorUi.reloaded(editing.gameData, onDisk);
    editorUi.savingIn(EditorSection::Player, subject).revert();

    REQUIRE(
        editing.gameData.playerData.fallFromHeightThreshold ==
        onDisk.playerData.fallFromHeightThreshold);
    REQUIRE_FALSE(editorUi.savingIn(EditorSection::Player, subject).unsaved);
}

TEST_CASE("Playing back is not a thing that saves", "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing;

    SectionSaving saving = editorUi.savingIn(EditorSection::Playback, editing.subject());

    REQUIRE_FALSE(saving.unsaved);
    REQUIRE(saving.save == nullptr);
}

TEST_CASE("A level that strands an npc says so where its save would be", "[EditorSaving]")
{
    EditorUi editorUi;
    Editing editing({strandedVillager()});

    SectionSaving saving = editorUi.savingIn(EditorSection::Level, editing.subject());

    REQUIRE(saving.cannotBecause.has_value());
    REQUIRE_THAT(*saving.cannotBecause, Catch::Matchers::ContainsSubstring("cannot get back"));
}
