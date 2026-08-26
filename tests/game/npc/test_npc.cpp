#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <cmath>
#include <filesystem>
#include <vector>
#include "game/npc/npc.hpp"
#include "game/tile_map/tile_map.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "test_helpers/asset_path.hpp"

namespace
{
    NpcData setupNpcData()
    {
        NpcData npcData;
        npcData.actorData.size = glm::vec2(16.0f);
        npcData.actorData.motionData.moveAbilityData = MoveAbilityData{60.0f};
        npcData.actorData.motionData.gravityAbilityData = GravityAbilityData{};
        npcData.actorData.physicsBodyData.colliderSize = glm::vec2(8.0f, 13.0f);
        npcData.actorData.physicsBodyData.colliderOffset = glm::vec2(4.0f, 3.0f);
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

    constexpr int TwoTierWidthTiles = 20;
    constexpr int TwoTierHeightTiles = 14;
    constexpr int FloorRow = 12;
    constexpr int PlatformRow = 8;
    constexpr int PlatformFirstTile = 3;
    constexpr int PlatformLastTile = 9;
    constexpr glm::ivec2 UnderThePlatform{6, FloorRow - 1};

    TileMap setupTwoTierTileMap()
    {
        TileMap tileMap = setupTileMap(TwoTierWidthTiles, TwoTierHeightTiles);

        for (int x = 0; x < TwoTierWidthTiles; ++x)
            tileMap.setTileIndex(glm::ivec2(x, FloorRow), 1);

        for (int x = PlatformFirstTile; x <= PlatformLastTile; ++x)
            tileMap.setTileIndex(glm::ivec2(x, PlatformRow), 1);

        tileMap.buildNavigationGraph();
        return tileMap;
    }

    float floorTopY(const TileMap &tileMap)
    {
        return static_cast<float>(FloorRow * tileMap.getTileSize());
    }

    float spanOf(int firstTile, int lastTile, const TileMap &tileMap)
    {
        return static_cast<float>((lastTile - firstTile + 1) * tileMap.getTileSize());
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

    float footX(const Npc &npc)
    {
        return npc.getPosition().x + npc.getPhysicsBody().getBottomCenterOffset().x;
    }

    float reachOf(const Npc &npc)
    {
        return npc.getPhysicsBody().getColliderSize().x * 0.5f + PatrolBehaviorData().arrivalThreshold;
    }

    std::vector<float> patrolFootXs(Npc &npc, const TileMap &tileMap, int steps)
    {
        std::vector<float> samples;
        for (int step = 0; step < steps; ++step)
        {
            stepNpc(npc, tileMap, 1);
            samples.push_back(footX(npc));
        }
        return samples;
    }

    bool cameWithin(const std::vector<float> &samples, float x, float reach)
    {
        for (float sample : samples)
            if (std::abs(sample - x) <= reach)
                return true;

        return false;
    }
}

TEST_CASE("Spawns where the level places it", "[Npc]")
{
    TileMap tileMap = setupWalkableTileMap();
    Npc npc(setupNpcData());

    glm::vec2 placed = spawnPosition(tileMap);
    npc.setPosition(placed);

    REQUIRE(npc.getPosition() == placed);
}

TEST_CASE("Where an npc is placed decides which way it sets off", "[Npc]")
{
    TileMap tileMap = setupWalkableTileMap();

    Npc left(setupNpcData());
    Npc right(setupNpcData());
    left.setPosition(tileMap.tileToWorldPosition(glm::ivec2(0, 5)));
    right.setPosition(tileMap.tileToWorldPosition(glm::ivec2(9, 5)));

    float leftStartX = footX(left);
    float rightStartX = footX(right);
    stepNpc(left, tileMap, 100);
    stepNpc(right, tileMap, 100);

    REQUIRE(footX(left) > leftStartX);
    REQUIRE(footX(right) < rightStartX);
}

TEST_CASE("Patrols between both ends of its platform", "[Npc]")
{
    TileMap tileMap = setupWalkableTileMap();
    Npc npc(setupNpcData());
    npc.setPosition(spawnPosition(tileMap));

    float lowestFootY = npc.getPosition().y + npc.getPhysicsBody().getBottomCenterOffset().y;
    std::vector<float> footXs;

    for (int step = 0; step < 4000; ++step)
    {
        stepNpc(npc, tileMap, 1);
        footXs.push_back(footX(npc));
        lowestFootY = std::max(lowestFootY, npc.getPosition().y + npc.getPhysicsBody().getBottomCenterOffset().y);
    }

    for (const auto &[id, node] : tileMap.getNavigationGraph().getNodes())
        REQUIRE(cameWithin(footXs, node.position.x, reachOf(npc)));

    REQUIRE(lowestFootY <= 6.0f * tileMap.getTileSize());

    glm::vec2 position = npc.getPosition();
    REQUIRE(position.x >= -static_cast<float>(tileMap.getTileSize()));
    REQUIRE(position.x <= static_cast<float>(tileMap.getWorldWidth()));
}

TEST_CASE("Stands still on a tile map with no navigation graph", "[Npc]")
{
    TileMap tileMap = setupTileMap();
    for (int x = 0; x < 10; ++x)
        tileMap.setTileIndex(glm::ivec2(x, 6), 1);

    Npc npc(setupNpcData());
    npc.setPosition(spawnPosition(tileMap));

    npc.setPosition(glm::vec2(48.0f, 64.0f));
    stepNpc(npc, tileMap, 100);

    REQUIRE(npc.getPosition().x == 48.0f);
}

TEST_CASE("Patrolling is deterministic, so where you place them is what differs", "[Npc]")
{
    TileMap tileMap = setupWalkableTileMap();

    Npc first(setupNpcData());
    Npc second(setupNpcData());
    first.setPosition(spawnPosition(tileMap));
    second.setPosition(spawnPosition(tileMap));

    stepNpc(first, tileMap, 600);
    stepNpc(second, tileMap, 600);

    REQUIRE(first.getPosition() == second.getPosition());
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

TEST_CASE("Every npc a shipped level places has somewhere to walk", "[Npc][Level]")
{
    int placed = 0;
    for (const auto &entry : std::filesystem::directory_iterator(assetPath("levels")))
    {
        if (entry.path().extension() != ".json")
            continue;

        TileMap tileMap(entry.path().string(), shippedPalettes());
        for (const NpcSpawnData &spawn : tileMap.getNpcs())
        {
            ++placed;
            INFO("npc \"" << spawn.type << "\" at " << spawn.tilePosition.x << "," << spawn.tilePosition.y
                          << " in " << entry.path().filename().string() << " has nowhere to walk");

            Npc npc(setupNpcData());
            npc.setPosition(tileMap.tileToWorldPosition(spawn.tilePosition));

            float startX = npc.getPosition().x;
            stepNpc(npc, tileMap, 400);

            REQUIRE(std::abs(npc.getPosition().x - startX) > 1.0f);
        }
    }

    REQUIRE(placed > 0);
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

TEST_CASE("An npc on the ground patrols the ground, not the platform above it", "[Npc]")
{
    TileMap tileMap = setupTwoTierTileMap();
    Npc npc(setupNpcData());
    npc.setPosition(tileMap.tileToWorldPosition(UnderThePlatform));

    float lowest = footX(npc);
    float highest = footX(npc);
    for (int step = 0; step < 4000; ++step)
    {
        stepNpc(npc, tileMap, 1);
        lowest = std::min(lowest, footX(npc));
        highest = std::max(highest, footX(npc));
    }

    float platformSpan = spanOf(PlatformFirstTile, PlatformLastTile, tileMap);
    float floorSpan = spanOf(0, TwoTierWidthTiles - 1, tileMap);
    REQUIRE(highest - lowest > (platformSpan + floorSpan) * 0.5f);
}

TEST_CASE("Arrives at a node its collider cannot stand exactly on", "[Npc]")
{
    TileMap tileMap = setupTwoTierTileMap();
    Npc npc(setupNpcData());
    npc.setPosition(tileMap.tileToWorldPosition(UnderThePlatform));

    std::vector<float> footXs = patrolFootXs(npc, tileMap, 4000);

    int floorNodes = 0;
    for (const auto &[id, node] : tileMap.getNavigationGraph().getNodes())
    {
        if (node.position.y != floorTopY(tileMap))
            continue;

        ++floorNodes;
        REQUIRE(cameWithin(footXs, node.position.x, reachOf(npc)));
    }

    REQUIRE(floorNodes > 1);
}

TEST_CASE("An npc given no behavior data does nothing", "[Npc]")
{
    TileMap tileMap = setupWalkableTileMap();

    NpcData npcData = setupNpcData();
    npcData.patrolBehaviorData.reset();

    Npc npc(npcData);
    glm::vec2 placed = spawnPosition(tileMap);
    npc.setPosition(placed);

    stepNpc(npc, tileMap, 400);

    REQUIRE(npc.getPosition().x == placed.x);
}
