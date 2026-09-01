#include <catch2/catch_test_macros.hpp>
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
#include "test_helpers/test_tile_map_utils.hpp"

namespace
{
    constexpr int MapTiles = 10;
    constexpr int FloorRow = 6;
    constexpr int Standing = FloorRow - 1;
    constexpr int PaintedTile = 1;

    Level levelPlacing(const std::vector<NpcSpawnData> &npcs)
    {
        LevelData levelData;
        levelData.tileMapData.size = 16;
        levelData.tileMapData.indices =
            std::vector<std::vector<int>>(MapTiles, std::vector<int>(MapTiles, 0));
        for (int x = 0; x < MapTiles; ++x)
            (*levelData.tileMapData.indices)[FloorRow][x] = PaintedTile;

        levelData.playerStartTilePosition = glm::ivec2(1, Standing);
        levelData.npcs = npcs;

        return Level(
            levelData, palettesFrom(getDefaultTileDataMap()), PlayerData(), shippedNpcData());
    }

    NpcSpawnData villagerAt(glm::ivec2 tilePosition)
    {
        NpcSpawnData spawn;
        spawn.type = "villager";
        spawn.tilePosition = tilePosition;
        return spawn;
    }

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
    EditorCommands commands;
    Level level = levelPlacing({});
    std::optional<Armed> armed = PaintTile{PaintedTile};
    glm::ivec2 target(3, 2);

    levelUi.update(holding(level, target), level, armed, commands);

    REQUIRE(level.getTileMap().tilePositionToTileIndex(target) == PaintedTile);
    REQUIRE(armed == std::optional<Armed>(PaintTile{PaintedTile}));
}

TEST_CASE("Painting waits for the button", "[LevelUi]")
{
    LevelUi levelUi;
    EditorCommands commands;
    Level level = levelPlacing({});
    std::optional<Armed> armed = PaintTile{PaintedTile};
    glm::ivec2 target(3, 2);

    levelUi.update(over(level, target), level, armed, commands);

    REQUIRE(level.getTileMap().tilePositionToTileIndex(target) == 0);
}

TEST_CASE("A click that belongs to the panel does not reach the map", "[LevelUi]")
{
    LevelUi levelUi;
    EditorCommands commands;
    Level level = levelPlacing({});
    std::optional<Armed> armed = PaintTile{PaintedTile};
    glm::ivec2 target(3, 2);

    MouseOnTheMap mouse = holding(level, target);
    mouse.overTheUi = true;
    levelUi.update(mouse, level, armed, commands);

    REQUIRE(level.getTileMap().tilePositionToTileIndex(target) == 0);
}

TEST_CASE("A click outside the map changes nothing", "[LevelUi]")
{
    LevelUi levelUi;
    EditorCommands commands;
    Level level = levelPlacing({});
    std::optional<Armed> armed = PaintTile{PaintedTile};

    MouseOnTheMap mouse;
    mouse.worldPosition = glm::vec2(-40.0f, -40.0f);
    mouse.heldDown = true;
    levelUi.update(mouse, level, armed, commands);

    REQUIRE(level.getTileMap().tilePositionToTileIndex(glm::ivec2(0, 0)) == 0);
}

TEST_CASE("Nothing happens when nothing is armed", "[LevelUi]")
{
    LevelUi levelUi;
    EditorCommands commands;
    Level level = levelPlacing({});
    std::optional<Armed> armed;
    glm::ivec2 target(3, 2);

    levelUi.update(clicking(level, target), level, armed, commands);

    REQUIRE(level.getTileMap().tilePositionToTileIndex(target) == 0);
}

TEST_CASE("Picking the player start moves it and puts the pick down", "[LevelUi]")
{
    LevelUi levelUi;
    EditorCommands commands;
    Level level = levelPlacing({});
    std::optional<Armed> armed = PickTile{PickTile::For::PlayerStart, 0};
    glm::ivec2 target(4, Standing);

    levelUi.update(clicking(level, target), level, armed, commands);

    REQUIRE(level.getPlayerStartTile() == target);
    REQUIRE_FALSE(armed);
}

TEST_CASE("A pick waits for the click rather than the hold", "[LevelUi]")
{
    LevelUi levelUi;
    EditorCommands commands;
    Level level = levelPlacing({});
    glm::ivec2 before = level.getPlayerStartTile();
    std::optional<Armed> armed = PickTile{PickTile::For::PlayerStart, 0};

    levelUi.update(holding(level, glm::ivec2(4, Standing)), level, armed, commands);

    REQUIRE(level.getPlayerStartTile() == before);
    REQUIRE(armed);
}

TEST_CASE("Picking an npc's spawn moves it and says the npcs changed", "[LevelUi]")
{
    LevelUi levelUi;
    EditorCommands commands;
    Level level = levelPlacing({villagerAt(glm::ivec2(2, Standing))});
    std::optional<Armed> armed = PickTile{PickTile::For::NpcSpawn, 0};
    glm::ivec2 target(5, Standing);

    bool asked = false;
    std::ignore = commands.onNpcsChanged.connect([&asked] { asked = true; });

    levelUi.update(clicking(level, target), level, armed, commands);
    commands.drain();

    REQUIRE(level.getNpcs().front().tilePosition == target);
    REQUIRE(asked);
    REQUIRE_FALSE(armed);
}

TEST_CASE("Picking one end of a beat leaves the other where it was", "[LevelUi]")
{
    LevelUi levelUi;
    EditorCommands commands;
    Level level = levelPlacing({villagerAt(glm::ivec2(2, Standing))});
    level.setNpcPatrol(0, PatrolData{glm::ivec2(1, Standing), glm::ivec2(8, Standing)});

    std::optional<Armed> armed = PickTile{PickTile::For::PatrolTo, 0};
    glm::ivec2 target(5, Standing);

    levelUi.update(clicking(level, target), level, armed, commands);

    REQUIRE(level.getNpcs().front().patrol->to == target);
    REQUIRE(level.getNpcs().front().patrol->from == glm::ivec2(1, Standing));
}

TEST_CASE("The first end picked of an absent beat becomes both of them", "[LevelUi]")
{
    LevelUi levelUi;
    EditorCommands commands;
    Level level = levelPlacing({villagerAt(glm::ivec2(2, Standing))});
    REQUIRE_FALSE(level.getNpcs().front().patrol);

    std::optional<Armed> armed = PickTile{PickTile::For::PatrolFrom, 0};
    glm::ivec2 target(5, Standing);

    levelUi.update(clicking(level, target), level, armed, commands);

    REQUIRE(level.getNpcs().front().patrol == PatrolData{target, target});
}

TEST_CASE("A pick naming an npc the level lost is put down, not acted on", "[LevelUi]")
{
    LevelUi levelUi;
    EditorCommands commands;
    Level level = levelPlacing({villagerAt(glm::ivec2(2, Standing))});
    std::optional<Armed> armed = PickTile{PickTile::For::NpcSpawn, 4};

    REQUIRE_NOTHROW(
        levelUi.update(clicking(level, glm::ivec2(5, Standing)), level, armed, commands));

    REQUIRE(level.getNpcs().front().tilePosition == glm::ivec2(2, Standing));
    REQUIRE_FALSE(armed);
}

TEST_CASE("Arming shows the grid and disarming puts it back", "[LevelUi]")
{
    LevelUi levelUi;
    EditorCommands commands;
    Level level = levelPlacing({});
    std::optional<Armed> armed;

    levelUi.update(over(level, glm::ivec2(3, 2)), level, armed, commands);
    REQUIRE_FALSE(levelUi.showingGrid());

    armed = PaintTile{PaintedTile};
    levelUi.update(over(level, glm::ivec2(3, 2)), level, armed, commands);
    REQUIRE(levelUi.showingGrid());

    armed.reset();
    levelUi.update(over(level, glm::ivec2(3, 2)), level, armed, commands);
    REQUIRE_FALSE(levelUi.showingGrid());
}

TEST_CASE("The grid follows arming even with the mouse over the panel", "[LevelUi]")
{
    LevelUi levelUi;
    EditorCommands commands;
    Level level = levelPlacing({});
    std::optional<Armed> armed = PickTile{PickTile::For::PlayerStart, 0};

    MouseOnTheMap onThePanel = over(level, glm::ivec2(3, 2));
    onThePanel.overTheUi = true;

    levelUi.update(onThePanel, level, armed, commands);

    REQUIRE(levelUi.showingGrid());
}

TEST_CASE("A pick that places itself leaves the grid as it found it", "[LevelUi]")
{
    LevelUi levelUi;
    EditorCommands commands;
    Level level = levelPlacing({});
    std::optional<Armed> armed = PickTile{PickTile::For::PlayerStart, 0};

    levelUi.update(clicking(level, glm::ivec2(3, 2)), level, armed, commands);
    REQUIRE_FALSE(armed);

    levelUi.update(over(level, glm::ivec2(3, 2)), level, armed, commands);

    REQUIRE_FALSE(levelUi.showingGrid());
}
