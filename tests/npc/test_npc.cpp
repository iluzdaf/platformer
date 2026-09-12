#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "actor/behaviors/attack_behavior_data.hpp"
#include "actor/abilities/pounce_ability_data.hpp"
#include "actor/abilities/charge_ability_data.hpp"
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <cmath>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/behaviors/patrol_behavior_data.hpp"
#include "game/level.hpp"
#include "game/level_data.hpp"
#include "helpers/actors.hpp"
#include "helpers/tile_positions.hpp"
#include "helpers/tiles.hpp"
#include "helpers/levels.hpp"
#include "helpers/ledge_and_wall.hpp"
#include "helpers/npc_fixtures.hpp"
#include "helpers/scripted_npcs.hpp"
#include "helpers/palettes.hpp"
#include "helpers/shipped.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_place.hpp"
#include "npc/npc.hpp"
#include "npc/npc_data.hpp"
#include "npc/npc_spawn_data.hpp"
#include "player/player_data.hpp"
#include "tile_map/tile_map.hpp"
#include "tile_map/tile_palette_data.hpp"

using namespace ledge_and_wall;

namespace
{
    std::map<std::string, NpcData> npcCatalogue()
    {
        return {{"rat", setupNpcData()}};
    }

    Level levelOf(const TileMap &tileMap, glm::ivec2 npcTile)
    {
        LevelData levelData;
        levelData.playerFeet = feetOf(glm::ivec2(0, 0));
        levelData.tileMapData = tileMap.toTileMapData();
        levelData.npcs = {spawnAt("rat", npcTile)};
        return Level(
            levelData, theOnlyPalette(aPaletteWithASolidTile()), PlayerData(), npcCatalogue(), {});
    }

    Level setupWalkableLevel()
    {
        std::vector<std::pair<glm::ivec2, int>> laid;
        for (int x = 0; x < 10; ++x)
            laid.push_back({glm::ivec2(x, 6), 1});

        return levelOf(aTileMap(laid), glm::ivec2(4, 5));
    }

    constexpr int TwoTierWidthTiles = 20;
    constexpr int TwoTierHeightTiles = 14;
    constexpr int FloorRow = 12;
    constexpr int PlatformRow = 8;
    constexpr int PlatformFirstTile = 3;
    constexpr int PlatformLastTile = 9;
    constexpr glm::ivec2 UnderThePlatform{6, FloorRow - 1};

    TileMap twoTierTiles()
    {
        std::vector<std::pair<glm::ivec2, int>> laid;
        for (int x = 0; x < TwoTierWidthTiles; ++x)
            laid.push_back({glm::ivec2(x, FloorRow), 1});

        for (int x = PlatformFirstTile; x <= PlatformLastTile; ++x)
            laid.push_back({glm::ivec2(x, PlatformRow), 1});

        return aTileMap(laid, TwoTierWidthTiles, TwoTierHeightTiles);
    }

    Level setupTwoTierLevel()
    {
        return levelOf(twoTierTiles(), UnderThePlatform);
    }

    Level twoTierLevelWith(const std::vector<NpcSpawnData> &npcs)
    {
        TileMap tileMap = twoTierTiles();

        LevelData levelData;

        levelData.playerFeet = feetOf(glm::ivec2(0, 0));
        levelData.tileMapData = tileMap.toTileMapData();
        levelData.npcs = npcs;

        return Level(
            levelData, theOnlyPalette(aPaletteWithASolidTile()), PlayerData(), npcCatalogue(), {});
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
        npc.standAt(tileMap.feetOnTile(tilePosition));
    }

    float footX(const Npc &npc)
    {
        return footOf(npc).x;
    }

    float reachOf(const Npc &npc)
    {
        return npc.body().colliderSize().x * 0.5f + PatrolBehaviorData().arrivalThreshold;
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
    Npc npc(spawnAt("rat", SpawnTile), setupNpcData());

    standIn(npc, tileMap, SpawnTile);

    REQUIRE(footOf(npc) == tileMap.feetOnTile(SpawnTile));
}

TEST_CASE("Where an npc is placed decides which way it sets off", "[Npc]")
{
    Level level = setupWalkableLevel();
    const TileMap &tileMap = level.getTileMap();

    Npc left(spawnAt("rat", SpawnTile), setupNpcData());
    Npc right(spawnAt("rat", SpawnTile), setupNpcData());
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
    Npc npc(spawnAt("rat", SpawnTile), setupNpcData());
    standIn(npc, tileMap, SpawnTile);

    float lowestFootY = footOf(npc).y;
    std::vector<float> footXs;

    for (int step = 0; step < 4000; ++step)
    {
        stepNpc(npc, level, 1);
        footXs.push_back(footX(npc));
        lowestFootY = std::max(lowestFootY, footOf(npc).y);
    }

    for (const auto &[id, node] : level.graphFor(npc.profile()).getNodes())
        REQUIRE(cameWithin(footXs, node.feet.x, reachOf(npc)));

    REQUIRE(lowestFootY <= 6.0f * tileMap.getTileSize());

    glm::vec2 position = npc.body().position();
    REQUIRE(position.x >= -static_cast<float>(tileMap.getTileSize()));
    REQUIRE(position.x <= static_cast<float>(tileMap.getWorldWidth()));
}

TEST_CASE("Stands still in a level with nothing to walk on", "[Npc]")
{
    TileMap tiles = aTileMap();
    Level level = levelOf(tiles, glm::ivec2(3, 4));

    Npc npc(spawnAt("rat", SpawnTile), setupNpcData());
    npc.standAt(feetOf(glm::ivec2(3, 4)));
    stepNpc(npc, level, 100);

    REQUIRE(npc.feet().x == feetOf(glm::ivec2(3, 4)).x);
}
TEST_CASE("Patrolling is deterministic, so where you place them is what differs", "[Npc]")
{
    Level level = setupWalkableLevel();
    const TileMap &tileMap = level.getTileMap();

    Npc first(spawnAt("rat", SpawnTile), setupNpcData());
    Npc second(spawnAt("rat", SpawnTile), setupNpcData());
    standIn(first, tileMap, SpawnTile);
    standIn(second, tileMap, SpawnTile);

    stepNpc(first, level, 600);
    stepNpc(second, level, 600);

    REQUIRE(first.body().position() == second.body().position());
}

TEST_CASE("A level names the npcs it is populated with", "[Npc][Level]")
{
    LevelData levelData;
    levelData.playerFeet = feetOf(glm::ivec2(0, 0));
    levelData.tileMapData.tilePalette = "default";
    levelData.tileMapData.indices = std::vector<std::vector<int>>(10, std::vector<int>(10, 0));
    levelData.npcs = {spawnAt("rat", {1, 1}), spawnAt("rat", {2, 1})};

    Level level(
        levelData, theOnlyPalette(aPaletteWithASolidTile()), PlayerData(), npcCatalogue(), {});

    REQUIRE(spawnsIn(level).size() == 2);
    REQUIRE(spawnsIn(level)[0].type == "rat");
    REQUIRE(spawnsIn(level)[0].feet == feetOf(glm::ivec2(1, 1)));
    REQUIRE(spawnsIn(level)[1].feet == feetOf(glm::ivec2(2, 1)));
    REQUIRE(spawnsIn(level) == levelData.npcs);
}

TEST_CASE("A level rejects an npc placed somewhere it cannot stand", "[Npc][Level]")
{
    LevelData levelData;
    levelData.playerFeet = feetOf(glm::ivec2(0, 0));
    TilePaletteData palette = aPaletteWithASolidTile();
    levelData.tileMapData.tilePalette = "default";
    levelData.tileMapData.indices = std::vector<std::vector<int>>(10, std::vector<int>(10, 0));
    for (int x = 0; x < 10; ++x)
        levelData.tileMapData.indices[6][x] = 1;

    auto levelWith = [&](std::vector<NpcSpawnData> npcs)
    {
        levelData.npcs = std::move(npcs);
        return Level(levelData, theOnlyPalette(palette), PlayerData(), npcCatalogue(), {});
    };

    SECTION("out of bounds")
    {
        REQUIRE_THROWS_WITH(
            levelWith({spawnAt("rat", {99, 99})}), "Npc start position is out of bounds");
    }

    SECTION("inside a solid tile")
    {
        REQUIRE_THROWS_WITH(
            levelWith({spawnAt("rat", {3, 6})}), "Npc start position is on a solid tile");
    }

    SECTION("somewhere it can stand")
    {
        REQUIRE_NOTHROW(levelWith({spawnAt("rat", {3, 5})}));
    }
}

TEST_CASE("An npc on the ground patrols the ground, not the platform above it", "[Npc]")
{
    Level level = setupTwoTierLevel();
    const TileMap &tileMap = level.getTileMap();
    Npc npc(spawnAt("rat", SpawnTile), setupNpcData());
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
    Npc npc(spawnAt("rat", SpawnTile), setupNpcData());
    standIn(npc, tileMap, UnderThePlatform);

    std::vector<float> footXs = patrolFootXs(npc, level, 4000);

    int floorNodes = 0;
    for (const auto &[id, node] : level.graphFor(npc.profile()).getNodes())
    {
        if (node.feet.y != floorTopY(tileMap))
            continue;

        ++floorNodes;
        REQUIRE(cameWithin(footXs, node.feet.x, reachOf(npc)));
    }

    REQUIRE(floorNodes > 1);
}

TEST_CASE("An npc given no behavior data does nothing", "[Npc]")
{
    Level level = setupWalkableLevel();
    const TileMap &tileMap = level.getTileMap();

    NpcData npcData = setupNpcData();
    npcData.stateMachineBehaviorData.reset();

    Npc npc(spawnAt("rat", SpawnTile), npcData);
    standIn(npc, tileMap, SpawnTile);

    stepNpc(npc, level, 400);

    REQUIRE(footOf(npc).x == tileMap.feetOnTile(SpawnTile).x);
}

TEST_CASE("An npc says which state it is in", "[Npc][Level]")
{
    NpcSpawnData spawn = patrolling("rat", LedgeRightEnd, LedgeLeftEnd, LedgeRightEnd);
    Level level = levelWithALedgeAndAWall({spawn});

    Npc npc(spawn, shippedNpcData().at("rat"));
    ScriptedNpcs scripts;
    scripts.script(npc);

    REQUIRE(npc.stateName() == "patrol");

    for (int step = 0; step < 20; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level, {.threatFeet = footOf(npc) + glm::vec2(8.0f, 0.0f)});
    }

    REQUIRE((npc.stateName() == "flee" || npc.stateName() == "pounce"));

    for (int step = 0; step < 400; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level, {.threatFeet = glm::vec2(112.0f, 192.0f)});
    }

    REQUIRE(npc.stateName() == "patrol");
}

TEST_CASE("An npc with no behavior names no state", "[Npc]")
{
    NpcData npcData = setupNpcData();
    npcData.stateMachineBehaviorData.reset();

    Npc npc(spawnAt("rat", SpawnTile), npcData);

    REQUIRE(npc.stateName().empty());
}

TEST_CASE("A beat a rat cannot make a round trip of is not walkable", "[Npc][Level]")
{
    NpcSpawnData onTheGround = patrolling(
        "rat",
        glm::ivec2(6, GroundRow - 1),
        glm::ivec2(2, GroundRow - 1),
        glm::ivec2(17, GroundRow - 1));
    Level level = levelWithALedgeAndAWall({onTheGround});

    Npc rat(onTheGround, shippedNpcData().at("rat"));
    const NavigationGraph &graph = level.graphFor(rat.profile());

    const std::optional<PatrolData> &authored = onTheGround.patrol;
    REQUIRE(authored);
    REQUIRE(canPatrolBetween(graph, authored->from, authored->to));

    NpcSpawnData reachingTooHigh = onTheGround;
    reachingTooHigh.patrol = beatOf(glm::ivec2(2, GroundRow - 1), LedgeLeftEnd);
    const std::optional<PatrolData> &impossible = reachingTooHigh.patrol;
    REQUIRE(impossible);
    REQUIRE_FALSE(canPatrolBetween(graph, impossible->from, impossible->to));
}

TEST_CASE("A beat naming both ends of a run walks the whole of it", "[Npc][Level]")
{
    NpcSpawnData spawn = patrolling("rat", LedgeRightEnd, LedgeLeftEnd, LedgeRightEnd);
    Level level = levelWithALedgeAndAWall({spawn});
    Npc npc(spawn, shippedNpcData().at("rat"));

    float leftMost = footOf(npc).x, rightMost = footOf(npc).x;
    for (int step = 0; step < 3000; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level);
        leftMost = std::min(leftMost, footOf(npc).x);
        rightMost = std::max(rightMost, footOf(npc).x);
    }

    float half = npc.body().aabb().size.x * 0.5f;
    float ledgeLeft = static_cast<float>(LedgeLeftEnd.x * 16);
    float ledgeRight = static_cast<float>((LedgeLastTile + 1) * 16);

    REQUIRE(leftMost - half < ledgeLeft + 4.0f);
    REQUIRE(rightMost + half > ledgeRight - 4.0f);
}

TEST_CASE("A beat ending partway up a wall is climbed to and no further", "[Npc][Level][Climb]")
{
    NpcSpawnData spawn =
        patrolling("spider", LedgeRightEnd, LedgeRightEnd, glm::ivec2(1, LedgeRow - 3));
    Level level = levelWithALedgeAndAWall({spawn});
    Npc npc(spawn, shippedNpcData().at("spider"));

    float highest = footOf(npc).y;
    for (int step = 0; step < 4000; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level);
        highest = std::min(highest, footOf(npc).y);
    }

    float askedFor = surfaceOf(LedgeRow - 2);

    REQUIRE(highest <= askedFor + 4.0f);
    REQUIRE(highest > surfaceOf(1));
}
TEST_CASE("An npc drops off a platform to a beat end below its edge", "[Npc]")
{
    constexpr glm::ivec2 UnderTheEdge{PlatformFirstTile, FloorRow - 1};
    constexpr glm::ivec2 AlongTheFloor{PlatformLastTile, FloorRow - 1};

    NpcSpawnData spawn = patrolling(
        "rat", glm::ivec2(PlatformFirstTile, PlatformRow - 1), UnderTheEdge, AlongTheFloor);
    Level level = twoTierLevelWith({spawn});

    Npc npc(spawn, npcCatalogue().at(spawn.type));
    standIn(npc, level.getTileMap(), glm::ivec2(PlatformFirstTile, PlatformRow - 1));

    const float theFloor = floorTopY(level.getTileMap());
    bool cameDown = false;
    for (int step = 0; step < 2000 && !cameDown; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level);
        cameDown = std::abs(footOf(npc).y - theFloor) < 2.0f;
    }

    REQUIRE(cameDown);
}

TEST_CASE("A patrolling npc says which node it set off from and where it is headed", "[Npc]")
{
    NpcSpawnData spawn = patrolling("spider", OnTheGround, OnTheGround, LedgeLeftEnd);
    Level level = levelWithALedgeAndAWall({spawn});

    Npc npc(spawn, shippedNpcData().at(spawn.type));

    REQUIRE_FALSE(npc.currentNodeId());

    std::set<std::pair<int, int>> legsWalked;
    for (int step = 0; step < 4000; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level);

        std::optional<int> setOffAt = npc.currentNodeId();
        std::optional<int> headingFor = npc.targetNodeId();
        if (setOffAt && headingFor)
            legsWalked.insert({*setOffAt, *headingFor});
    }

    REQUIRE(legsWalked.size() > 1);
    for (const auto &[setOffAt, headingFor] : legsWalked)
        REQUIRE(setOffAt != headingFor);
}

TEST_CASE("A level hands its npcs the player to react to", "[Npc][Level]")
{
    NpcSpawnData spawn = patrolling("rat", LedgeRightEnd, LedgeLeftEnd, LedgeRightEnd);
    Level level = levelWithALedgeAndAWall({spawn});

    REQUIRE(level.getNpcs().size() == 1);
    Npc &npc = *level.getNpcs().front();
    ScriptedNpcs scripts;
    scripts.script(npc);
    REQUIRE(npc.stateName() == "patrol");

    for (int step = 0; step < 20; ++step)
    {
        level.beginFrame();
        level.fixedUpdate(0.01f, {.threatFeet = footOf(npc) + glm::vec2(8.0f, 0.0f)});
    }

    REQUIRE((npc.stateName() == "flee" || npc.stateName() == "pounce"));
}

TEST_CASE("A level drives the npcs it holds", "[Npc][Level]")
{
    NpcSpawnData spawn = patrolling("rat", LedgeRightEnd, LedgeLeftEnd, LedgeRightEnd);
    Level level = levelWithALedgeAndAWall({spawn});
    const Npc &npc = *level.getNpcs().front();

    glm::vec2 setOffAt = footOf(npc);

    for (int step = 0; step < 200; ++step)
    {
        level.beginFrame();
        level.fixedUpdate(0.01f, {.threatFeet = glm::vec2(112.0f, 192.0f)});
        level.postFixedUpdate();
    }

    REQUIRE(footOf(npc).x != setOffAt.x);
}

namespace
{
    float leftmostReached(Level &level, int steps)
    {
        const Npc &npc = *level.getNpcs().front();
        float leftmost = footOf(npc).x;

        for (int step = 0; step < steps; ++step)
        {
            level.beginFrame();
            level.fixedUpdate(0.01f, {.threatFeet = glm::vec2(1000.0f, 1000.0f)});
            level.postFixedUpdate();
            leftmost = std::min(leftmost, footOf(npc).x);
        }

        return leftmost;
    }
}

TEST_CASE("An npc walks further when its beat is the whole ledge", "[Npc][Level]")
{
    const glm::ivec2 shortOfTheEnd{LedgeLastTile - 2, LedgeRow - 1};

    Level wholeLedge =
        levelWithALedgeAndAWall({patrolling("rat", LedgeRightEnd, LedgeLeftEnd, LedgeRightEnd)});
    Level shortBeat =
        levelWithALedgeAndAWall({patrolling("rat", LedgeRightEnd, shortOfTheEnd, LedgeRightEnd)});

    REQUIRE(leftmostReached(wholeLedge, 600) < leftmostReached(shortBeat, 600));
}

TEST_CASE("An npc with no beat at all walks past where a beat would turn it", "[Npc][Level]")
{
    const glm::ivec2 shortOfTheEnd{LedgeLastTile - 2, LedgeRow - 1};

    Level kept =
        levelWithALedgeAndAWall({patrolling("rat", LedgeRightEnd, shortOfTheEnd, LedgeRightEnd)});

    Level freed = levelWithALedgeAndAWall({spawnAt("rat", LedgeRightEnd)});

    REQUIRE(leftmostReached(freed, 600) < leftmostReached(kept, 600));
}

namespace
{
    NpcData aCreatureWhoseStateAttacksWith(const char *with, bool able)
    {
        NpcData data = setupNpcData();
        if (able && with == PounceAttack)
            data.actorData.abilities.pounce = PounceAbilityData{};
        if (able && with == ChargeAttack)
            data.actorData.abilities.charge = ChargeAbilityData{};
        BehaviorStateData attacking;
        attacking.name = "attack";
        attacking.does = AttackBehaviorData{with};
        data.stateMachineBehaviorData->states.push_back(attacking);
        return data;
    }
}

TEST_CASE("A creature whose state attacks with something it cannot do is refused", "[Npc]")
{
    REQUIRE_THROWS_WITH(
        Npc(spawnAt("rat", SpawnTile), aCreatureWhoseStateAttacksWith("pounce", false)),
        Catch::Matchers::ContainsSubstring("pounce") &&
            Catch::Matchers::ContainsSubstring("no such ability"));
    REQUIRE_THROWS_WITH(
        Npc(spawnAt("rat", SpawnTile), aCreatureWhoseStateAttacksWith("headbutt", true)),
        Catch::Matchers::ContainsSubstring("headbutt"));
}

TEST_CASE("A creature whose state attacks with an ability it has is welcome", "[Npc]")
{
    REQUIRE_NOTHROW(Npc(spawnAt("rat", SpawnTile), aCreatureWhoseStateAttacksWith("pounce", true)));
    REQUIRE_NOTHROW(Npc(spawnAt("rat", SpawnTile), aCreatureWhoseStateAttacksWith("charge", true)));
}
