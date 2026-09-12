#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <string>
#include <vector>
#include "animations/animator_data.hpp"
#include "actor/observed.hpp"
#include "actor/actor_state.hpp"
#include "game/level.hpp"
#include "game/level_data.hpp"
#include "npc/npc_spawn_data.hpp"
#include "pickups/pickup_spawn_data.hpp"
#include "player/player_data.hpp"
#include "ui/actors_in_level.hpp"
#include "ui/armed.hpp"
#include "helpers/headless_imgui.hpp"
#include "helpers/palettes.hpp"
#include "helpers/shipped.hpp"
#include "helpers/tile_positions.hpp"
#include "helpers/levels.hpp"
#include "helpers/floor_level.hpp"

namespace
{
    Level levelPlacing(const std::vector<NpcSpawnData> &npcs)
    {
        return Level(
            aFloorLevelPlacing(npcs),
            theOnlyPalette(aPaletteWithASolidTile()),
            PlayerData(),
            shippedNpcData(),
            shippedPickupData());
    }

    constexpr int IslandRow = 1;
    constexpr int IslandFirstTile = 7;

    Level levelHolding(const std::vector<PickupSpawnData> &pickups);

    Level levelWithAnIsland(const std::vector<NpcSpawnData> &npcs)
    {
        LevelData levelData = aFloorLevelPlacing(npcs);
        for (int x = IslandFirstTile; x < FloorLevelTiles; ++x)
            levelData.tileMapData.indices[IslandRow][x] = SolidTile;

        return Level(
            levelData,
            theOnlyPalette(aPaletteWithASolidTile()),
            PlayerData(),
            shippedNpcData(),
            shippedPickupData());
    }

    Level levelHolding(const std::vector<PickupSpawnData> &pickups)
    {
        LevelData levelData = aFloorLevelPlacing({});
        levelData.pickups = pickups;

        return Level(
            levelData,
            theOnlyPalette(aPaletteWithASolidTile()),
            PlayerData(),
            shippedNpcData(),
            shippedPickupData());
    }

    ActorAsked askedFor(
        HeadlessImGui &gui,
        const Level &level,
        ActorShown showing,
        std::optional<Armed> &armed)
    {
        AnimatorData animations;
        Observed observed;
        ActorState playerState;
        ActorAsked asked;

        gui.frame(
            [&]
            {
                asked = drawActorsInLevel(
                    level,
                    animations,
                    observed,
                    level.getTileMap().feetOnTile(
                        level.getTileMap().tileUnderFeet(level.getPlayerStart())),
                    playerState,
                    shippedNpcData(),
                    showing,
                    armed);
            });

        return asked;
    }
}

TEST_CASE("A pickup in the level is shown the way an npc is", "[ActorsInLevel]")
{
    HeadlessImGui gui;
    Level level =
        levelHolding({PickupSpawnData{"coin", feetOf(glm::ivec2(2, FloorLevelStanding))}});
    std::optional<Armed> armed;

    ActorShown showing{ActorShown::What::Pickup, 0};
    ActorAsked asked = askedFor(gui, level, showing, armed);

    REQUIRE(asked.show == showing);
    REQUIRE_FALSE(asked.removeShown);
}

TEST_CASE("A pickup the level no longer has stops being shown", "[ActorsInLevel]")
{
    HeadlessImGui gui;
    Level level =
        levelHolding({PickupSpawnData{"coin", feetOf(glm::ivec2(2, FloorLevelStanding))}});
    std::optional<Armed> armed;

    ActorAsked asked = askedFor(gui, level, ActorShown{ActorShown::What::Pickup, 7}, armed);

    REQUIRE(asked.show == ActorShown{});
}

TEST_CASE("Left alone, the actors panel asks for nothing", "[ActorsInLevel]")
{
    HeadlessImGui gui;
    Level level = levelPlacing({aRatAt(glm::ivec2(2, FloorLevelStanding))});
    std::optional<Armed> armed;

    ActorShown showing{ActorShown::What::Npc, 0};
    ActorAsked asked = askedFor(gui, level, showing, armed);

    REQUIRE(asked.show == showing);
    REQUIRE_FALSE(asked.removeShown);
    REQUIRE_FALSE(asked.clearShownBeat);
    REQUIRE_FALSE(armed);
}

TEST_CASE("An npc the level no longer has stops being shown", "[ActorsInLevel]")
{
    HeadlessImGui gui;
    Level level = levelPlacing({aRatAt(glm::ivec2(2, FloorLevelStanding))});
    std::optional<Armed> armed;

    ActorAsked asked = askedFor(gui, level, ActorShown{ActorShown::What::Npc, 7}, armed);

    REQUIRE(asked.show == ActorShown{});
    REQUIRE_FALSE(asked.removeShown);
}

TEST_CASE("An npc with a beat it cannot walk is still drawn", "[ActorsInLevel]")
{
    HeadlessImGui gui;
    NpcSpawnData reachingTooHigh = aRatAt(glm::ivec2(2, FloorLevelStanding));
    reachingTooHigh.patrol = beatOf(glm::ivec2(1, FloorLevelStanding), glm::ivec2(8, 1));

    Level level = levelPlacing({reachingTooHigh});
    std::optional<Armed> armed;

    ActorShown showing{ActorShown::What::Npc, 0};

    REQUIRE_NOTHROW(askedFor(gui, level, showing, armed));
}

TEST_CASE("The player is drawn whether or not the level has npcs", "[ActorsInLevel]")
{
    HeadlessImGui gui;
    std::optional<Armed> armed;
    ActorShown showing{ActorShown::What::Player, 0};

    Level empty = levelPlacing({});
    REQUIRE(askedFor(gui, empty, showing, armed).show == showing);

    Level peopled = levelPlacing({aRatAt(glm::ivec2(2, FloorLevelStanding))});
    REQUIRE(askedFor(gui, peopled, showing, armed).show == showing);
}

TEST_CASE("Showing nobody draws nobody and asks for nothing", "[ActorsInLevel]")
{
    HeadlessImGui gui;
    Level level = levelPlacing({aRatAt(glm::ivec2(2, FloorLevelStanding))});
    std::optional<Armed> armed;

    ActorAsked asked = askedFor(gui, level, ActorShown{}, armed);

    REQUIRE(asked.show == ActorShown{});
}

TEST_CASE("A pick already armed survives being drawn", "[ActorsInLevel]")
{
    HeadlessImGui gui;
    Level level = levelPlacing({aRatAt(glm::ivec2(2, FloorLevelStanding))});
    std::optional<Armed> armed = PickTile{PickTile::For::PatrolFrom, 0};

    askedFor(gui, level, ActorShown{ActorShown::What::Npc, 0}, armed);

    REQUIRE(armed == std::optional<Armed>(PickTile{PickTile::For::PatrolFrom, 0}));
}

TEST_CASE("A level whose beats can all be walked stops no save", "[ActorsInLevel]")
{
    NpcSpawnData walkable = aRatAt(glm::ivec2(2, FloorLevelStanding));
    walkable.patrol = beatOf(glm::ivec2(1, FloorLevelStanding), glm::ivec2(8, FloorLevelStanding));

    REQUIRE_FALSE(npcsThatCannotGetBack(levelPlacing({walkable})));
    REQUIRE_FALSE(npcsThatCannotGetBack(levelPlacing({})));
}

TEST_CASE("An npc with no beat at all stops no save", "[ActorsInLevel]")
{
    REQUIRE_FALSE(npcsThatCannotGetBack(levelPlacing({aRatAt(glm::ivec2(2, FloorLevelStanding))})));
}

TEST_CASE("A beat that cannot be walked names the npc it belongs to", "[ActorsInLevel]")
{
    NpcSpawnData strandedHalfway = aRatAt(glm::ivec2(2, FloorLevelStanding));
    strandedHalfway.patrol =
        beatOf(glm::ivec2(1, FloorLevelStanding), glm::ivec2(IslandFirstTile + 1, IslandRow - 1));

    std::optional<std::string> fault = npcsThatCannotGetBack(levelWithAnIsland({strandedHalfway}));

    REQUIRE(fault);
    REQUIRE(*fault == "rat 1 cannot get back from there");
}

TEST_CASE("Every npc that cannot get back is named", "[ActorsInLevel]")
{
    constexpr glm::ivec2 OnTheIsland{IslandFirstTile + 1, IslandRow - 1};

    NpcSpawnData walkable = aRatAt(glm::ivec2(2, FloorLevelStanding));
    walkable.patrol = beatOf(glm::ivec2(1, FloorLevelStanding), glm::ivec2(5, FloorLevelStanding));

    NpcSpawnData stranded = aRatAt(glm::ivec2(3, FloorLevelStanding));
    stranded.patrol = beatOf(glm::ivec2(1, FloorLevelStanding), OnTheIsland);

    NpcSpawnData alsoStranded = aRatAt(glm::ivec2(4, FloorLevelStanding));
    alsoStranded.patrol = beatOf(glm::ivec2(2, FloorLevelStanding), OnTheIsland);

    std::optional<std::string> fault =
        npcsThatCannotGetBack(levelWithAnIsland({walkable, stranded, alsoStranded}));

    REQUIRE(fault);
    REQUIRE(*fault == "rat 2, rat 3 cannot get back from there");
}

TEST_CASE("A beat whose ends share a tile still draws both", "[ActorsInLevel]")
{
    HeadlessImGui gui;
    NpcSpawnData bothAtOnce = aRatAt(glm::ivec2(2, FloorLevelStanding));
    bothAtOnce.patrol =
        beatOf(glm::ivec2(4, FloorLevelStanding), glm::ivec2(4, FloorLevelStanding));

    Level level = levelPlacing({bothAtOnce});
    std::optional<Armed> armed = PickTile{PickTile::For::PatrolTo, 0};

    askedFor(gui, level, ActorShown{ActorShown::What::Npc, 0}, armed);

    REQUIRE(armed == std::optional<Armed>(PickTile{PickTile::For::PatrolTo, 0}));
}

TEST_CASE("An npc with no beat still offers both ends to place", "[ActorsInLevel]")
{
    HeadlessImGui gui;
    Level level = levelPlacing({aRatAt(glm::ivec2(2, FloorLevelStanding))});
    std::optional<Armed> armed = PickTile{PickTile::For::PatrolFrom, 0};

    ActorAsked asked = askedFor(gui, level, ActorShown{ActorShown::What::Npc, 0}, armed);

    REQUIRE(armed == std::optional<Armed>(PickTile{PickTile::For::PatrolFrom, 0}));
    REQUIRE_FALSE(asked.clearShownBeat);
}
