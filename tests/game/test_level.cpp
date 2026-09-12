#include <algorithm>
#include "game/beat_between.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <cstddef>
#include <exception>
#include <string>
#include <map>
#include <optional>
#include <utility>
#include <vector>
#include "navigation/navigation_profile.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_path.hpp"
#include "tile_map/tile_map.hpp"
#include "navigation/named_navigation_graph.hpp"
#include "navigation/navigation_profile_builder.hpp"
#include "game/level.hpp"
#include "game/level_data.hpp"
#include "npc/npc_spawn_data.hpp"
#include "npc/npc_data.hpp"
#include "npc/npc.hpp"
#include "pickups/pickup_data.hpp"
#include "pickups/pickup_spawn_data.hpp"
#include "player/player_data.hpp"
#include "helpers/palettes.hpp"
#include "helpers/tiles.hpp"
#include "helpers/levels.hpp"
#include "helpers/actors.hpp"

namespace
{
    constexpr int CeilingRow = 4;
    constexpr glm::ivec2 StandingTile{2, FloorLevelStanding};

    NpcData npcOfHeight(float height)
    {
        NpcData npcData;
        npcData.actorData.physicsBodyData.colliderSize = glm::vec2(8.0f, height);
        return npcData;
    }

    std::map<std::string, NpcData> theUsualNpcs()
    {
        return {
            {"short", npcOfHeight(13.0f)},
            {"tall", npcOfHeight(20.0f)},
            {"alsoShort", npcOfHeight(13.0f)}};
    }

    PlayerData playerOfHeight(float height)
    {
        PlayerData playerData;
        playerData.actorData.physicsBodyData.colliderSize = glm::vec2(8.0f, height);
        return playerData;
    }

    LevelData corridorPlacing(const std::vector<NpcSpawnData> &npcs)
    {
        Placed laid;
        layRow(laid, FloorLevelRow, 0, FloorLevelTiles - 1);
        layRow(laid, CeilingRow, 0, FloorLevelTiles - 1);
        return aLevelPlacing(
            laid, FloorLevelTiles, FloorLevelTiles, glm::ivec2(1, FloorLevelStanding), npcs);
    }

    Level levelOf(const LevelData &levelData, const PlayerData &playerData = playerOfHeight(13.0f))
    {
        return Level(
            levelData, theOnlyPalette(aPaletteWithASolidTile()), playerData, theUsualNpcs(), {});
    }

    Level levelPlacing(
        const std::vector<NpcSpawnData> &npcs,
        const PlayerData &playerData = playerOfHeight(13.0f))
    {
        return Level(
            corridorPlacing(npcs),
            theOnlyPalette(aPaletteWithASolidTile()),
            playerData,
            theUsualNpcs(),
            {});
    }

    constexpr int LedgeMapTiles = 14;
    constexpr int LedgeMapRows = 13;
    constexpr int LedgeGroundRow = 11;
    constexpr int LedgeRow = 5;
    constexpr int LedgeFirstTile = 0;
    constexpr int LedgeLastTile = 6;
    constexpr int BelowRow = 7;
    constexpr int BelowFirstTile = 4;
    constexpr int BelowLastTile = 9;
    constexpr glm::ivec2 OnTheLedge{3, LedgeRow - 1};

    Level levelWithALedge(const std::vector<NpcSpawnData> &npcs)
    {
        Placed laid;
        layRow(laid, LedgeGroundRow, 0, LedgeMapTiles - 1);
        layRow(laid, LedgeGroundRow + 1, 0, LedgeMapTiles - 1);
        layRow(laid, LedgeRow, LedgeFirstTile, LedgeLastTile);
        layRow(laid, BelowRow, BelowFirstTile, BelowLastTile);

        return Level(
            aLevelPlacing(
                laid, LedgeMapTiles, LedgeMapRows, glm::ivec2(1, LedgeGroundRow - 1), npcs),
            theOnlyPalette(aPaletteWithASolidTile()),
            playerOfHeight(13.0f),
            theUsualNpcs(),
            {});
    }

    size_t nodesOnTheFloor(const NavigationGraph &graph)
    {
        size_t count = 0;
        for (const auto &[id, node] : graph.getNodes())
            if (node.feet.y == surfaceOf(FloorLevelRow))
                ++count;
        return count;
    }
}

TEST_CASE("A level offers the tiles it was built from", "[Level]")
{
    Level level = levelPlacing({});

    REQUIRE(level.getTileMap().getWidth() == FloorLevelTiles);
    REQUIRE(level.getTileMap().getHeight() == FloorLevelTiles);
}

TEST_CASE("A level builds a graph for an npc it places", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile)});

    const NavigationGraph &graph = level.graphFor(profileOfHeight(13.0f));

    REQUIRE(nodesOnTheFloor(graph) == 2);
    REQUIRE_FALSE(graph.getEdges().empty());
}

TEST_CASE("A level has no graph for an actor it was not told about", "[Level]")
{
    Level level = levelPlacing({});

    REQUIRE_THROWS_AS(level.graphFor(profileOfHeight(99.0f)), std::exception);
}

TEST_CASE("A level builds a graph for the player as well", "[Level]")
{
    Level level = levelPlacing({}, playerOfHeight(20.0f));

    REQUIRE(nodesOnTheFloor(level.graphFor(profileOfHeight(20.0f))) == 0);
}

TEST_CASE("Npcs that move differently get their own graphs", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile), spawnAt("tall", StandingTile)});

    const NavigationGraph &shortGraph = level.graphFor(profileOfHeight(13.0f));
    const NavigationGraph &tallGraph = level.graphFor(profileOfHeight(20.0f));

    REQUIRE(&shortGraph != &tallGraph);
    REQUIRE(nodesOnTheFloor(shortGraph) == 2);
    REQUIRE(nodesOnTheFloor(tallGraph) == 0);
}

TEST_CASE("A level naming an npc that does not exist fails to load", "[Level]")
{
    REQUIRE_THROWS_WITH(
        levelPlacing({spawnAt("nobody", StandingTile)}),
        Catch::Matchers::ContainsSubstring("nobody"));
}

TEST_CASE("A gap in the floor changes what the graphs describe", "[Level]")
{
    NavigationProfile walker = profileOfHeight(13.0f);

    LevelData whole = corridorPlacing({spawnAt("short", StandingTile)});
    LevelData holed = whole;
    holed.tileMapData.indices[FloorLevelRow][5] = EmptyTile;

    REQUIRE(nodesOnTheFloor(levelOf(whole).graphFor(walker)) == 2);
    REQUIRE(nodesOnTheFloor(levelOf(holed).graphFor(walker)) == 4);
}

TEST_CASE("A graph is named for every actor that navigates by it", "[Level]")
{
    Level level = levelPlacing({});

    std::vector<std::string> names;
    for (const NamedNavigationGraph &graph : level.getGraphs())
        names.push_back(graph.name);

    REQUIRE(std::find(names.begin(), names.end(), "player, alsoShort, short") != names.end());
    REQUIRE(std::find(names.begin(), names.end(), "tall") != names.end());
}

TEST_CASE("Every type the level knows has a graph before anything is placed", "[Level]")
{
    Level level = levelPlacing({});

    REQUIRE(spawnsIn(level).empty());
    REQUIRE_NOTHROW(level.graphFor(profileOfHeight(20.0f)));
    REQUIRE_NOTHROW(level.graphFor(profileOfHeight(13.0f)));
}

TEST_CASE("Actors that navigate alike share a graph, and it says so", "[Level]")
{
    Level level =
        levelPlacing({spawnAt("short", StandingTile), spawnAt("alsoShort", StandingTile)});

    REQUIRE(
        &level.graphFor(level.getNpcs()[0]->profile()) ==
        &level.graphFor(level.getNpcs()[1]->profile()));
    REQUIRE(
        &level.graphFor(level.getNpcs()[0]->profile()) != &level.graphFor(profileOfHeight(20.0f)));

    std::vector<std::string> names;
    for (const NamedNavigationGraph &graph : level.getGraphs())
        names.push_back(graph.name);

    REQUIRE(names.size() == 2);
    REQUIRE(std::find(names.begin(), names.end(), "player, alsoShort, short") != names.end());
}

TEST_CASE("A level names the level its data points at next", "[Level]")
{
    LevelData levelData = corridorPlacing({});
    levelData.nextLevel.path = "levels/level4.json";

    REQUIRE(levelOf(levelData).getNextLevel() == "levels/level4.json");
}

TEST_CASE("A level whose data names no level next is refused", "[Level]")
{
    LevelData levelData = corridorPlacing({});
    levelData.nextLevel.path.clear();

    REQUIRE_THROWS(levelOf(levelData));
}

TEST_CASE("A graph does not name the same actor twice", "[Level]")
{
    Level level =
        levelPlacing({spawnAt("short", StandingTile), spawnAt("short", {2, FloorLevelStanding})});

    for (const NamedNavigationGraph &graph : level.getGraphs())
        REQUIRE(graph.name.find("short", graph.name.find("short") + 1) == std::string::npos);
}

TEST_CASE("An npc is built from the catalogue entry its type names", "[Level]")
{
    Level level = levelPlacing({spawnAt("tall", StandingTile)});

    REQUIRE(level.getNpcs().front()->profile() == profileOfHeight(20.0f));
}

TEST_CASE("A tile off the map has no feet to stand on", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile)});

    REQUIRE_THROWS(level.getTileMap().feetOnTile(glm::ivec2(-1, 0)));
}

TEST_CASE("A beat cannot be built from a tile off the map", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile)});

    REQUIRE_THROWS(beatBetween(level.getTileMap(), StandingTile, glm::ivec2(-1, 0)));
}

TEST_CASE("A level refuses an npc of a type it has never heard of", "[Level]")
{
    REQUIRE_THROWS(levelPlacing({spawnAt("dragon", StandingTile)}));
}

TEST_CASE("The run beneath an npc reaches both ends of the floor it stands on", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile)});

    std::optional<PatrolData> run = level.runBeneath(profileOfHeight(13.0f), feetOf(StandingTile));

    REQUIRE(run);
    REQUIRE(run->from.y == feetOf(StandingTile).y);
    REQUIRE(run->to.y == feetOf(StandingTile).y);
    REQUIRE(run->from.x < feetOf(StandingTile).x);
    REQUIRE(run->to.x > feetOf(StandingTile).x);
}

TEST_CASE("A run handed back is a beat a level will take", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile)});
    std::optional<PatrolData> run = level.runBeneath(profileOfHeight(13.0f), feetOf(StandingTile));

    REQUIRE(run);

    LevelData walking = corridorPlacing({spawnAt("short", StandingTile)});
    walking.npcs.front().patrol = run;

    REQUIRE(spawnsIn(levelOf(walking)).front().patrol == run);
}

TEST_CASE("Looking under an npc reads the graph that npc walks", "[Level]")
{
    Level level = levelPlacing({});

    REQUIRE(
        level.runBeneath(profileOfHeight(20.0f), feetOf(StandingTile)) !=
        level.runBeneath(profileOfHeight(13.0f), feetOf(StandingTile)));
}

TEST_CASE("A level has nothing to look under for a profile it does not graph", "[Level]")
{
    Level level = levelPlacing({});

    REQUIRE_THROWS(level.runBeneath(profileOfHeight(99.0f), feetOf(StandingTile)));
}

TEST_CASE("A beat reaches the outer edges of the tiles it names", "[Level]")
{
    NpcSpawnData spawn = spawnAt("short", OnTheLedge);
    spawn.patrol = beatOf(glm::ivec2(1, LedgeRow - 1), glm::ivec2(4, LedgeRow - 1));
    Level level = levelWithALedge({spawn});
    float tileSize = static_cast<float>(level.getTileMap().getTileSize());

    const std::optional<PatrolData> &beat = level.getNpcs()[0]->getSpawn().patrol;

    REQUIRE(beat);
    REQUIRE(beat->from.x == 1 * tileSize);
    REQUIRE(beat->to.x == 5 * tileSize);
}

TEST_CASE("A beat named right to left reaches the same two edges", "[Level]")
{
    NpcSpawnData spawn = spawnAt("short", OnTheLedge);
    spawn.patrol = beatOf(glm::ivec2(4, LedgeRow - 1), glm::ivec2(1, LedgeRow - 1));
    Level level = levelWithALedge({spawn});
    float tileSize = static_cast<float>(level.getTileMap().getTileSize());

    const std::optional<PatrolData> &beat = level.getNpcs()[0]->getSpawn().patrol;

    REQUIRE(beat);
    REQUIRE(beat->from.x == 5 * tileSize);
    REQUIRE(beat->to.x == 1 * tileSize);
}

TEST_CASE("A level stands its npcs up in the order its data places them", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile), spawnAt("tall", StandingTile)});

    REQUIRE(level.getNpcs().size() == 2);
    REQUIRE(level.getNpcs()[0]->getSpawn().type == "short");
    REQUIRE(level.getNpcs()[1]->getSpawn().type == "tall");
}

TEST_CASE("A tile picked in the editor survives the trip through world space", "[Level]")
{
    Level level = levelPlacing({});
    const TileMap &tileMap = level.getTileMap();

    for (int x = 0; x < tileMap.getWidth(); ++x)
        for (int y = 0; y < tileMap.getHeight(); ++y)
        {
            glm::ivec2 tile(x, y);
            REQUIRE(tileMap.tileUnderFeet(tileMap.feetOnTile(tile)) == tile);
        }
}

TEST_CASE("A beat picked in the editor survives the trip through world space", "[Level]")
{
    Level level = levelPlacing({});
    const TileMap &tileMap = level.getTileMap();

    glm::ivec2 left(1, FloorLevelStanding), right(6, FloorLevelStanding);

    REQUIRE(tilesOfBeat(tileMap, beatBetween(tileMap, left, right)) == std::pair(left, right));
    REQUIRE(tilesOfBeat(tileMap, beatBetween(tileMap, right, left)) == std::pair(right, left));
}

TEST_CASE("A beat between tiles is the same with or without a map", "[Level]")
{
    Level level = levelPlacing({});
    const TileMap &tileMap = level.getTileMap();

    REQUIRE(beatBetween(tileMap, {1, 4}, {5, 4}) == beatBetween({1, 4}, {5, 4}, 16));
    REQUIRE(beatBetween({1, 4}, {5, 4}, 16) == PatrolData{{16.0f, 80.0f}, {96.0f, 80.0f}});
}

namespace
{
    std::map<std::string, NpcData> withTheTallOnesTaller(float height)
    {
        std::map<std::string, NpcData> npcs = theUsualNpcs();
        npcs.at("tall") = npcOfHeight(height);
        return npcs;
    }

    PickupData aPickupWorth(int scoreDelta)
    {
        PickupData pickup;
        pickup.size = glm::vec2(8.0f);
        pickup.scoreDelta = scoreDelta;
        return pickup;
    }

    void tick(Level &level)
    {
        level.beginFrame();
        level.fixedUpdate(0.01f, glm::vec2(0.0f));
    }
}

TEST_CASE(
    "Rebuilding the graphs for a new cast gives it the graphs it walks, and drops the old",
    "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile)});
    REQUIRE_THROWS_AS(level.graphFor(profileOfHeight(24.0f)), std::exception);

    level.rebuildGraphsFor(playerOfHeight(30.0f), withTheTallOnesTaller(24.0f));

    REQUIRE_NOTHROW(level.graphFor(profileOfHeight(24.0f)));
    REQUIRE_NOTHROW(level.graphFor(profileOfHeight(30.0f)));
    REQUIRE_THROWS_AS(level.graphFor(profileOfHeight(20.0f)), std::exception);
}

TEST_CASE("Rebuilding the graphs hands every creature the graph it walks now", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile)});
    Npc &creature = *level.getNpcs()[0];
    tick(level);
    REQUIRE_NOTHROW(creature.onSameSurfaceAs(creature.feet()));

    level.rebuildGraphsFor(playerOfHeight(13.0f), theUsualNpcs());

    REQUIRE(creature.onSameSurfaceAs(creature.feet()));
    REQUIRE_FALSE(creature.onSameSurfaceAs(creature.feet() - glm::vec2(0.0f, 64.0f)));
}

TEST_CASE("Remaking a creature puts a new one at its spawn, built from the data given", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile), spawnAt("tall", StandingTile)});
    const Npc *shortOne = level.getNpcs()[0].get();
    const Npc *tallOne = level.getNpcs()[1].get();
    level.getNpcs()[1]->standAt(tallOne->feet() + glm::vec2(24.0f, 0.0f));

    Npc &remade = level.remake(1, npcOfHeight(24.0f));

    REQUIRE(&remade == level.getNpcs()[1].get());
    REQUIRE(&remade != tallOne);
    REQUIRE(level.getNpcs()[0].get() == shortOne);
    REQUIRE(remade.feet() == feetOf(StandingTile));
    REQUIRE(remade.getSpawn().type == "tall");
    REQUIRE(remade.builtFrom().actorData.physicsBodyData.colliderSize.y == 24.0f);
}

TEST_CASE(
    "Recasting the pickups re-makes only the changed kind, and what was taken stays taken",
    "[Level]")
{
    LevelData levelData = corridorPlacing({});
    levelData.pickups = {
        PickupSpawnData{"coin", feetOf(glm::ivec2(3, FloorLevelStanding))},
        PickupSpawnData{"gem", feetOf(glm::ivec2(5, FloorLevelStanding))},
        PickupSpawnData{"coin", feetOf(glm::ivec2(7, FloorLevelStanding))}};
    std::map<std::string, PickupData> kinds{{"coin", aPickupWorth(1)}, {"gem", aPickupWorth(5)}};
    Level level(
        levelData,
        theOnlyPalette(aPaletteWithASolidTile()),
        playerOfHeight(13.0f),
        theUsualNpcs(),
        kinds);
    level.takePickupsTouching(level.getPickups()[0].getAABB());
    REQUIRE(level.getPickups().size() == 3);
    REQUIRE_FALSE(level.getPickups()[0].stillThere());

    kinds.at("coin") = aPickupWorth(3);
    level.recastPickups(kinds);

    REQUIRE(level.getPickups().size() == 3);
    REQUIRE(level.getPickups()[0].getScoreDelta() == 3);
    REQUIRE_FALSE(level.getPickups()[0].stillThere());
    REQUIRE(level.getPickups()[1].getScoreDelta() == 5);
    REQUIRE(level.getPickups()[2].getScoreDelta() == 3);
}

TEST_CASE("Recasting the pickups without a kind on the floor is refused by name", "[Level]")
{
    LevelData levelData = corridorPlacing({});
    levelData.pickups = {PickupSpawnData{"coin", feetOf(glm::ivec2(3, FloorLevelStanding))}};
    std::map<std::string, PickupData> kinds{{"coin", aPickupWorth(1)}};
    Level level(
        levelData,
        theOnlyPalette(aPaletteWithASolidTile()),
        playerOfHeight(13.0f),
        theUsualNpcs(),
        kinds);

    REQUIRE_THROWS_WITH(level.recastPickups({}), Catch::Matchers::ContainsSubstring("coin"));
    REQUIRE(level.getPickups().size() == 1);
}

TEST_CASE("Rebuilding the graphs under a creature whose kind is gone is refused", "[Level]")
{
    Level level = levelPlacing({spawnAt("tall", StandingTile)});

    REQUIRE_THROWS_WITH(
        level.rebuildGraphsFor(playerOfHeight(13.0f), withTheTallOnesTaller(24.0f)),
        Catch::Matchers::ContainsSubstring("no navigation graph"));
}
