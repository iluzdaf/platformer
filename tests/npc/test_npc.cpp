#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <cmath>
#include <filesystem>
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
        BehaviorStateData patrolling;
        patrolling.name = "patrol";
        patrolling.patrolBehaviorData = PatrolBehaviorData();
        npcData.stateMachineBehaviorData = StateMachineBehaviorData{{patrolling}, {}};
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
    constexpr glm::ivec2 SpawnTile{4, 5};

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

    void standIn(Npc &npc, const TileMap &tileMap, glm::ivec2 tilePosition)
    {
        npc.setPosition(
            tileMap.tileToBottomCenterPosition(tilePosition) -
            npc.getPhysicsBody().getBottomCenterOffset());
    }

    void stepNpc(Npc &npc, const Level &level, int steps)
    {
        for (int step = 0; step < steps; ++step)
        {
            npc.preFixedUpdate();
            npc.fixedUpdate(0.01f, level);
        }
    }

    glm::vec2 footOf(const Npc &npc)
    {
        return npc.getPhysicsBody().getAABB().bottomCenter();
    }

    float footX(const Npc &npc)
    {
        return footOf(npc).x;
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

    standIn(npc, tileMap, SpawnTile);

    REQUIRE(footOf(npc) == tileMap.tileToBottomCenterPosition(SpawnTile));
}

TEST_CASE("Where an npc is placed decides which way it sets off", "[Npc]")
{
    Level level = setupWalkableLevel();
    const TileMap &tileMap = level.getTileMap();

    Npc left(setupNpcData());
    Npc right(setupNpcData());
    standIn(left, tileMap, glm::ivec2(0, 5));
    standIn(right, tileMap, glm::ivec2(9, 5));

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
    standIn(npc, tileMap, SpawnTile);

    float lowestFootY = footOf(npc).y;
    std::vector<float> footXs;

    for (int step = 0; step < 4000; ++step)
    {
        stepNpc(npc, level, 1);
        footXs.push_back(footX(npc));
        lowestFootY = std::max(lowestFootY, footOf(npc).y);
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
    standIn(first, tileMap, SpawnTile);
    standIn(second, tileMap, SpawnTile);

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
            standIn(npc, tileMap, spawn.tilePosition);

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
    standIn(npc, tileMap, UnderThePlatform);

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
    standIn(npc, tileMap, UnderThePlatform);

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
    npcData.stateMachineBehaviorData.reset();

    Npc npc(npcData);
    standIn(npc, tileMap, SpawnTile);

    stepNpc(npc, level, 400);

    REQUIRE(footOf(npc).x == tileMap.tileToBottomCenterPosition(SpawnTile).x);
}

TEST_CASE("The shipped explorer walks level6 from the floor to the top and back",
          "[Npc][Level]")
{
    GameData gameData;
    REQUIRE_FALSE(glz::read_file_json(gameData, assetPath("game_data.json"), std::string{}));
    LevelData levelData;
    REQUIRE_FALSE(glz::read_file_json(levelData, assetPath("levels/level6.json"), std::string{}));

    Level level(levelData, gameData.tilePalettes, gameData.playerData, gameData.npcData);

    const NpcSpawnData &spawn = level.getNpcs().at(2);
    REQUIRE(spawn.type == "explorer");
    REQUIRE(spawn.patrol);

    Npc npc(gameData.npcData.at("explorer"), level.patrolFor(spawn));
    standIn(npc, level.getTileMap(), spawn.tilePosition);

    constexpr float TopPlatform = 96.0f;
    constexpr float Floor = 192.0f;

    bool startedOnTheFloor = false, reachedTheTop = false, cameBackDown = false;
    float previousX = npc.getPosition().x;
    int standingStill = 0, longestStandingStill = 0;

    for (int step = 0; step < 4000; ++step)
    {
        npc.preFixedUpdate();
        npc.fixedUpdate(0.01f, level);

        if (!npc.getMotion().getState().contacts.onGround)
            continue;

        float foot = npc.getPosition().y + 16.0f;
        if (!reachedTheTop)
            startedOnTheFloor = startedOnTheFloor || foot >= Floor;
        if (startedOnTheFloor && std::abs(foot - TopPlatform) < 1.0f)
            reachedTheTop = true;
        if (reachedTheTop && foot >= Floor)
            cameBackDown = true;

        standingStill = std::abs(npc.getPosition().x - previousX) < 0.01f ? standingStill + 1 : 0;
        longestStandingStill = std::max(longestStandingStill, standingStill);
        previousX = npc.getPosition().x;
    }

    REQUIRE(startedOnTheFloor);
    REQUIRE(reachedTheTop);
    REQUIRE(cameBackDown);
    REQUIRE(longestStandingStill < 100);
}

TEST_CASE("The shipped villager runs from the player and settles once it is gone",
          "[Npc][Level]")
{
    GameData gameData;
    REQUIRE_FALSE(glz::read_file_json(gameData, assetPath("game_data.json"), std::string{}));
    LevelData levelData;
    REQUIRE_FALSE(glz::read_file_json(levelData, assetPath("levels/level6.json"), std::string{}));

    Level level(levelData, gameData.tilePalettes, gameData.playerData, gameData.npcData);

    const NpcSpawnData &spawn = level.getNpcs().at(0);
    REQUIRE(spawn.type == "villager");

    Npc npc(gameData.npcData.at("villager"), level.patrolFor(spawn));
    standIn(npc, level.getTileMap(), spawn.tilePosition);

    glm::vec2 crowding = footOf(npc) + glm::vec2(12.0f, 0.0f);
    float startedAt = footOf(npc).x;

    for (int step = 0; step < 300; ++step)
    {
        npc.preFixedUpdate();
        npc.fixedUpdate(0.01f, level, crowding);
    }

    REQUIRE(footOf(npc).x < startedAt);
    REQUIRE(glm::distance(footOf(npc), crowding) > 40.0f);

    float ranTo = footOf(npc).x;
    for (int step = 0; step < 600; ++step)
    {
        npc.preFixedUpdate();
        npc.fixedUpdate(0.01f, level);
    }

    REQUIRE(footOf(npc).x != ranTo);
}

TEST_CASE("The shipped villager never freezes out in the open on its platform",
          "[Npc][Level]")
{
    GameData gameData;
    REQUIRE_FALSE(glz::read_file_json(gameData, assetPath("game_data.json"), std::string{}));
    LevelData levelData;
    REQUIRE_FALSE(glz::read_file_json(levelData, assetPath("levels/level6.json"), std::string{}));

    Level level(levelData, gameData.tilePalettes, gameData.playerData, gameData.npcData);

    const NpcSpawnData &spawn = level.getNpcs().at(1);
    REQUIRE(spawn.type == "villager");

    Npc npc(gameData.npcData.at("villager"), level.patrolFor(spawn));
    standIn(npc, level.getTileMap(), spawn.tilePosition);

    constexpr float LeftEnd = 16.0f, RightEnd = 112.0f;
    auto outInTheOpen = [](float x)
    {
        return std::min(std::abs(x - LeftEnd), std::abs(x - RightEnd)) > 10.0f;
    };

    glm::vec2 chasing = footOf(npc) + glm::vec2(8.0f, 0.0f);
    float previousX = footOf(npc).x;
    int standingStill = 0, longestOutInTheOpen = 0;

    for (int step = 0; step < 500; ++step)
    {
        npc.preFixedUpdate();
        npc.fixedUpdate(0.01f, level, chasing);

        chasing.x = std::max(16.0f, chasing.x - 1.1f);

        standingStill = std::abs(footOf(npc).x - previousX) < 0.01f ? standingStill + 1 : 0;
        if (outInTheOpen(footOf(npc).x))
            longestOutInTheOpen = std::max(longestOutInTheOpen, standingStill);
        previousX = footOf(npc).x;
    }

    REQUIRE(longestOutInTheOpen < 100);
}

TEST_CASE("The shipped villager holds its ground while the player shares its platform",
          "[Npc][Level]")
{
    GameData gameData;
    REQUIRE_FALSE(glz::read_file_json(gameData, assetPath("game_data.json"), std::string{}));
    LevelData levelData;
    REQUIRE_FALSE(glz::read_file_json(levelData, assetPath("levels/level6.json"), std::string{}));

    Level level(levelData, gameData.tilePalettes, gameData.playerData, gameData.npcData);

    const NpcSpawnData &spawn = level.getNpcs().at(1);
    REQUIRE(spawn.type == "villager");

    Npc npc(gameData.npcData.at("villager"), level.patrolFor(spawn));
    standIn(npc, level.getTileMap(), spawn.tilePosition);

    glm::vec2 cornering(112.0f, 96.0f);
    for (int step = 0; step < 600; ++step)
    {
        npc.preFixedUpdate();
        npc.fixedUpdate(0.01f, level, cornering);
    }

    float cowering = footOf(npc).x;
    REQUIRE(cowering < 32.0f);

    float wandered = cowering;
    for (int step = 0; step < 400; ++step)
    {
        npc.preFixedUpdate();
        npc.fixedUpdate(0.01f, level, cornering);
        wandered = std::max(wandered, footOf(npc).x);
    }

    REQUIRE(wandered < 48.0f);

    for (int step = 0; step < 600; ++step)
    {
        npc.preFixedUpdate();
        npc.fixedUpdate(0.01f, level, glm::vec2(112.0f, 192.0f));
    }

    REQUIRE(footOf(npc).x > cowering + 16.0f);
}

TEST_CASE("The shipped villager does not shuffle on the spot once it is cornered",
          "[Npc][Level]")
{
    GameData gameData;
    REQUIRE_FALSE(glz::read_file_json(gameData, assetPath("game_data.json"), std::string{}));
    LevelData levelData;
    REQUIRE_FALSE(glz::read_file_json(levelData, assetPath("levels/level6.json"), std::string{}));

    Level level(levelData, gameData.tilePalettes, gameData.playerData, gameData.npcData);

    const NpcSpawnData &spawn = level.getNpcs().at(1);
    REQUIRE(spawn.type == "villager");

    Npc npc(gameData.npcData.at("villager"), level.patrolFor(spawn));
    standIn(npc, level.getTileMap(), spawn.tilePosition);

    glm::vec2 driving(8.0f, 96.0f);
    int flips = 0;
    bool wasFacingLeft = npc.getState().facingLeft;

    for (int step = 0; step < 600; ++step)
    {
        npc.preFixedUpdate();
        npc.fixedUpdate(0.01f, level, driving);

        if (npc.getState().facingLeft != wasFacingLeft)
            ++flips;
        wasFacingLeft = npc.getState().facingLeft;
    }

    REQUIRE(footOf(npc).x > 96.0f);
    REQUIRE(flips < 6);
}

TEST_CASE("The shipped villager pays no mind to a player on the platform below",
          "[Npc][Level]")
{
    GameData gameData;
    REQUIRE_FALSE(glz::read_file_json(gameData, assetPath("game_data.json"), std::string{}));
    LevelData levelData;
    REQUIRE_FALSE(glz::read_file_json(levelData, assetPath("levels/level6.json"), std::string{}));

    Level level(levelData, gameData.tilePalettes, gameData.playerData, gameData.npcData);

    const NpcSpawnData &spawn = level.getNpcs().at(1);
    REQUIRE(spawn.type == "villager");

    Npc npc(gameData.npcData.at("villager"), level.patrolFor(spawn));
    standIn(npc, level.getTileMap(), spawn.tilePosition);

    float leftMost = footOf(npc).x, rightMost = footOf(npc).x;
    for (int step = 0; step < 1200; ++step)
    {
        npc.preFixedUpdate();
        npc.fixedUpdate(0.01f, level, glm::vec2(footOf(npc).x, 128.0f));

        leftMost = std::min(leftMost, footOf(npc).x);
        rightMost = std::max(rightMost, footOf(npc).x);
    }

    REQUIRE(rightMost - leftMost > 64.0f);
}
