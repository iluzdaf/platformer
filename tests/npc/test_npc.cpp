#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <cmath>
#include <filesystem>
#include <set>
#include <vector>
#include <glaze/glaze.hpp>
#include "npc/npc.hpp"
#include "tile_map/tile_map.hpp"
#include "game/level.hpp"
#include "game/level_data.hpp"
#include "game/game_data.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "test_helpers/asset_path.hpp"

namespace
{
    NpcSpawnData spawnAt(std::string type, glm::ivec2 tilePosition)
    {
        NpcSpawnData spawn;
        spawn.type = std::move(type);
        spawn.tilePosition = tilePosition;
        return spawn;
    }

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

    std::unordered_map<std::string, NpcData> npcCatalogue()
    {
        return {{"villager", setupNpcData()}};
    }

    Level levelOf(TileMap &tileMap, glm::ivec2 npcTile)
    {
        LevelData levelData;
        levelData.tileMapData = tileMap.toTileMapData();
        levelData.npcs = {spawnAt("villager", npcTile)};
        return Level(
            levelData,
            palettesFrom(getDefaultTileDataMap()),
            PlayerData(),
            npcCatalogue());
    }

    Level setupWalkableLevel()
    {
        TileMap tileMap = setupTileMap();
        for (int x = 0; x < 10; ++x)
            tileMap.setTileIndex(glm::ivec2(x, 6), 1);
        return levelOf(tileMap, glm::ivec2(4, 5));
    }

    constexpr int TwoTierWidthTiles = 20;
    constexpr int TwoTierHeightTiles = 14;
    constexpr int FloorRow = 12;
    constexpr int PlatformRow = 8;
    constexpr int PlatformFirstTile = 3;
    constexpr int PlatformLastTile = 9;
    constexpr glm::ivec2 UnderThePlatform{6, FloorRow - 1};

    Level setupTwoTierLevel()
    {
        TileMap tileMap = setupTileMap(TwoTierWidthTiles, TwoTierHeightTiles);

        for (int x = 0; x < TwoTierWidthTiles; ++x)
            tileMap.setTileIndex(glm::ivec2(x, FloorRow), 1);

        for (int x = PlatformFirstTile; x <= PlatformLastTile; ++x)
            tileMap.setTileIndex(glm::ivec2(x, PlatformRow), 1);

        return levelOf(tileMap, UnderThePlatform);
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

    void stepNpc(Npc &npc, const Level &level, int steps)
    {
        for (int step = 0; step < steps; ++step)
        {
            npc.preFixedUpdate();
            npc.fixedUpdate(0.01f, level);
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

    std::vector<float> patrolFootXs(Npc &npc, const Level &level, int steps)
    {
        std::vector<float> samples;
        for (int step = 0; step < steps; ++step)
        {
            stepNpc(npc, level, 1);
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
    Level level = setupWalkableLevel();
    const TileMap &tileMap = level.getTileMap();
    Npc npc(setupNpcData());

    glm::vec2 placed = spawnPosition(tileMap);
    npc.setPosition(placed);

    REQUIRE(npc.getPosition() == placed);
}

TEST_CASE("Where an npc is placed decides which way it sets off", "[Npc]")
{
    Level level = setupWalkableLevel();
    const TileMap &tileMap = level.getTileMap();

    Npc left(setupNpcData());
    Npc right(setupNpcData());
    left.setPosition(tileMap.tileToWorldPosition(glm::ivec2(0, 5)));
    right.setPosition(tileMap.tileToWorldPosition(glm::ivec2(9, 5)));

    float leftStartX = footX(left);
    float rightStartX = footX(right);
    stepNpc(left, level, 100);
    stepNpc(right, level, 100);

    REQUIRE(footX(left) > leftStartX);
    REQUIRE(footX(right) < rightStartX);
}

TEST_CASE("Patrols between both ends of its platform", "[Npc]")
{
    Level level = setupWalkableLevel();
    const TileMap &tileMap = level.getTileMap();
    Npc npc(setupNpcData());
    npc.setPosition(spawnPosition(tileMap));

    float lowestFootY = npc.getPosition().y + npc.getPhysicsBody().getBottomCenterOffset().y;
    std::vector<float> footXs;

    for (int step = 0; step < 4000; ++step)
    {
        stepNpc(npc, level, 1);
        footXs.push_back(footX(npc));
        lowestFootY = std::max(lowestFootY, npc.getPosition().y + npc.getPhysicsBody().getBottomCenterOffset().y);
    }

    for (const auto &[id, node] : level.graphFor(npc.getNavigationProfile()).getNodes())
        REQUIRE(cameWithin(footXs, node.position.x, reachOf(npc)));

    REQUIRE(lowestFootY <= 6.0f * tileMap.getTileSize());

    glm::vec2 position = npc.getPosition();
    REQUIRE(position.x >= -static_cast<float>(tileMap.getTileSize()));
    REQUIRE(position.x <= static_cast<float>(tileMap.getWorldWidth()));
}

TEST_CASE("Stands still in a level with nothing to walk on", "[Npc]")
{
    TileMap tiles = setupTileMap();
    Level level = levelOf(tiles, glm::ivec2(3, 4));

    Npc npc(setupNpcData());
    npc.setPosition(glm::vec2(48.0f, 64.0f));
    stepNpc(npc, level, 100);

    REQUIRE(npc.getPosition().x == 48.0f);
}
TEST_CASE("Patrolling is deterministic, so where you place them is what differs", "[Npc]")
{
    Level level = setupWalkableLevel();
    const TileMap &tileMap = level.getTileMap();

    Npc first(setupNpcData());
    Npc second(setupNpcData());
    first.setPosition(spawnPosition(tileMap));
    second.setPosition(spawnPosition(tileMap));

    stepNpc(first, level, 600);
    stepNpc(second, level, 600);

    REQUIRE(first.getPosition() == second.getPosition());
}

TEST_CASE("A level names the npcs it is populated with", "[Npc][Level]")
{
    LevelData levelData;
    levelData.tileMapData.size = 16;
    levelData.tileMapData.width = 10;
    levelData.tileMapData.height = 10;
    levelData.npcs = {spawnAt("villager", {1, 1}), spawnAt("villager", {2, 1})};

    Level level(
        levelData,
        palettesFrom(getDefaultTileDataMap()),
        PlayerData(),
        npcCatalogue());

    REQUIRE(level.getNpcs().size() == 2);
    REQUIRE(level.getNpcs()[0].type == "villager");
    REQUIRE(level.getNpcs()[0].tilePosition == glm::ivec2(1, 1));
    REQUIRE(level.getNpcs()[1].tilePosition == glm::ivec2(2, 1));
    REQUIRE(level.toLevelData().npcs == levelData.npcs);
}

TEST_CASE("Every npc a shipped level places has somewhere to walk", "[Npc][Level]")
{
    int placed = 0;
    for (const auto &entry : std::filesystem::directory_iterator(assetPath("levels")))
    {
        if (entry.path().extension() != ".json")
            continue;

        Level level(entry.path().string(), shippedPalettes(), PlayerData(), shippedNpcData());
        const TileMap &tileMap = level.getTileMap();
        for (const NpcSpawnData &spawn : level.getNpcs())
        {
            ++placed;
            INFO("npc \"" << spawn.type << "\" at " << spawn.tilePosition.x << "," << spawn.tilePosition.y
                          << " in " << entry.path().filename().string() << " has nowhere to walk");

            Npc npc(setupNpcData());
            npc.setPosition(tileMap.tileToWorldPosition(spawn.tilePosition));

            float startX = npc.getPosition().x;
            stepNpc(npc, level, 400);

            REQUIRE(std::abs(npc.getPosition().x - startX) > 1.0f);
        }
    }

    REQUIRE(placed > 0);
}

TEST_CASE("A level rejects an npc placed somewhere it cannot stand", "[Npc][Level]")
{
    LevelData levelData;
    levelData.tileMapData.size = 16;
    TilePalette palette = getDefaultTileDataMap();
    levelData.tileMapData.indices = std::vector<std::vector<int>>(10, std::vector<int>(10, 0));
    for (int x = 0; x < 10; ++x)
        (*levelData.tileMapData.indices)[6][x] = 1;

    auto levelWith = [&](std::vector<NpcSpawnData> npcs)
    {
        levelData.npcs = std::move(npcs);
        return Level(levelData, palettesFrom(palette), PlayerData(), npcCatalogue());
    };

    SECTION("out of bounds")
    {
        REQUIRE_THROWS_WITH(levelWith({spawnAt("villager", {99, 99})}), "Npc start position is out of bounds");
    }

    SECTION("inside a solid tile")
    {
        REQUIRE_THROWS_WITH(levelWith({spawnAt("villager", {3, 6})}), "Npc start position is on a solid tile");
    }

    SECTION("somewhere it can stand")
    {
        REQUIRE_NOTHROW(levelWith({spawnAt("villager", {3, 5})}));
    }
}

TEST_CASE("An npc on the ground patrols the ground, not the platform above it", "[Npc]")
{
    Level level = setupTwoTierLevel();
    const TileMap &tileMap = level.getTileMap();
    Npc npc(setupNpcData());
    npc.setPosition(tileMap.tileToWorldPosition(UnderThePlatform));

    float lowest = footX(npc);
    float highest = footX(npc);
    for (int step = 0; step < 4000; ++step)
    {
        stepNpc(npc, level, 1);
        lowest = std::min(lowest, footX(npc));
        highest = std::max(highest, footX(npc));
    }

    float platformSpan = spanOf(PlatformFirstTile, PlatformLastTile, tileMap);
    float floorSpan = spanOf(0, TwoTierWidthTiles - 1, tileMap);
    REQUIRE(highest - lowest > (platformSpan + floorSpan) * 0.5f);
}

TEST_CASE("Arrives at a node its collider cannot stand exactly on", "[Npc]")
{
    Level level = setupTwoTierLevel();
    const TileMap &tileMap = level.getTileMap();
    Npc npc(setupNpcData());
    npc.setPosition(tileMap.tileToWorldPosition(UnderThePlatform));

    std::vector<float> footXs = patrolFootXs(npc, level, 4000);

    int floorNodes = 0;
    for (const auto &[id, node] : level.graphFor(npc.getNavigationProfile()).getNodes())
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
    Level level = setupWalkableLevel();
    const TileMap &tileMap = level.getTileMap();

    NpcData npcData = setupNpcData();
    npcData.patrolBehaviorData.reset();

    Npc npc(npcData);
    glm::vec2 placed = spawnPosition(tileMap);
    npc.setPosition(placed);

    stepNpc(npc, level, 400);

    REQUIRE(npc.getPosition().x == placed.x);
}

TEST_CASE("The shipped explorer patrols level6 without getting stuck", "[Npc][Level]")
{
    GameData gameData;
    REQUIRE_FALSE(glz::read_file_json(gameData, assetPath("game_data.json"), std::string{}));
    LevelData levelData;
    REQUIRE_FALSE(glz::read_file_json(levelData, assetPath("levels/level6.json"), std::string{}));

    Level level(levelData, gameData.tilePalettes, gameData.playerData, gameData.npcData);

    // On the left middle platform directly under the top one, so its first jump
    // has a ceiling close above its head. Where the level happens to spawn it is
    // a level editing decision, but this is the corner it has to cope with.
    // Built the way the game builds it, patrol and all, since where it walks
    // is the level's to say now.
    const NpcSpawnData &spawn = level.getNpcs().at(2);
    REQUIRE(spawn.type == "explorer");
    REQUIRE(spawn.patrol);

    Npc npc(gameData.npcData.at("explorer"), level.patrolFor(spawn));
    npc.setPosition(level.getTileMap().tileToWorldPosition(spawn.tilePosition));

    std::set<int> surfacesStoodOn;
    float previousX = npc.getPosition().x;
    int standingStill = 0;
    int longestStandingStill = 0;

    for (int step = 0; step < 4000; ++step)
    {
        npc.preFixedUpdate();
        npc.fixedUpdate(0.01f, level);

        if (!npc.getMotion().getState().contacts.onGround)
            continue;

        surfacesStoodOn.insert(static_cast<int>(npc.getPosition().y));
        standingStill = std::abs(npc.getPosition().x - previousX) < 0.01f ? standingStill + 1 : 0;
        longestStandingStill = std::max(longestStandingStill, standingStill);
        previousX = npc.getPosition().x;
    }

    INFO("stood on " << surfacesStoodOn.size() << " surfaces, idle for up to "
                     << longestStandingStill << " steps");
    REQUIRE(surfacesStoodOn.size() >= 2);
    REQUIRE(longestStandingStill < 100);
}
