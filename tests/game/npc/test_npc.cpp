#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <unordered_set>
#include "game/npc/npc.hpp"
#include "game/tile_map/tile_map.hpp"
#include "test_helpers/test_tile_map_utils.hpp"

namespace
{
    NpcData setupNpcData()
    {
        NpcData npcData;
        npcData.agentData.size = glm::vec2(16.0f);
        npcData.agentData.moveAbilityData = MoveAbilityData{60.0f};
        npcData.agentData.gravityAbilityData = GravityAbilityData{};
        npcData.agentData.physicsBodyData.colliderSize = glm::vec2(8.0f, 13.0f);
        npcData.agentData.physicsBodyData.colliderOffset = glm::vec2(4.0f, 3.0f);
        npcData.patrolBehaviorData = PatrolBehaviorData();
        return npcData;
    }

    TileMap setupWalkableTileMap()
    {
        TileMap tileMap = setupTileMap();
        for (int x = 0; x < 10; ++x)
            tileMap.setTileIndex(glm::ivec2(x, 6), 1);
        tileMap.buildNavigationGraph();
        return tileMap;
    }

    glm::vec2 spawnPosition(const TileMap &tileMap)
    {
        return tileMap.tileToWorldPosition(glm::ivec2(4, 5));
    }

    void stepNpc(Npc &npc, const TileMap &tileMap, int steps)
    {
        for (int step = 0; step < steps; ++step)
        {
            npc.preFixedUpdate();
            npc.fixedUpdate(0.01f, tileMap);
        }
    }
} // namespace

TEST_CASE("Spawns where the level places it", "[Npc]")
{
    TileMap tileMap = setupWalkableTileMap();
    Npc npc(setupNpcData());

    glm::vec2 placed = spawnPosition(tileMap);
    npc.spawnAt(placed, tileMap.getNavigationGraph());

    REQUIRE(npc.getAgent().getState().position == placed);
}

TEST_CASE("Starts from the navigation node nearest where it was placed", "[Npc]")
{
    TileMap tileMap = setupWalkableTileMap();
    const NavigationGraph &navigationGraph = tileMap.getNavigationGraph();

    Npc npc(setupNpcData());
    glm::vec2 placed = spawnPosition(tileMap);
    npc.spawnAt(placed, navigationGraph);

    REQUIRE(npc.getCurrentNodeId().has_value());

    glm::vec2 footPosition = placed + npc.getFootOffset();
    float chosen = glm::distance(
        navigationGraph.getNode(*npc.getCurrentNodeId()).position,
        footPosition);

    for (const auto &[id, node] : navigationGraph.getNodes())
        REQUIRE(glm::distance(node.position, footPosition) >= chosen);
}

TEST_CASE("Two npcs placed apart start on different nodes", "[Npc]")
{
    TileMap tileMap = setupWalkableTileMap();

    Npc left(setupNpcData());
    Npc right(setupNpcData());
    left.spawnAt(tileMap.tileToWorldPosition(glm::ivec2(0, 5)), tileMap.getNavigationGraph());
    right.spawnAt(tileMap.tileToWorldPosition(glm::ivec2(9, 5)), tileMap.getNavigationGraph());

    REQUIRE(left.getCurrentNodeId() != right.getCurrentNodeId());
}

TEST_CASE("Walks along the graph without being told to", "[Npc]")
{
    TileMap tileMap = setupWalkableTileMap();
    Npc npc(setupNpcData());
    npc.spawnAt(spawnPosition(tileMap), tileMap.getNavigationGraph());

    float startX = npc.getAgent().getState().position.x;
    stepNpc(npc, tileMap, 200);

    REQUIRE(std::abs(npc.getAgent().getState().position.x - startX) > 1.0f);
}

TEST_CASE("Patrols between both ends of its platform", "[Npc]")
{
    TileMap tileMap = setupWalkableTileMap();
    Npc npc(setupNpcData());
    npc.spawnAt(spawnPosition(tileMap), tileMap.getNavigationGraph());

    std::unordered_set<int> visited;
    float lowestFootY = npc.getAgent().getState().position.y + npc.getFootOffset().y;

    for (int step = 0; step < 4000; ++step)
    {
        stepNpc(npc, tileMap, 1);
        visited.insert(*npc.getCurrentNodeId());
        lowestFootY = std::max(lowestFootY, npc.getAgent().getState().position.y + npc.getFootOffset().y);
    }

    REQUIRE(visited.size() == tileMap.getNavigationGraph().getNodes().size());

    REQUIRE(lowestFootY <= 6.0f * tileMap.getTileSize());

    glm::vec2 position = npc.getAgent().getState().position;
    REQUIRE(position.x >= -static_cast<float>(tileMap.getTileSize()));
    REQUIRE(position.x <= static_cast<float>(tileMap.getWorldWidth()));
}

TEST_CASE("Stands still on a tile map with no navigation graph", "[Npc]")
{
    TileMap tileMap = setupTileMap();
    for (int x = 0; x < 10; ++x)
        tileMap.setTileIndex(glm::ivec2(x, 6), 1);

    Npc npc(setupNpcData());
    npc.spawnAt(spawnPosition(tileMap), tileMap.getNavigationGraph());

    npc.setPosition(glm::vec2(48.0f, 64.0f));
    stepNpc(npc, tileMap, 100);

    REQUIRE(npc.getAgent().getState().position.x == 48.0f);
}

TEST_CASE("Patrolling is deterministic, so where you place them is what differs", "[Npc]")
{
    TileMap tileMap = setupWalkableTileMap();

    Npc first(setupNpcData());
    Npc second(setupNpcData());
    first.spawnAt(spawnPosition(tileMap), tileMap.getNavigationGraph());
    second.spawnAt(spawnPosition(tileMap), tileMap.getNavigationGraph());

    stepNpc(first, tileMap, 600);
    stepNpc(second, tileMap, 600);

    REQUIRE(first.getAgent().getState().position == second.getAgent().getState().position);
}

TEST_CASE("A level names the npcs it is populated with", "[Npc][Level]")
{
    TileMapData tileMapData;
    tileMapData.size = 16;
    tileMapData.width = 10;
    tileMapData.height = 10;
    TilePalette palette = getDefaultTileDataMap();
    tileMapData.npcs = {{"villager", {1, 1}}, {"villager", {2, 1}}};

    TileMap tileMap(tileMapData, palettesFrom(palette));

    REQUIRE(tileMap.getNpcs().size() == 2);
    REQUIRE(tileMap.getNpcs()[0].type == "villager");
    REQUIRE(tileMap.getNpcs()[0].tilePosition == glm::ivec2(1, 1));
    REQUIRE(tileMap.getNpcs()[1].tilePosition == glm::ivec2(2, 1));
    REQUIRE(tileMap.toTileMapData().npcs == tileMapData.npcs);
}

TEST_CASE("The shipped level6 has a graph an npc can wander", "[Npc][Level]")
{
    TileMap tileMap("../../assets/levels/level6.json", shippedPalettes());

    const NavigationGraph &navigationGraph = tileMap.getNavigationGraph();
    REQUIRE(navigationGraph.getNodes().size() > 2);
    REQUIRE_FALSE(navigationGraph.getEdges().empty());
    REQUIRE(tileMap.getNpcs().size() == 2);
    REQUIRE(tileMap.getNpcs()[0].type == "villager");
    REQUIRE(tileMap.getNpcs()[0].tilePosition == glm::ivec2(6, 11));

    Npc npc(setupNpcData());
    npc.spawnAt(tileMap.tileToWorldPosition(glm::ivec2(6, 11)), navigationGraph);
    REQUIRE(npc.getCurrentNodeId().has_value());

    float startX = npc.getAgent().getState().position.x;
    stepNpc(npc, tileMap, 400);

    REQUIRE(std::abs(npc.getAgent().getState().position.x - startX) > 1.0f);
}

TEST_CASE("A level rejects an npc placed somewhere it cannot stand", "[Npc][Level]")
{
    TileMapData tileMapData;
    tileMapData.size = 16;
    TilePalette palette = getDefaultTileDataMap();
    tileMapData.indices = std::vector<std::vector<int>>(10, std::vector<int>(10, 0));
    for (int x = 0; x < 10; ++x)
        (*tileMapData.indices)[6][x] = 1;

    SECTION("out of bounds")
    {
        tileMapData.npcs = {{"villager", {99, 99}}};
        REQUIRE_THROWS_WITH(TileMap(tileMapData, palettesFrom(palette)), "Npc start position is out of bounds");
    }

    SECTION("inside a solid tile")
    {
        tileMapData.npcs = {{"villager", {3, 6}}};
        REQUIRE_THROWS_WITH(TileMap(tileMapData, palettesFrom(palette)), "Npc start position is on a solid tile");
    }

    SECTION("somewhere it can stand")
    {
        tileMapData.npcs = {{"villager", {3, 5}}};
        REQUIRE_NOTHROW(TileMap(tileMapData, palettesFrom(palette)));
    }
}

TEST_CASE("An npc on the ground patrols the ground, not the platform above it", "[Npc][Level]")
{
    TileMap tileMap("../../assets/levels/level6.json", shippedPalettes());
    Npc npc(setupNpcData());
    npc.spawnAt(tileMap.tileToWorldPosition(glm::ivec2(6, 11)), tileMap.getNavigationGraph());

    REQUIRE(npc.getCurrentNodeId().has_value());

    glm::vec2 footPosition = npc.getAgent().getState().position + npc.getFootOffset();
    NavigationNode anchor = tileMap.getNavigationGraph().getNode(*npc.getCurrentNodeId());
    REQUIRE(anchor.position.y == footPosition.y);

    float lowest = footPosition.x;
    float highest = footPosition.x;
    for (int step = 0; step < 4000; ++step)
    {
        stepNpc(npc, tileMap, 1);
        float footX = npc.getAgent().getState().position.x + npc.getFootOffset().x;
        lowest = std::min(lowest, footX);
        highest = std::max(highest, footX);
    }

    // The floor spans roughly 288, the platform above it only 112.
    REQUIRE(highest - lowest > 200.0f);
}

TEST_CASE("Arrives at a node its collider cannot stand exactly on", "[Npc][Level]")
{
    TileMap tileMap("../../assets/levels/level6.json", shippedPalettes());
    Npc npc(setupNpcData());
    npc.spawnAt(tileMap.tileToWorldPosition(glm::ivec2(6, 11)), tileMap.getNavigationGraph());

    std::unordered_set<int> reached;
    for (int step = 0; step < 4000; ++step)
    {
        stepNpc(npc, tileMap, 1);
        reached.insert(*npc.getCurrentNodeId());
    }

    std::unordered_set<int> floorNodes;
    for (const auto &[id, node] : tileMap.getNavigationGraph().getNodes())
        if (node.position.y == 192.0f)
            floorNodes.insert(id);

    REQUIRE(reached == floorNodes);
}

TEST_CASE("An npc given no behavior data does nothing", "[Npc]")
{
    TileMap tileMap = setupWalkableTileMap();

    NpcData npcData = setupNpcData();
    npcData.patrolBehaviorData.reset();

    Npc npc(npcData);
    glm::vec2 placed = spawnPosition(tileMap);
    npc.spawnAt(placed, tileMap.getNavigationGraph());

    REQUIRE_FALSE(npc.getCurrentNodeId().has_value());
    REQUIRE_FALSE(npc.getTargetNodeId().has_value());

    stepNpc(npc, tileMap, 400);

    REQUIRE(npc.getAgent().getState().position.x == placed.x);
}
