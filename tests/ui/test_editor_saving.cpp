#include "test_helpers/test_tile_map_utils.hpp"
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
#include "game/levels.hpp"
#include "npc/npc_spawn_data.hpp"
#include "rendering/texture_cache.hpp"
#include "test_helpers/asset_path.hpp"
#include "ui/editor_section.hpp"
#include "ui/editor_ui.hpp"

namespace
{
    struct Editing
    {
        GameData gameData = loadGameData();
        Levels levels{assetPath("levels.json")};
        std::string levelPath = assetPath("levels/level6.json");
        Level level{
            readLevelData(levelPath),
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
    Editing editing;

    editing.level.addNpc(
        NpcSpawnData{"villager", glm::ivec2(2, 8), std::nullopt}, editing.gameData.npcData);
    std::size_t placed = spawnsIn(editing.level).size() - 1;
    editing.level.setNpcPatrol(placed, PatrolData{glm::ivec2(2, 8), glm::ivec2(2, 1)});

    SectionSaving saving = editorUi.savingIn(EditorSection::Level, editing.subject());

    REQUIRE(saving.cannotBecause.has_value());
    REQUIRE_THAT(*saving.cannotBecause, Catch::Matchers::ContainsSubstring("cannot get back"));
}
