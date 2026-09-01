#include <catch2/catch_test_macros.hpp>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "actor/actor_motion_state.hpp"
#include "actor/actor_state.hpp"
#include "game/level.hpp"
#include "game/level_data.hpp"
#include "npc/npc.hpp"
#include "npc/npc_spawn_data.hpp"
#include "player/player_data.hpp"
#include "ui/actors_in_level.hpp"
#include "ui/armed.hpp"
#include "test_helpers/headless_imgui.hpp"
#include "test_helpers/test_tile_map_utils.hpp"

namespace
{
    constexpr int MapTiles = 10;
    constexpr int FloorRow = 6;
    constexpr int Standing = FloorRow - 1;

    Level levelPlacing(const std::vector<NpcSpawnData> &npcs)
    {
        LevelData levelData;
        levelData.tileMapData.size = 16;
        levelData.tileMapData.indices =
            std::vector<std::vector<int>>(MapTiles, std::vector<int>(MapTiles, 0));
        for (int x = 0; x < MapTiles; ++x)
            (*levelData.tileMapData.indices)[FloorRow][x] = 1;

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

    std::vector<std::unique_ptr<Npc>> liveNpcsFor(const Level &level)
    {
        std::vector<std::unique_ptr<Npc>> npcs;
        for (const NpcSpawnData &spawn : level.getNpcs())
            npcs.push_back(
                std::make_unique<Npc>(shippedNpcData().at(spawn.type), level.patrolFor(spawn)));

        return npcs;
    }

    ActorAsked askedFor(
        HeadlessImGui &gui,
        const Level &level,
        const std::vector<std::unique_ptr<Npc>> &npcs,
        ActorShown showing,
        std::optional<Armed> &armed)
    {
        ActorMotionState motion;
        ActorState playerState;
        ActorAsked asked;

        gui.frame(
            [&]
            {
                asked = drawActorsInLevel(
                    level,
                    npcs,
                    motion,
                    level.getTileMap().feetOnTile(level.getPlayerStartTile()),
                    playerState,
                    shippedNpcData(),
                    showing,
                    armed);
            });

        return asked;
    }
}

TEST_CASE("Left alone, the actors panel asks for nothing", "[ActorsInLevel]")
{
    HeadlessImGui gui;
    Level level = levelPlacing({villagerAt(glm::ivec2(2, Standing))});
    std::vector<std::unique_ptr<Npc>> npcs = liveNpcsFor(level);
    std::optional<Armed> armed;

    ActorShown showing{ActorShown::What::Npc, 0};
    ActorAsked asked = askedFor(gui, level, npcs, showing, armed);

    REQUIRE(asked.show == showing);
    REQUIRE_FALSE(asked.removeShownNpc);
    REQUIRE_FALSE(asked.clearShownBeat);
    REQUIRE_FALSE(asked.addNpcOfType);
    REQUIRE_FALSE(armed);
}

TEST_CASE("An npc the level no longer has stops being shown", "[ActorsInLevel]")
{
    HeadlessImGui gui;
    Level level = levelPlacing({villagerAt(glm::ivec2(2, Standing))});
    std::vector<std::unique_ptr<Npc>> npcs = liveNpcsFor(level);
    std::optional<Armed> armed;

    ActorAsked asked = askedFor(gui, level, npcs, ActorShown{ActorShown::What::Npc, 7}, armed);

    REQUIRE(asked.show == ActorShown{});
    REQUIRE_FALSE(asked.removeShownNpc);
}

TEST_CASE("An npc placed but not yet running is drawn without one", "[ActorsInLevel]")
{
    HeadlessImGui gui;
    Level level = levelPlacing({villagerAt(glm::ivec2(2, Standing))});
    std::vector<std::unique_ptr<Npc>> nobody;
    std::optional<Armed> armed;

    ActorShown showing{ActorShown::What::Npc, 0};
    ActorAsked asked = askedFor(gui, level, nobody, showing, armed);

    REQUIRE(asked.show == showing);
}

TEST_CASE("An npc with a beat it cannot walk is still drawn", "[ActorsInLevel]")
{
    HeadlessImGui gui;
    NpcSpawnData reachingTooHigh = villagerAt(glm::ivec2(2, Standing));
    reachingTooHigh.patrol = PatrolData{glm::ivec2(1, Standing), glm::ivec2(8, 1)};

    Level level = levelPlacing({reachingTooHigh});
    std::vector<std::unique_ptr<Npc>> npcs = liveNpcsFor(level);
    std::optional<Armed> armed;

    ActorShown showing{ActorShown::What::Npc, 0};

    REQUIRE_NOTHROW(askedFor(gui, level, npcs, showing, armed));
}

TEST_CASE("The player is drawn whether or not the level has npcs", "[ActorsInLevel]")
{
    HeadlessImGui gui;
    std::optional<Armed> armed;
    ActorShown showing{ActorShown::What::Player, 0};

    Level empty = levelPlacing({});
    std::vector<std::unique_ptr<Npc>> nobody;
    REQUIRE(askedFor(gui, empty, nobody, showing, armed).show == showing);

    Level peopled = levelPlacing({villagerAt(glm::ivec2(2, Standing))});
    std::vector<std::unique_ptr<Npc>> npcs = liveNpcsFor(peopled);
    REQUIRE(askedFor(gui, peopled, npcs, showing, armed).show == showing);
}

TEST_CASE("Showing nobody draws nobody and asks for nothing", "[ActorsInLevel]")
{
    HeadlessImGui gui;
    Level level = levelPlacing({villagerAt(glm::ivec2(2, Standing))});
    std::vector<std::unique_ptr<Npc>> npcs = liveNpcsFor(level);
    std::optional<Armed> armed;

    ActorAsked asked = askedFor(gui, level, npcs, ActorShown{}, armed);

    REQUIRE(asked.show == ActorShown{});
    REQUIRE_FALSE(asked.addNpcOfType);
}

TEST_CASE("A pick already armed survives being drawn", "[ActorsInLevel]")
{
    HeadlessImGui gui;
    Level level = levelPlacing({villagerAt(glm::ivec2(2, Standing))});
    std::vector<std::unique_ptr<Npc>> npcs = liveNpcsFor(level);
    std::optional<Armed> armed = PickTile{PickTile::For::PatrolFrom, 0};

    askedFor(gui, level, npcs, ActorShown{ActorShown::What::Npc, 0}, armed);

    REQUIRE(armed == std::optional<Armed>(PickTile{PickTile::For::PatrolFrom, 0}));
}
