#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include "game/level.hpp"
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
            {"alsoShort", npcOfHeight(13.0f)},
            {"tall", npcOfHeight(20.0f)}};
    }

    TileMapData corridorPlacing(const std::vector<NpcSpawnData> &npcs)
    {
        TileMapData tileMapData;
        tileMapData.size = 16;
        tileMapData.indices =
            std::vector<std::vector<int>>(MapTiles, std::vector<int>(MapTiles, 0));
        for (int x = 0; x < MapTiles; ++x)
        {
            (*tileMapData.indices)[FloorRow][x] = 1;
            (*tileMapData.indices)[CeilingRow][x] = 1;
        }
        tileMapData.npcs = npcs;
        return tileMapData;
    }

    Level levelPlacing(const std::vector<NpcSpawnData> &npcs)
    {
        return Level(
            corridorPlacing(npcs),
            palettesFrom(getDefaultTileDataMap()),
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

    const NavigationGraph &graph = level.graphFor("short");

    REQUIRE(nodesOnTheFloor(graph) == 2);
    REQUIRE_FALSE(graph.getEdges().empty());
}

TEST_CASE("A level has no graph for an npc it does not place", "[Level]")
{
    Level level = levelPlacing({});

    REQUIRE_THROWS_WITH(level.graphFor("short"), Catch::Matchers::ContainsSubstring("short"));
}

TEST_CASE("Npcs that move alike share one graph", "[Level]")
{
    Level level = levelPlacing({{"short", StandingTile}, {"alsoShort", StandingTile}});

    REQUIRE(&level.graphFor("short") == &level.graphFor("alsoShort"));
}

TEST_CASE("Npcs that move differently get their own graphs", "[Level]")
{
    Level level = levelPlacing({{"short", StandingTile}, {"tall", StandingTile}});

    const NavigationGraph &shortGraph = level.graphFor("short");
    const NavigationGraph &tallGraph = level.graphFor("tall");

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
