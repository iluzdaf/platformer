#include <catch2/catch_test_macros.hpp>
#include <string>
#include <optional>
#include <tuple>
#include <vector>
#include "game/level.hpp"
#include "game/level_data.hpp"
#include "npc/npc_spawn_data.hpp"
#include "pickups/pickup_spawn_data.hpp"
#include "player/player_data.hpp"
#include "tile_map/tile_map.hpp"
#include "ui/armed.hpp"
#include "ui/editor_commands.hpp"
#include "tile_map/tile_map_data.hpp"
#include "ui/editor_history.hpp"
#include "ui/editor_section.hpp"
#include "ui/level_ui.hpp"
#include "ui/mouse_on_the_map.hpp"
#include <imgui_internal.h>
#include "animations/animator_data.hpp"
#include "actor/observed.hpp"
#include "actor/actor_state.hpp"
#include "helpers/headless_imgui.hpp"
#include "helpers/palettes.hpp"
#include "helpers/shipped.hpp"
#include "helpers/tile_positions.hpp"
#include "helpers/levels.hpp"
#include "helpers/floor_level.hpp"
#include "game/level_resizing.hpp"
#include "physics/aabb.hpp"
#include "physics/physics_body.hpp"
#include "npc/npc.hpp"
#include "ui/actors_in_level.hpp"

namespace
{
    const std::string LevelPath = "levels/being_edited.json";

    constexpr int PaintedTile = 1;

    LevelData dataPlacing(
        const std::vector<NpcSpawnData> &npcs,
        const std::vector<PickupSpawnData> &pickups = {})
    {
        LevelData levelData = aFloorLevelPlacing(npcs, PaintedTile);
        levelData.pickups = pickups;

        return levelData;
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
        std::optional<TileMapData> painted;

        explicit Editing(
            const std::vector<NpcSpawnData> &npcs = {},
            const std::vector<PickupSpawnData> &pickups = {})
            : levelData(dataPlacing(npcs, pickups)), level(levelOf(levelData))
        {

            std::ignore =
                commands.onLevelEdited.connect([this](const LevelData &now) { edited = now; });
            std::ignore =
                commands.onTilesChanged.connect([this](const TileMapData &now) { painted = now; });
        }

        const LevelData &asked()
        {
            commands.drain();
            REQUIRE(edited);
            return *edited;
        }

        const TileMapData &askedForTiles()
        {
            commands.drain();
            REQUIRE(painted);
            return *painted;
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

    MouseOnTheMap clickingAt(glm::vec2 worldPosition)
    {
        MouseOnTheMap mouse;
        mouse.worldPosition = worldPosition;
        mouse.heldDown = true;
        mouse.justClicked = true;

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
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing;
    std::optional<Armed> armed = PaintTile{PaintedTile};
    glm::ivec2 target(3, 2);

    levelUi.update(
        holding(editing.level, target),
        editing.level,
        editing.levelData,
        LevelPath,
        AABB{},
        shippedNpcData(),
        shippedPickupData(),
        armed,
        editing.commands);

    REQUIRE(editing.askedForTiles().indices[target.y][target.x] == PaintedTile);
    REQUIRE(armed == std::optional<Armed>(PaintTile{PaintedTile}));
}

TEST_CASE("Painting waits for the button", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing;
    std::optional<Armed> armed = PaintTile{PaintedTile};
    glm::ivec2 target(3, 2);

    levelUi.update(
        over(editing.level, target),
        editing.level,
        editing.levelData,
        LevelPath,
        AABB{},
        shippedNpcData(),
        shippedPickupData(),
        armed,
        editing.commands);

    editing.commands.drain();
    REQUIRE_FALSE(editing.painted);
}

TEST_CASE("A click that belongs to the panel does not reach the map", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing;
    std::optional<Armed> armed = PaintTile{PaintedTile};
    glm::ivec2 target(3, 2);

    MouseOnTheMap mouse = holding(editing.level, target);
    mouse.overTheUi = true;
    levelUi.update(
        mouse,
        editing.level,
        editing.levelData,
        LevelPath,
        AABB{},
        shippedNpcData(),
        shippedPickupData(),
        armed,
        editing.commands);

    editing.commands.drain();
    REQUIRE_FALSE(editing.edited);
}

TEST_CASE("A click outside the map changes nothing", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing;
    std::optional<Armed> armed = PaintTile{PaintedTile};

    MouseOnTheMap mouse;
    mouse.worldPosition = glm::vec2(-40.0f, -40.0f);
    mouse.heldDown = true;
    levelUi.update(
        mouse,
        editing.level,
        editing.levelData,
        LevelPath,
        AABB{},
        shippedNpcData(),
        shippedPickupData(),
        armed,
        editing.commands);

    editing.commands.drain();
    REQUIRE_FALSE(editing.edited);
}

TEST_CASE("Nothing happens when nothing is armed", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing;
    std::optional<Armed> armed;
    glm::ivec2 target(3, 2);

    levelUi.update(
        clicking(editing.level, target),
        editing.level,
        editing.levelData,
        LevelPath,
        AABB{},
        shippedNpcData(),
        shippedPickupData(),
        armed,
        editing.commands);

    editing.commands.drain();
    REQUIRE_FALSE(editing.edited);
}

TEST_CASE("Picking the player start moves it and puts the pick down", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing;
    std::optional<Armed> armed = PickTile{PickTile::For::PlayerStart, 0};
    glm::ivec2 target(4, FloorLevelStanding);

    levelUi.update(
        clicking(editing.level, target),
        editing.level,
        editing.levelData,
        LevelPath,
        AABB{},
        shippedNpcData(),
        shippedPickupData(),
        armed,
        editing.commands);

    REQUIRE(editing.level.getTileMap().tileUnderFeet(editing.asked().playerFeet) == target);
    REQUIRE_FALSE(armed);
}

TEST_CASE("A pick waits for the click rather than the hold", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing;
    std::optional<Armed> armed = PickTile{PickTile::For::PlayerStart, 0};

    levelUi.update(
        holding(editing.level, glm::ivec2(4, FloorLevelStanding)),
        editing.level,
        editing.levelData,
        LevelPath,
        AABB{},
        shippedNpcData(),
        shippedPickupData(),
        armed,
        editing.commands);

    editing.commands.drain();
    REQUIRE_FALSE(editing.edited);
    REQUIRE(armed);
}

TEST_CASE("Picking an npc's spawn moves it and says the npcs changed", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing({aRatAt(glm::ivec2(2, FloorLevelStanding))});
    std::optional<Armed> armed = PickTile{PickTile::For::NpcSpawn, 0};
    glm::ivec2 target(5, FloorLevelStanding);

    levelUi.update(
        clicking(editing.level, target),
        editing.level,
        editing.levelData,
        LevelPath,
        AABB{},
        shippedNpcData(),
        shippedPickupData(),
        armed,
        editing.commands);
    REQUIRE(editing.asked().npcs.front().feet == feetOf(target));
    REQUIRE_FALSE(armed);
}

TEST_CASE("A level told to name another palette asks for the level again", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing;

    levelUi.namesPalette("another", editing.levelData, editing.commands);

    REQUIRE(editing.asked().tileMapData.tilePalette == "another");
    REQUIRE(history.anythingToUndo());
}

TEST_CASE("A level told to name the palette it already names asks for nothing", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing;

    levelUi.namesPalette(
        editing.levelData.tileMapData.tilePalette, editing.levelData, editing.commands);
    editing.commands.drain();

    REQUIRE_FALSE(editing.edited);
    REQUIRE_FALSE(history.anythingToUndo());
}

TEST_CASE("Picking a pickup's spawn moves it and says the level changed", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing({}, {PickupSpawnData{"coin", feetOf(glm::ivec2(2, FloorLevelStanding))}});
    std::optional<Armed> armed = PickTile{PickTile::For::PickupSpawn, 0};
    glm::ivec2 target(5, FloorLevelStanding);

    levelUi.update(
        clicking(editing.level, target),
        editing.level,
        editing.levelData,
        LevelPath,
        AABB{},
        shippedNpcData(),
        shippedPickupData(),
        armed,
        editing.commands);

    REQUIRE(editing.asked().pickups.front().feet == feetOf(target));
    REQUIRE_FALSE(armed);
}

TEST_CASE("A pick naming a pickup the level lost is put down, not acted on", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing;
    std::optional<Armed> armed = PickTile{PickTile::For::PickupSpawn, 0};

    levelUi.update(
        clicking(editing.level, glm::ivec2(5, FloorLevelStanding)),
        editing.level,
        editing.levelData,
        LevelPath,
        AABB{},
        shippedNpcData(),
        shippedPickupData(),
        armed,
        editing.commands);
    editing.commands.drain();

    REQUIRE_FALSE(editing.edited);
    REQUIRE_FALSE(armed);
}

TEST_CASE("Picking one end of a beat leaves the other where it was", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    NpcSpawnData walking = aRatAt(glm::ivec2(2, FloorLevelStanding));
    walking.patrol = beatOf(glm::ivec2(1, FloorLevelStanding), glm::ivec2(8, FloorLevelStanding));
    Editing editing({walking});

    std::optional<Armed> armed = PickTile{PickTile::For::PatrolTo, 0};
    glm::ivec2 target(5, FloorLevelStanding);

    levelUi.update(
        clicking(editing.level, target),
        editing.level,
        editing.levelData,
        LevelPath,
        AABB{},
        shippedNpcData(),
        shippedPickupData(),
        armed,
        editing.commands);

    REQUIRE(editing.asked().npcs.front().patrol->to == beatOf(target, target).to);
    REQUIRE(
        editing.asked().npcs.front().patrol->from ==
        beatOf(glm::ivec2(1, FloorLevelStanding), target).from);
}

TEST_CASE("The first end picked of an absent beat becomes both of them", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing({aRatAt(glm::ivec2(2, FloorLevelStanding))});
    REQUIRE_FALSE(editing.levelData.npcs.front().patrol);

    std::optional<Armed> armed = PickTile{PickTile::For::PatrolFrom, 0};
    glm::ivec2 target(5, FloorLevelStanding);

    levelUi.update(
        clicking(editing.level, target),
        editing.level,
        editing.levelData,
        LevelPath,
        AABB{},
        shippedNpcData(),
        shippedPickupData(),
        armed,
        editing.commands);

    REQUIRE(editing.asked().npcs.front().patrol == beatOf(target, target));
}

TEST_CASE("A pick naming an npc the level lost is put down, not acted on", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing({aRatAt(glm::ivec2(2, FloorLevelStanding))});
    std::optional<Armed> armed = PickTile{PickTile::For::NpcSpawn, 4};

    REQUIRE_NOTHROW(levelUi.update(
        clicking(editing.level, glm::ivec2(5, FloorLevelStanding)),
        editing.level,
        editing.levelData,
        LevelPath,
        AABB{},
        shippedNpcData(),
        shippedPickupData(),
        armed,
        editing.commands));

    editing.commands.drain();
    REQUIRE_FALSE(editing.edited);
    REQUIRE_FALSE(armed);
}

TEST_CASE("A click places what is armed where it lands", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing;
    std::optional<Armed> armed = PlaceOne{PlaceOne::What::Pickup, "coin"};
    glm::ivec2 target(5, FloorLevelStanding);

    levelUi.update(
        clicking(editing.level, target),
        editing.level,
        editing.levelData,
        LevelPath,
        AABB{},
        shippedNpcData(),
        shippedPickupData(),
        armed,
        editing.commands);

    const LevelData &asked = editing.asked();
    REQUIRE(asked.pickups.size() == 1);
    REQUIRE(asked.pickups.front().type == "coin");
    REQUIRE(asked.pickups.front().feet == feetOf(target));
    REQUIRE(levelUi.shown() == ActorShown{ActorShown::What::Pickup, 0});
}

TEST_CASE("What is placed stays armed, so another can be placed", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing;
    std::optional<Armed> armed = PlaceOne{PlaceOne::What::Pickup, "coin"};

    levelUi.update(
        clicking(editing.level, glm::ivec2(5, FloorLevelStanding)),
        editing.level,
        editing.levelData,
        LevelPath,
        AABB{},
        shippedNpcData(),
        shippedPickupData(),
        armed,
        editing.commands);

    REQUIRE(armed == std::optional<Armed>(PlaceOne{PlaceOne::What::Pickup, "coin"}));
}

TEST_CASE("A creature placed is given the run it stands on", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing;
    const std::string &kind = shippedNpcData().begin()->first;
    std::optional<Armed> armed = PlaceOne{PlaceOne::What::Npc, kind};

    levelUi.update(
        clicking(editing.level, glm::ivec2(5, FloorLevelStanding)),
        editing.level,
        editing.levelData,
        LevelPath,
        AABB{},
        shippedNpcData(),
        shippedPickupData(),
        armed,
        editing.commands);

    const LevelData &asked = editing.asked();
    REQUIRE(asked.npcs.size() == 1);
    REQUIRE(asked.npcs.front().type == kind);
    REQUIRE(asked.npcs.front().patrol);
}

TEST_CASE("A type the cast no longer has puts the arm down", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing;
    std::optional<Armed> armed = PlaceOne{PlaceOne::What::Npc, "nobody"};

    levelUi.update(
        clicking(editing.level, glm::ivec2(5, FloorLevelStanding)),
        editing.level,
        editing.levelData,
        LevelPath,
        AABB{},
        shippedNpcData(),
        shippedPickupData(),
        armed,
        editing.commands);
    editing.commands.drain();

    REQUIRE_FALSE(editing.edited);
    REQUIRE_FALSE(armed);
}

TEST_CASE("A click with nothing armed shows what is under it", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing({aRatAt(glm::ivec2(2, FloorLevelStanding))});
    std::optional<Armed> armed;

    levelUi.update(
        clickingAt(editing.level.getNpcs().front()->body().aabb().center()),
        editing.level,
        editing.levelData,
        LevelPath,
        AABB{},
        shippedNpcData(),
        shippedPickupData(),
        armed,
        editing.commands);

    REQUIRE(levelUi.shown() == ActorShown{ActorShown::What::Npc, 0});
}

TEST_CASE("A click where nothing stands shows nobody", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing({aRatAt(glm::ivec2(2, FloorLevelStanding))});
    std::optional<Armed> armed;
    const Level &level = editing.level;

    levelUi.update(
        clickingAt(level.getNpcs().front()->body().aabb().center()),
        level,
        editing.levelData,
        LevelPath,
        AABB{},
        shippedNpcData(),
        shippedPickupData(),
        armed,
        editing.commands);
    REQUIRE(levelUi.shown() == ActorShown{ActorShown::What::Npc, 0});

    levelUi.update(
        clickingAt(level.getTileMap().feetOnTile(glm::ivec2(8, 1))),
        level,
        editing.levelData,
        LevelPath,
        AABB{},
        shippedNpcData(),
        shippedPickupData(),
        armed,
        editing.commands);

    REQUIRE(levelUi.shown() == ActorShown{});
}

TEST_CASE("A click while something is armed places it rather than showing", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing({aRatAt(glm::ivec2(2, FloorLevelStanding))});
    std::optional<Armed> armed = PickTile{PickTile::For::NpcSpawn, 0};

    levelUi.update(
        clickingAt(editing.level.getNpcs().front()->body().aabb().center()),
        editing.level,
        editing.levelData,
        LevelPath,
        AABB{},
        shippedNpcData(),
        shippedPickupData(),
        armed,
        editing.commands);

    REQUIRE(levelUi.shown() == ActorShown{});
    REQUIRE_FALSE(armed);
}

TEST_CASE("Delete takes away what is shown", "[LevelUi]")
{
    HeadlessImGui gui;
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing(
        {aRatAt(glm::ivec2(2, FloorLevelStanding)), aRatAt(glm::ivec2(5, FloorLevelStanding))});
    std::optional<Armed> armed;
    AnimatorData animations;
    Observed observed;
    ActorState playerState;

    levelUi.update(
        clickingAt(editing.level.getNpcs().front()->body().aabb().center()),
        editing.level,
        editing.levelData,
        LevelPath,
        AABB{},
        shippedNpcData(),
        shippedPickupData(),
        armed,
        editing.commands);
    REQUIRE(levelUi.shown() == ActorShown{ActorShown::What::Npc, 0});

    ImGui::GetIO().AddKeyEvent(ImGuiKey_Delete, true);
    gui.frame(
        [&]
        {
            levelUi.draw(
                editing.level,
                editing.levelData,
                animations,
                observed,
                editing.levelData.playerFeet,
                playerState,
                shippedNpcData(),
                armed,
                editing.commands);
        });
    ImGui::GetIO().AddKeyEvent(ImGuiKey_Delete, false);

    REQUIRE(editing.asked().npcs.size() == 1);
    REQUIRE(levelUi.shown() == ActorShown{});
}

TEST_CASE("The level section draws without a tile sheet", "[LevelUi]")
{
    HeadlessImGui gui;
    EditorHistory history;
    LevelUi levelUi{history};
    LevelData levelData = dataPlacing({aRatAt(glm::ivec2(3, FloorLevelStanding))});
    Level level = levelOf(levelData);
    AnimatorData animations;
    Observed observed;
    ActorState playerState;
    std::optional<Armed> armed;
    EditorCommands commands;

    REQUIRE_NOTHROW(gui.frame(
        [&]
        {
            levelUi.draw(
                level,
                levelData,
                animations,
                observed,
                level.getTileMap().feetOnTile(glm::ivec2(1, FloorLevelStanding)),
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

    REQUIRE(asked.level.tileMapData.indices[0].size() == FloorLevelTiles + 1);
    REQUIRE(asked.shift == glm::vec2(16.0f, 0.0f));
}

TEST_CASE(
    "A column off the left is asked for with the shift that moves the player back",
    "[LevelUi]")
{
    Resized asked = resizedBy(Resize{Side::Left, false});

    REQUIRE(asked.level.tileMapData.indices[0].size() == FloorLevelTiles - 1);
    REQUIRE(asked.shift == glm::vec2(-16.0f, 0.0f));
}

TEST_CASE("A row below is asked for with no shift", "[LevelUi]")
{
    Resized asked = resizedBy(Resize{Side::Below, true});

    REQUIRE(asked.level.tileMapData.indices.size() == FloorLevelTiles + 1);
    REQUIRE(asked.shift == glm::vec2(0.0f));
}

namespace
{
    void paints(
        LevelUi &levelUi,
        Editing &editing,
        const MouseOnTheMap &mouse,
        std::optional<Armed> &armed)
    {
        levelUi.update(
            mouse,
            editing.level,
            editing.levelData,
            LevelPath,
            AABB{},
            shippedNpcData(),
            shippedPickupData(),
            armed,
            editing.commands);
    }

    MouseOnTheMap lettingGo()
    {
        return MouseOnTheMap();
    }
}

TEST_CASE("The level as it was before the paint is remembered", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing;
    std::optional<Armed> armed = PaintTile{PaintedTile};
    glm::ivec2 target(3, 2);
    int wasThere = editing.levelData.tileMapData.indices[target.y][target.x];

    paints(levelUi, editing, holding(editing.level, target), armed);
    REQUIRE(editing.askedForTiles().indices[target.y][target.x] == PaintedTile);

    std::optional<EditorStep> back = history.stepBack();
    REQUIRE(back);
    REQUIRE(back->section == EditorSection::Level);
    REQUIRE(back->levelData->tileMapData.indices[target.y][target.x] == wasThere);
}

TEST_CASE("A stroke of paint is one step back, not one for each tile", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing;
    std::optional<Armed> armed = PaintTile{PaintedTile};

    paints(levelUi, editing, holding(editing.level, glm::ivec2(3, 2)), armed);
    paints(levelUi, editing, holding(editing.level, glm::ivec2(4, 2)), armed);
    paints(levelUi, editing, lettingGo(), armed);
    paints(levelUi, editing, holding(editing.level, glm::ivec2(5, 2)), armed);

    REQUIRE(history.stepBack());
    REQUIRE(history.stepBack());
    REQUIRE_FALSE(history.stepBack());
}

TEST_CASE("The player start is remembered where it stood", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing;
    std::optional<Armed> armed = PickTile{PickTile::For::PlayerStart, 0};
    glm::vec2 stood = editing.levelData.playerFeet;

    paints(levelUi, editing, clicking(editing.level, glm::ivec2(4, FloorLevelStanding)), armed);
    REQUIRE(editing.asked().playerFeet != stood);

    REQUIRE(history.stepBack()->levelData->playerFeet == stood);
}

TEST_CASE("A resize is remembered with the shift that moves the player back", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing;
    std::optional<Resized> asked;
    std::ignore = editing.commands.onLevelResized.connect(
        [&](const LevelData &level, const glm::vec2 &shift) { asked = Resized{level, shift}; });

    levelUi.resizes(Resize{Side::Left, true}, editing.levelData, 16, editing.commands);
    editing.commands.drain();
    REQUIRE(asked->level.tileMapData.indices[0].size() == FloorLevelTiles + 1);

    std::optional<EditorStep> back = history.stepBack();
    REQUIRE(back);
    REQUIRE(back->levelData->tileMapData.indices[0].size() == FloorLevelTiles);
    REQUIRE(back->movingThePlayerBack == glm::vec2(-16.0f, 0.0f));
}

TEST_CASE("A level nobody has edited leaves the history empty", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing;
    std::optional<Armed> armed;

    paints(levelUi, editing, clicking(editing.level, glm::ivec2(4, FloorLevelStanding)), armed);

    REQUIRE_FALSE(history.anythingToUndo());
}

TEST_CASE("Moving to another level forgets what the last one was", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing;
    std::optional<Armed> armed = PaintTile{PaintedTile};

    paints(levelUi, editing, holding(editing.level, glm::ivec2(3, 2)), armed);
    REQUIRE(history.anythingToUndo());

    levelUi.update(
        lettingGo(),
        editing.level,
        editing.levelData,
        "levels/somewhere_else.json",
        AABB{},
        shippedNpcData(),
        shippedPickupData(),
        armed,
        editing.commands);

    REQUIRE_FALSE(history.anythingToUndo());
}

TEST_CASE("A section told to forget has nothing to undo", "[LevelUi]")
{
    EditorHistory history;
    LevelUi levelUi{history};
    Editing editing;
    std::optional<Armed> armed = PaintTile{PaintedTile};

    paints(levelUi, editing, holding(editing.level, glm::ivec2(3, 2)), armed);
    REQUIRE(history.anythingToUndo());

    levelUi.forgets();

    REQUIRE_FALSE(history.anythingToUndo());
}
