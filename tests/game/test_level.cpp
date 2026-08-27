#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include "game/level.hpp"
#include "game/level_data.hpp"
#include "player/player_data.hpp"
#include "test_helpers/test_tile_map_utils.hpp"

namespace
{
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

    std::unordered_map<std::string, NpcData> theUsualNpcs()
    {
        return {
            {"short", npcOfHeight(13.0f)},
            {"tall", npcOfHeight(20.0f)},
            {"alsoShort", npcOfHeight(13.0f)}};
    }

    NavigationProfile profileOfHeight(float height)
    {
        return NavigationProfile{glm::vec2(8.0f, height), {}};
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
    Level level = levelPlacing({{"short", StandingTile}});

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
    Level level = levelPlacing({{"short", StandingTile}, {"tall", StandingTile}});

    const NavigationGraph &shortGraph = level.graphFor(profileOfHeight(13.0f));
    const NavigationGraph &tallGraph = level.graphFor(profileOfHeight(20.0f));

    REQUIRE(&shortGraph != &tallGraph);
    REQUIRE(nodesOnTheFloor(shortGraph) == 2);
    REQUIRE(nodesOnTheFloor(tallGraph) == 0);
}

TEST_CASE("A level naming an npc that does not exist fails to load", "[Level]")
{
    REQUIRE_THROWS_WITH(
        levelPlacing({{"nobody", StandingTile}}),
        Catch::Matchers::ContainsSubstring("nobody") &&
            Catch::Matchers::ContainsSubstring("new_level.json"));
}

TEST_CASE("Editing the tiles changes what the graphs describe", "[Level]")
{
    Level level = levelPlacing({{"short", StandingTile}});
    NavigationProfile walker = profileOfHeight(13.0f);

    REQUIRE(nodesOnTheFloor(level.graphFor(walker)) == 2);

    level.getTileMap().setTileIndex(glm::ivec2(5, FloorRow), 0);
    level.rebuildGraphs();

    REQUIRE(nodesOnTheFloor(level.graphFor(walker)) == 4);
}

TEST_CASE("Rebuilding keeps a graph for every profile it had", "[Level]")
{
    Level level = levelPlacing({{"short", StandingTile}, {"tall", StandingTile}});
    size_t before = level.getGraphs().size();

    level.rebuildGraphs();

    REQUIRE(level.getGraphs().size() == before);
    REQUIRE(nodesOnTheFloor(level.graphFor(profileOfHeight(13.0f))) == 2);
    REQUIRE(nodesOnTheFloor(level.graphFor(profileOfHeight(20.0f))) == 0);
}

TEST_CASE("A graph is named for the actor that needed it", "[Level]")
{
    Level level = levelPlacing({{"tall", StandingTile}});

    std::vector<std::string> names;
    for (const NamedNavigationGraph &graph : level.getGraphs())
        names.push_back(graph.name);

    REQUIRE(std::find(names.begin(), names.end(), "player") != names.end());
    REQUIRE(std::find(names.begin(), names.end(), "tall") != names.end());
}

TEST_CASE("Actors that navigate alike share a graph, and it says so", "[Level]")
{
    Level level = levelPlacing({{"short", StandingTile}, {"alsoShort", StandingTile}});

    std::vector<std::string> names;
    for (const NamedNavigationGraph &graph : level.getGraphs())
        names.push_back(graph.name);

    REQUIRE(names.size() == 1);
    REQUIRE(names.front() == "player, short, alsoShort");
}

TEST_CASE("A level can be pointed at a different level next", "[Level]")
{
    Level level = levelPlacing({});

    level.setNextLevel("../../assets/levels/level4.json");

    REQUIRE(level.getNextLevel() == "../../assets/levels/level4.json");
    REQUIRE(level.toLevelData().nextLevel == "../../assets/levels/level4.json");
}

TEST_CASE("A level refuses to have no level next", "[Level]")
{
    Level level = levelPlacing({});

    REQUIRE_THROWS(level.setNextLevel(""));
    REQUIRE_FALSE(level.getNextLevel().empty());
}

TEST_CASE("A graph does not name the same actor twice", "[Level]")
{
    Level level = levelPlacing({{"short", StandingTile}, {"short", {2, FloorRow - 1}}});

    for (const NamedNavigationGraph &graph : level.getGraphs())
        REQUIRE(graph.name == "player, short");
}
