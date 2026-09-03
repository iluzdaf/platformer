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
#include "player/player_data.hpp"
#include "test_helpers/test_tile_map_utils.hpp"

namespace
{
    NpcSpawnData spawnAt(std::string type, glm::ivec2 tilePosition)
    {
        NpcSpawnData spawn;
        spawn.type = std::move(type);
        spawn.position = feetOf(tilePosition);
        return spawn;
    }

    constexpr int MapTiles = 10;
    constexpr int FloorRow = 6;
    constexpr int CeilingRow = 4;
    constexpr glm::ivec2 StandingTile{2, FloorRow - 1};

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

    NavigationProfile profileOfHeight(float height)
    {
        return buildNavigationProfile(npcOfHeight(height).actorData);
    }

    PlayerData playerOfHeight(float height)
    {
        PlayerData playerData;
        playerData.actorData.physicsBodyData.colliderSize = glm::vec2(8.0f, height);
        return playerData;
    }

    LevelData corridorPlacing(const std::vector<NpcSpawnData> &npcs)
    {
        LevelData levelData;
        levelData.playerStart = feetOf(glm::ivec2(0, 0));
        levelData.tileMapData.indices =
            std::vector<std::vector<int>>(MapTiles, std::vector<int>(MapTiles, 0));
        for (int x = 0; x < MapTiles; ++x)
        {
            (*levelData.tileMapData.indices)[FloorRow][x] = 1;
            (*levelData.tileMapData.indices)[CeilingRow][x] = 1;
        }
        levelData.playerStart = feetOf(glm::ivec2(1, FloorRow - 1));
        levelData.npcs = npcs;
        return levelData;
    }

    Level levelPlacing(
        const std::vector<NpcSpawnData> &npcs,
        const PlayerData &playerData = playerOfHeight(13.0f))
    {
        return Level(
            corridorPlacing(npcs),
            palettesFrom(getDefaultTileDataMap()),
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
        LevelData levelData;
        levelData.playerStart = feetOf(glm::ivec2(0, 0));
        levelData.tileMapData.indices =
            std::vector<std::vector<int>>(LedgeMapRows, std::vector<int>(LedgeMapTiles, 0));
        std::vector<std::vector<int>> &indices = *levelData.tileMapData.indices;

        for (int x = 0; x < LedgeMapTiles; ++x)
        {
            indices[LedgeGroundRow][x] = 1;
            indices[LedgeGroundRow + 1][x] = 1;
        }

        for (int x = LedgeFirstTile; x <= LedgeLastTile; ++x)
            indices[LedgeRow][x] = 1;

        for (int x = BelowFirstTile; x <= BelowLastTile; ++x)
            indices[BelowRow][x] = 1;

        levelData.playerStart = feetOf(glm::ivec2(1, LedgeGroundRow - 1));
        levelData.npcs = npcs;

        return Level(
            levelData,
            palettesFrom(getDefaultTileDataMap()),
            playerOfHeight(13.0f),
            theUsualNpcs(),
            {});
    }

    size_t nodesOnTheFloor(const NavigationGraph &graph)
    {
        size_t count = 0;
        for (const auto &[id, node] : graph.getNodes())
            if (node.position.y == static_cast<float>(FloorRow * 16))
                ++count;
        return count;
    }
}

TEST_CASE("A level offers the tiles it was built from", "[Level]")
{
    Level level = levelPlacing({});

    REQUIRE(level.getTileMap().getWidth() == MapTiles);
    REQUIRE(level.getTileMap().getHeight() == MapTiles);
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

TEST_CASE("Editing the tiles changes what the graphs describe", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile)});
    NavigationProfile walker = profileOfHeight(13.0f);

    REQUIRE(nodesOnTheFloor(level.graphFor(walker)) == 2);

    level.getTileMap().setTileIndex(glm::ivec2(5, FloorRow), 0);
    level.rebuildGraphs();

    REQUIRE(nodesOnTheFloor(level.graphFor(walker)) == 4);
}

TEST_CASE("Rebuilding keeps a graph for every profile it had", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile), spawnAt("tall", StandingTile)});
    size_t before = level.getGraphs().size();

    level.rebuildGraphs();

    REQUIRE(level.getGraphs().size() == before);
    REQUIRE(nodesOnTheFloor(level.graphFor(profileOfHeight(13.0f))) == 2);
    REQUIRE(nodesOnTheFloor(level.graphFor(profileOfHeight(20.0f))) == 0);
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
        &level.graphFor(level.getNpc(0).getNavigationProfile()) ==
        &level.graphFor(level.getNpc(1).getNavigationProfile()));
    REQUIRE(
        &level.graphFor(level.getNpc(0).getNavigationProfile()) !=
        &level.graphFor(profileOfHeight(20.0f)));

    std::vector<std::string> names;
    for (const NamedNavigationGraph &graph : level.getGraphs())
        names.push_back(graph.name);

    REQUIRE(names.size() == 2);
    REQUIRE(std::find(names.begin(), names.end(), "player, alsoShort, short") != names.end());
}

TEST_CASE("A level can be pointed at a different level next", "[Level]")
{
    Level level = levelPlacing({});

    level.setNextLevel("levels/level4.json");

    REQUIRE(level.getNextLevel() == "levels/level4.json");
    REQUIRE(level.toLevelData().nextLevel == "levels/level4.json");
}

TEST_CASE("A level refuses to have no level next", "[Level]")
{
    Level level = levelPlacing({});

    REQUIRE_THROWS(level.setNextLevel(""));
    REQUIRE_FALSE(level.getNextLevel().empty());
}

TEST_CASE("A graph does not name the same actor twice", "[Level]")
{
    Level level =
        levelPlacing({spawnAt("short", StandingTile), spawnAt("short", {2, FloorRow - 1})});

    for (const NamedNavigationGraph &graph : level.getGraphs())
        REQUIRE(graph.name.find("short", graph.name.find("short") + 1) == std::string::npos);
}

TEST_CASE("A level takes an npc placed after it was built", "[Level]")
{
    Level level = levelPlacing({});
    REQUIRE(spawnsIn(level).empty());

    level.addNpc(spawnAt("short", StandingTile), theUsualNpcs().at("short"));

    REQUIRE(spawnsIn(level).size() == 1);
    REQUIRE(spawnsIn(level).front().position == feetOf(StandingTile));
    REQUIRE(spawnsIn(level).front().type == "short");
}

TEST_CASE("An npc placed after building is part of what the level would save", "[Level]")
{
    Level level = levelPlacing({});

    level.addNpc(spawnAt("short", StandingTile), theUsualNpcs().at("short"));

    REQUIRE(level.toLevelData().npcs == spawnsIn(level));
}

TEST_CASE("An npc is built from the data it was added with", "[Level]")
{
    Level level = levelPlacing({});

    level.addNpc(spawnAt("tall", StandingTile), theUsualNpcs().at("tall"));

    REQUIRE(level.getNpcs().front()->getNavigationProfile() == profileOfHeight(20.0f));
}

TEST_CASE("A level keeps the npcs it already had when another is placed", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile)});

    level.addNpc(spawnAt("short", glm::ivec2(4, FloorRow - 1)), theUsualNpcs().at("short"));

    REQUIRE(spawnsIn(level).size() == 2);
}

TEST_CASE("A level lets go of an npc it was placing", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile)});

    level.removeNpc(0);

    REQUIRE(spawnsIn(level).empty());
}

TEST_CASE("Removing an npc leaves the others where they were", "[Level]")
{
    glm::ivec2 secondTile(4, FloorRow - 1);
    Level level = levelPlacing({spawnAt("short", StandingTile), spawnAt("short", secondTile)});

    level.removeNpc(0);

    REQUIRE(spawnsIn(level).size() == 1);
    REQUIRE(spawnsIn(level).front().position == feetOf(secondTile));
}

TEST_CASE("A level refuses to remove an npc it does not have", "[Level]")
{
    Level level = levelPlacing({});

    REQUIRE_THROWS_WITH(level.removeNpc(0), "This level has no npc 0, it has 0");
}

TEST_CASE("An npc removed is gone from what the level would save", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile)});

    level.removeNpc(0);

    REQUIRE(level.toLevelData().npcs.empty());
}

TEST_CASE("An npc can be moved to another tile", "[Level]")
{
    glm::ivec2 elsewhere(4, FloorRow - 1);
    Level level = levelPlacing({spawnAt("short", StandingTile)});

    level.getNpc(0).moveTo(feetOf(elsewhere));

    REQUIRE(spawnsIn(level).front().position == feetOf(elsewhere));
    REQUIRE(level.toLevelData().npcs.front().position == feetOf(elsewhere));
}

TEST_CASE("A level refuses to move an npc it does not have", "[Level]")
{
    Level level = levelPlacing({});

    REQUIRE_THROWS_WITH(
        level.getNpc(0).moveTo(glm::vec2(0.0f)), "This level has no npc 0, it has 0");
}

TEST_CASE("A level refuses to move an npc off the map", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile)});

    REQUIRE_THROWS(level.getTileMap().feetOnTile(glm::ivec2(-1, 0)));
    REQUIRE(spawnsIn(level).front().position == feetOf(StandingTile));
}

TEST_CASE("An npc can be given a beat it did not have", "[Level]")
{
    glm::ivec2 other(4, FloorRow - 1);
    Level level = levelPlacing({spawnAt("short", StandingTile)});
    REQUIRE_FALSE(spawnsIn(level).front().patrol);

    PatrolData beat = beatOf(StandingTile, other);
    level.getNpc(0).setPatrol(beat);

    REQUIRE(spawnsIn(level).front().patrol == beat);
    REQUIRE(level.toLevelData().npcs.front().patrol == beat);
}

TEST_CASE("A beat cannot be built from a tile off the map", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile)});

    REQUIRE_THROWS(beatBetween(level.getTileMap(), StandingTile, glm::ivec2(-1, 0)));
    REQUIRE_FALSE(spawnsIn(level).front().patrol);
}

TEST_CASE("A level refuses to give a beat to an npc it does not have", "[Level]")
{
    Level level = levelPlacing({});

    REQUIRE_THROWS_WITH(
        level.getNpc(0).setPatrol(PatrolData{}), "This level has no npc 0, it has 0");
}

TEST_CASE("An npc can be left with no beat at all", "[Level]")
{
    glm::ivec2 other(4, FloorRow - 1);
    Level level = levelPlacing({spawnAt("short", StandingTile)});
    level.getNpc(0).setPatrol(beatOf(StandingTile, other));

    level.getNpc(0).clearPatrol();

    REQUIRE_FALSE(spawnsIn(level).front().patrol);
    REQUIRE_FALSE(level.toLevelData().npcs.front().patrol);
}

TEST_CASE("A level refuses to clear the beat of an npc it does not have", "[Level]")
{
    Level level = levelPlacing({});

    REQUIRE_THROWS_WITH(level.getNpc(0).clearPatrol(), "This level has no npc 0, it has 0");
}

TEST_CASE("Placing an npc adds no graph, because its type already had one", "[Level]")
{
    Level level = levelPlacing({});
    std::size_t before = level.getGraphs().size();

    level.addNpc(spawnAt("tall", StandingTile), theUsualNpcs().at("tall"));

    REQUIRE_NOTHROW(level.graphFor(level.getNpc(0).getNavigationProfile()));
    REQUIRE(level.getGraphs().size() == before);
}

TEST_CASE("A level refuses an npc of a type it has never heard of", "[Level]")
{
    Level level = levelPlacing({});

    REQUIRE_THROWS(level.addNpc(spawnAt("dragon", StandingTile), NpcData{}));
    REQUIRE(spawnsIn(level).empty());
}

TEST_CASE("The run beneath an npc reaches both ends of the floor it stands on", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile)});

    std::optional<PatrolData> run = level.runBeneathNpc(0);

    REQUIRE(run);
    REQUIRE(run->from.y == feetOf(StandingTile).y);
    REQUIRE(run->to.y == feetOf(StandingTile).y);
    REQUIRE(run->from.x < feetOf(StandingTile).x);
    REQUIRE(run->to.x > feetOf(StandingTile).x);
}

TEST_CASE("The run beneath an npc names tiles the beat can be saved as", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile)});

    std::optional<PatrolData> run = level.runBeneathNpc(0);

    REQUIRE(run);
    REQUIRE_NOTHROW(level.getNpc(0).setPatrol(*run));
    REQUIRE(spawnsIn(level).front().patrol == run);
}

TEST_CASE("Looking under an npc reads the graph that npc walks", "[Level]")
{
    Level tallOne = levelPlacing({spawnAt("tall", StandingTile)});
    Level shortOne = levelPlacing({spawnAt("short", StandingTile)});

    REQUIRE(tallOne.runBeneathNpc(0) != shortOne.runBeneathNpc(0));
}

TEST_CASE("A level refuses to look under an npc it does not have", "[Level]")
{
    Level level = levelPlacing({});

    REQUIRE_THROWS_WITH(level.runBeneathNpc(0), "This level has no npc 0, it has 0");
}

TEST_CASE("A beat reaches the outer edges of the tiles it names", "[Level]")
{
    NpcSpawnData spawn = spawnAt("short", OnTheLedge);
    spawn.patrol = beatOf(glm::ivec2(1, LedgeRow - 1), glm::ivec2(4, LedgeRow - 1));
    Level level = levelWithALedge({spawn});
    float tileSize = static_cast<float>(level.getTileMap().getTileSize());

    const std::optional<PatrolData> &beat = level.getNpc(0).getSpawn().patrol;

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

    const std::optional<PatrolData> &beat = level.getNpc(0).getSpawn().patrol;

    REQUIRE(beat);
    REQUIRE(beat->from.x == 5 * tileSize);
    REQUIRE(beat->to.x == 1 * tileSize);
}

TEST_CASE("A level hands back the npc standing at an index", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile), spawnAt("tall", StandingTile)});

    REQUIRE(level.getNpc(0).getSpawn().type == "short");
    REQUIRE(level.getNpc(1).getSpawn().type == "tall");
    REQUIRE(&level.getNpc(1) == level.getNpcs()[1].get());
}

TEST_CASE("A level says how many npcs it has when asked for one it lacks", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile)});

    REQUIRE_THROWS_WITH(level.getNpc(1), "This level has no npc 1, it has 1");
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

    glm::ivec2 left(1, FloorRow - 1), right(6, FloorRow - 1);

    REQUIRE(tilesOfBeat(tileMap, beatBetween(tileMap, left, right)) == std::pair(left, right));
    REQUIRE(tilesOfBeat(tileMap, beatBetween(tileMap, right, left)) == std::pair(right, left));
}
