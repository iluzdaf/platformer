#include <algorithm>
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
        spawn.tilePosition = tilePosition;
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
        levelData.tileMapData.size = 16;
        levelData.tileMapData.indices =
            std::vector<std::vector<int>>(MapTiles, std::vector<int>(MapTiles, 0));
        for (int x = 0; x < MapTiles; ++x)
        {
            (*levelData.tileMapData.indices)[FloorRow][x] = 1;
            (*levelData.tileMapData.indices)[CeilingRow][x] = 1;
        }
        levelData.playerStartTilePosition = glm::ivec2(1, FloorRow - 1);
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
            theUsualNpcs());
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
        levelData.tileMapData.size = 16;
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

        levelData.playerStartTilePosition = glm::ivec2(1, LedgeGroundRow - 1);
        levelData.npcs = npcs;

        return Level(
            levelData,
            palettesFrom(getDefaultTileDataMap()),
            playerOfHeight(13.0f),
            theUsualNpcs());
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

    REQUIRE_THROWS_AS(level.graphFor(profileOfHeight(20.0f)), std::exception);
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
        Catch::Matchers::ContainsSubstring("nobody") &&
            Catch::Matchers::ContainsSubstring("new_level.json"));
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

TEST_CASE("A graph is named for the actor that needed it", "[Level]")
{
    Level level = levelPlacing({spawnAt("tall", StandingTile)});

    std::vector<std::string> names;
    for (const NamedNavigationGraph &graph : level.getGraphs())
        names.push_back(graph.name);

    REQUIRE(std::find(names.begin(), names.end(), "player") != names.end());
    REQUIRE(std::find(names.begin(), names.end(), "tall") != names.end());
}

TEST_CASE("Actors that navigate alike share a graph, and it says so", "[Level]")
{
    Level level =
        levelPlacing({spawnAt("short", StandingTile), spawnAt("alsoShort", StandingTile)});

    std::vector<std::string> names;
    for (const NamedNavigationGraph &graph : level.getGraphs())
        names.push_back(graph.name);

    REQUIRE(names.size() == 1);
    REQUIRE(names.front() == "player, short, alsoShort");
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
        REQUIRE(graph.name == "player, short");
}

TEST_CASE("A level takes an npc placed after it was built", "[Level]")
{
    Level level = levelPlacing({});
    REQUIRE(level.getNpcs().empty());

    level.addNpc(spawnAt("short", StandingTile));

    REQUIRE(level.getNpcs().size() == 1);
    REQUIRE(level.getNpcs().front().tilePosition == StandingTile);
    REQUIRE(level.getNpcs().front().type == "short");
}

TEST_CASE("An npc placed after building is part of what the level would save", "[Level]")
{
    Level level = levelPlacing({});

    level.addNpc(spawnAt("short", StandingTile));

    REQUIRE(level.toLevelData().npcs == level.getNpcs());
}

TEST_CASE("A level keeps the npcs it already had when another is placed", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile)});

    level.addNpc(spawnAt("short", glm::ivec2(4, FloorRow - 1)));

    REQUIRE(level.getNpcs().size() == 2);
}

TEST_CASE("A level lets go of an npc it was placing", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile)});

    level.removeNpc(0);

    REQUIRE(level.getNpcs().empty());
}

TEST_CASE("Removing an npc leaves the others where they were", "[Level]")
{
    glm::ivec2 secondTile(4, FloorRow - 1);
    Level level = levelPlacing({spawnAt("short", StandingTile), spawnAt("short", secondTile)});

    level.removeNpc(0);

    REQUIRE(level.getNpcs().size() == 1);
    REQUIRE(level.getNpcs().front().tilePosition == secondTile);
}

TEST_CASE("A level refuses to remove an npc it does not have", "[Level]")
{
    Level level = levelPlacing({});

    REQUIRE_THROWS_WITH(level.removeNpc(0), "Cannot remove an npc the level does not have");
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

    level.setNpcSpawnTile(0, elsewhere);

    REQUIRE(level.getNpcs().front().tilePosition == elsewhere);
    REQUIRE(level.toLevelData().npcs.front().tilePosition == elsewhere);
}

TEST_CASE("A level refuses to move an npc it does not have", "[Level]")
{
    Level level = levelPlacing({});

    REQUIRE_THROWS_WITH(
        level.setNpcSpawnTile(0, StandingTile), "Cannot move an npc the level does not have");
}

TEST_CASE("A level refuses to move an npc off the map", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile)});

    REQUIRE_THROWS(level.setNpcSpawnTile(0, glm::ivec2(-1, 0)));
    REQUIRE(level.getNpcs().front().tilePosition == StandingTile);
}

TEST_CASE("An npc can be given a beat it did not have", "[Level]")
{
    glm::ivec2 other(4, FloorRow - 1);
    Level level = levelPlacing({spawnAt("short", StandingTile)});
    REQUIRE_FALSE(level.getNpcs().front().patrol);

    level.setNpcPatrol(0, PatrolData{StandingTile, other});

    REQUIRE(level.getNpcs().front().patrol == PatrolData{StandingTile, other});
    REQUIRE(level.toLevelData().npcs.front().patrol == PatrolData{StandingTile, other});
}

TEST_CASE("A level refuses a beat that leaves the map", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile)});

    REQUIRE_THROWS(level.setNpcPatrol(0, PatrolData{StandingTile, glm::ivec2(-1, 0)}));
    REQUIRE_FALSE(level.getNpcs().front().patrol);
}

TEST_CASE("A level refuses to give a beat to an npc it does not have", "[Level]")
{
    Level level = levelPlacing({});

    REQUIRE_THROWS_WITH(
        level.setNpcPatrol(0, PatrolData{StandingTile, StandingTile}),
        "Cannot give a beat to an npc the level does not have");
}

TEST_CASE("An npc can be left with no beat at all", "[Level]")
{
    glm::ivec2 other(4, FloorRow - 1);
    Level level = levelPlacing({spawnAt("short", StandingTile)});
    level.setNpcPatrol(0, PatrolData{StandingTile, other});

    level.clearNpcPatrol(0);

    REQUIRE_FALSE(level.getNpcs().front().patrol);
    REQUIRE_FALSE(level.toLevelData().npcs.front().patrol);
}

TEST_CASE("A level refuses to clear the beat of an npc it does not have", "[Level]")
{
    Level level = levelPlacing({});

    REQUIRE_THROWS_WITH(
        level.clearNpcPatrol(0), "Cannot clear the beat of an npc the level does not have");
}

TEST_CASE("An npc of a type the level had none of still gets a graph", "[Level]")
{
    Level level = levelPlacing({});
    REQUIRE(level.getGraphs().size() == 1);

    level.addNpc(spawnAt("tall", StandingTile));

    REQUIRE_NOTHROW(level.graphForNpc(level.getNpcs().front()));
    REQUIRE(level.getGraphs().size() == 2);
}

TEST_CASE("A level refuses an npc of a type it has never heard of", "[Level]")
{
    Level level = levelPlacing({});

    REQUIRE_THROWS(level.addNpc(spawnAt("dragon", StandingTile)));
    REQUIRE(level.getNpcs().empty());
}

TEST_CASE("The run beneath an npc reaches both ends of the floor it stands on", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile)});

    std::optional<PatrolData> run = level.runBeneathNpc(0);

    REQUIRE(run);
    REQUIRE(run->from.y == FloorRow - 1);
    REQUIRE(run->to.y == FloorRow - 1);
    REQUIRE(run->from.x < StandingTile.x);
    REQUIRE(run->to.x > StandingTile.x);
}

TEST_CASE("The run beneath an npc names tiles the beat can be saved as", "[Level]")
{
    Level level = levelPlacing({spawnAt("short", StandingTile)});

    std::optional<PatrolData> run = level.runBeneathNpc(0);

    REQUIRE(run);
    REQUIRE_NOTHROW(level.setNpcPatrol(0, *run));
    REQUIRE(level.getNpcs().front().patrol == run);
}

TEST_CASE("A level refuses to look under an npc it does not have", "[Level]")
{
    Level level = levelPlacing({});

    REQUIRE_THROWS_WITH(level.runBeneathNpc(0), "Cannot look under an npc the level does not have");
}

TEST_CASE("A beat end picked on a ledge stays on that ledge", "[Level]")
{
    Level level = levelWithALedge({spawnAt("short", OnTheLedge)});
    const NpcSpawnData &spawn = level.getNpcs().front();

    for (int x = LedgeFirstTile; x <= LedgeLastTile; ++x)
        REQUIRE(level.beatEndAt(spawn, glm::ivec2(x, LedgeRow - 1)).y == LedgeRow - 1);
}

TEST_CASE("A beat end picked on a ledge names a tile of that ledge", "[Level]")
{
    Level level = levelWithALedge({spawnAt("short", OnTheLedge)});
    const NpcSpawnData &spawn = level.getNpcs().front();

    for (int x = LedgeFirstTile; x <= LedgeLastTile; ++x)
    {
        glm::ivec2 landed = level.beatEndAt(spawn, glm::ivec2(x, LedgeRow - 1));
        REQUIRE(landed.x >= LedgeFirstTile);
        REQUIRE(landed.x <= LedgeLastTile);
    }
}

TEST_CASE("A beat end keeps the tile that was picked", "[Level]")
{
    Level level = levelWithALedge({spawnAt("short", OnTheLedge)});
    const NpcSpawnData &spawn = level.getNpcs().front();

    for (int x = LedgeFirstTile; x <= LedgeLastTile; ++x)
    {
        glm::ivec2 clicked(x, LedgeRow - 1);
        REQUIRE(level.beatEndAt(spawn, clicked) == clicked);
    }
}

TEST_CASE("A beat end picked past the run stops at the end of it", "[Level]")
{
    Level level = levelWithALedge({spawnAt("short", OnTheLedge)});
    const NpcSpawnData &spawn = level.getNpcs().front();

    glm::ivec2 pastTheEnd(LedgeLastTile + 2, LedgeRow - 1);

    REQUIRE(level.beatEndAt(spawn, pastTheEnd) == glm::ivec2(LedgeLastTile, LedgeRow - 1));
}

TEST_CASE("A level refuses a beat end off the map", "[Level]")
{
    Level level = levelWithALedge({spawnAt("short", OnTheLedge)});

    REQUIRE_THROWS(level.beatEndAt(level.getNpcs().front(), glm::ivec2(-1, 0)));
}
