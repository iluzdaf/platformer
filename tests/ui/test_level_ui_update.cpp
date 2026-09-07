#include <catch2/catch_test_macros.hpp>
#include <string>
#include <optional>
#include <tuple>
#include <vector>
#include "game/level.hpp"
#include "game/level_data.hpp"
#include "npc/npc_spawn_data.hpp"
#include "player/player_data.hpp"
#include "tile_map/tile_map.hpp"
#include "ui/armed.hpp"
#include "ui/editor_commands.hpp"
#include "ui/level_ui.hpp"
#include "ui/mouse_on_the_map.hpp"
#include <imgui_internal.h>
#include "actor/actor_motion_state.hpp"
#include "actor/actor_state.hpp"
#include "helpers/headless_imgui.hpp"
#include "helpers/palettes.hpp"
#include "helpers/shipped.hpp"
#include "helpers/tiles.hpp"
#include "helpers/levels.hpp"
#include "game/level_resizing.hpp"

namespace
{
    const std::string LevelPath = "levels/being_edited.json";

    constexpr int MapTiles = 10;
    constexpr int FloorRow = 6;
    constexpr int Standing = FloorRow - 1;
    constexpr int PaintedTile = 1;

    LevelData dataPlacing(const std::vector<NpcSpawnData> &npcs)
    {
        return aFloorLevelPlacing(npcs, PaintedTile);
    }

    Level levelOf(const LevelData &levelData)
    {
        return Level(
            levelData,
            theOnlyPalette(aPaletteWithASolidTile()),
            PlayerData(),
            shippedNpcData(),
            shippedPickupData());
    }

    struct Editing
    {
        LevelData levelData;
        Level level;
        EditorCommands commands;
        std::optional<LevelData> edited;

        explicit Editing(const std::vector<NpcSpawnData> &npcs = {})
            : levelData(dataPlacing(npcs)), level(levelOf(levelData))
        {
            std::ignore =
                commands.onLevelEdited.connect([this](const LevelData &now) { edited = now; });
        }

        const LevelData &asked()
        {
            commands.drain();
            REQUIRE(edited);
            return *edited;
        }
    };

    MouseOnTheMap over(const Level &level, glm::ivec2 tilePosition)
    {
        MouseOnTheMap mouse;
        mouse.worldPosition = level.getTileMap().feetOnTile(tilePosition) - glm::vec2(0.0f, 1.0f);
        return mouse;
    }

    MouseOnTheMap holding(const Level &level, glm::ivec2 tilePosition)
    {
        MouseOnTheMap mouse = over(level, tilePosition);
        mouse.heldDown = true;
        return mouse;
    }

    MouseOnTheMap clicking(const Level &level, glm::ivec2 tilePosition)
    {
        MouseOnTheMap mouse = holding(level, tilePosition);
        mouse.justClicked = true;
        return mouse;
    }
}

TEST_CASE("Painting sets the tile under the mouse while the button is down", "[LevelUi]")
{
    LevelUi levelUi;
    Editing editing;
    std::optional<Armed> armed = PaintTile{PaintedTile};
    glm::ivec2 target(3, 2);

    levelUi.update(
        holding(editing.level, target),
        editing.level,
        editing.levelData,
        LevelPath,
        armed,
        editing.commands);

    REQUIRE(editing.asked().tileMapData.indices[target.y][target.x] == PaintedTile);
    REQUIRE(armed == std::optional<Armed>(PaintTile{PaintedTile}));
}

TEST_CASE("Painting waits for the button", "[LevelUi]")
{
    LevelUi levelUi;
    Editing editing;
    std::optional<Armed> armed = PaintTile{PaintedTile};
    glm::ivec2 target(3, 2);

    levelUi.update(
        over(editing.level, target),
        editing.level,
        editing.levelData,
        LevelPath,
        armed,
        editing.commands);

    editing.commands.drain();
    REQUIRE_FALSE(editing.edited);
}

TEST_CASE("A click that belongs to the panel does not reach the map", "[LevelUi]")
{
    LevelUi levelUi;
    Editing editing;
    std::optional<Armed> armed = PaintTile{PaintedTile};
    glm::ivec2 target(3, 2);

    MouseOnTheMap mouse = holding(editing.level, target);
    mouse.overTheUi = true;
    levelUi.update(mouse, editing.level, editing.levelData, LevelPath, armed, editing.commands);

    editing.commands.drain();
    REQUIRE_FALSE(editing.edited);
}

TEST_CASE("A click outside the map changes nothing", "[LevelUi]")
{
    LevelUi levelUi;
    Editing editing;
    std::optional<Armed> armed = PaintTile{PaintedTile};

    MouseOnTheMap mouse;
    mouse.worldPosition = glm::vec2(-40.0f, -40.0f);
    mouse.heldDown = true;
    levelUi.update(mouse, editing.level, editing.levelData, LevelPath, armed, editing.commands);

    editing.commands.drain();
    REQUIRE_FALSE(editing.edited);
}

TEST_CASE("Nothing happens when nothing is armed", "[LevelUi]")
{
    LevelUi levelUi;
    Editing editing;
    std::optional<Armed> armed;
    glm::ivec2 target(3, 2);

    levelUi.update(
        clicking(editing.level, target),
        editing.level,
        editing.levelData,
        LevelPath,
        armed,
        editing.commands);

    editing.commands.drain();
    REQUIRE_FALSE(editing.edited);
}

TEST_CASE("Picking the player start moves it and puts the pick down", "[LevelUi]")
{
    LevelUi levelUi;
    Editing editing;
    std::optional<Armed> armed = PickTile{PickTile::For::PlayerStart, 0};
    glm::ivec2 target(4, Standing);

    levelUi.update(
        clicking(editing.level, target),
        editing.level,
        editing.levelData,
        LevelPath,
        armed,
        editing.commands);

    REQUIRE(editing.level.getTileMap().tileUnderFeet(editing.asked().playerFeet) == target);
    REQUIRE_FALSE(armed);
}

TEST_CASE("A pick waits for the click rather than the hold", "[LevelUi]")
{
    LevelUi levelUi;
    Editing editing;
    std::optional<Armed> armed = PickTile{PickTile::For::PlayerStart, 0};

    levelUi.update(
        holding(editing.level, glm::ivec2(4, Standing)),
        editing.level,
        editing.levelData,
        LevelPath,
        armed,
        editing.commands);

    editing.commands.drain();
    REQUIRE_FALSE(editing.edited);
    REQUIRE(armed);
}

TEST_CASE("Picking an npc's spawn moves it and says the npcs changed", "[LevelUi]")
{
    LevelUi levelUi;
    Editing editing({aVillagerAt(glm::ivec2(2, Standing))});
    std::optional<Armed> armed = PickTile{PickTile::For::NpcSpawn, 0};
    glm::ivec2 target(5, Standing);

    levelUi.update(
        clicking(editing.level, target),
        editing.level,
        editing.levelData,
        LevelPath,
        armed,
        editing.commands);
    REQUIRE(editing.asked().npcs.front().feet == feetOf(target));
    REQUIRE_FALSE(armed);
}

TEST_CASE("Picking one end of a beat leaves the other where it was", "[LevelUi]")
{
    LevelUi levelUi;
    NpcSpawnData walking = aVillagerAt(glm::ivec2(2, Standing));
    walking.patrol = beatOf(glm::ivec2(1, Standing), glm::ivec2(8, Standing));
    Editing editing({walking});

    std::optional<Armed> armed = PickTile{PickTile::For::PatrolTo, 0};
    glm::ivec2 target(5, Standing);

    levelUi.update(
        clicking(editing.level, target),
        editing.level,
        editing.levelData,
        LevelPath,
        armed,
        editing.commands);

    REQUIRE(editing.asked().npcs.front().patrol->to == beatOf(target, target).to);
    REQUIRE(
        editing.asked().npcs.front().patrol->from == beatOf(glm::ivec2(1, Standing), target).from);
}

TEST_CASE("The first end picked of an absent beat becomes both of them", "[LevelUi]")
{
    LevelUi levelUi;
    Editing editing({aVillagerAt(glm::ivec2(2, Standing))});
    REQUIRE_FALSE(editing.levelData.npcs.front().patrol);

    std::optional<Armed> armed = PickTile{PickTile::For::PatrolFrom, 0};
    glm::ivec2 target(5, Standing);

    levelUi.update(
        clicking(editing.level, target),
        editing.level,
        editing.levelData,
        LevelPath,
        armed,
        editing.commands);

    REQUIRE(editing.asked().npcs.front().patrol == beatOf(target, target));
}

TEST_CASE("A pick naming an npc the level lost is put down, not acted on", "[LevelUi]")
{
    LevelUi levelUi;
    Editing editing({aVillagerAt(glm::ivec2(2, Standing))});
    std::optional<Armed> armed = PickTile{PickTile::For::NpcSpawn, 4};

    REQUIRE_NOTHROW(levelUi.update(
        clicking(editing.level, glm::ivec2(5, Standing)),
        editing.level,
        editing.levelData,
        LevelPath,
        armed,
        editing.commands));

    editing.commands.drain();
    REQUIRE_FALSE(editing.edited);
    REQUIRE_FALSE(armed);
}

TEST_CASE("The level section draws without a tile sheet", "[LevelUi]")
{
    HeadlessImGui gui;
    LevelUi levelUi;
    LevelData levelData = dataPlacing({aVillagerAt(glm::ivec2(3, Standing))});
    Level level = levelOf(levelData);
    ActorMotionState motion;
    ActorState playerState;
    std::optional<Armed> armed;
    EditorCommands commands;

    REQUIRE_NOTHROW(gui.frame(
        [&]
        {
            ImGui::TreeNodeSetOpen(ImGui::GetID("Actors"), true);
            levelUi.draw(
                level,
                levelData,
                LevelPath,
                motion,
                level.getTileMap().feetOnTile(glm::ivec2(1, Standing)),
                playerState,
                shippedNpcData(),
                armed,
                commands);
        }));
}

namespace
{
    struct Resized
    {
        LevelData level;
        glm::vec2 shift;
    };

    Resized resizedBy(Resize resize)
    {
        Editing editing;
        std::optional<Resized> asked;
        std::ignore = editing.commands.onLevelResized.connect(
            [&](const LevelData &level, const glm::vec2 &shift) { asked = Resized{level, shift}; });

        askedToResize(resize, editing.levelData, 16, editing.commands);
        editing.commands.drain();

        REQUIRE(asked);
        return *asked;
    }
}

TEST_CASE("A column on the left is asked for with the shift that moves the player", "[LevelUi]")
{
    Resized asked = resizedBy(Resize{Side::Left, true});

    REQUIRE(asked.level.tileMapData.indices[0].size() == MapTiles + 1);
    REQUIRE(asked.shift == glm::vec2(16.0f, 0.0f));
}

TEST_CASE(
    "A column off the left is asked for with the shift that moves the player back",
    "[LevelUi]")
{
    Resized asked = resizedBy(Resize{Side::Left, false});

    REQUIRE(asked.level.tileMapData.indices[0].size() == MapTiles - 1);
    REQUIRE(asked.shift == glm::vec2(-16.0f, 0.0f));
}

TEST_CASE("A row below is asked for with no shift", "[LevelUi]")
{
    Resized asked = resizedBy(Resize{Side::Below, true});

    REQUIRE(asked.level.tileMapData.indices.size() == MapTiles + 1);
    REQUIRE(asked.shift == glm::vec2(0.0f));
}
