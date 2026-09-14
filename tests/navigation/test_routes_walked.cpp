#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/actor_data.hpp"
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "game/level_data_file.hpp"
#include "helpers/asset_path.hpp"
#include "helpers/navigation_maps.hpp"
#include "helpers/actors.hpp"
#include "helpers/levels.hpp"
#include "helpers/npc_fixtures.hpp"
#include "npc/npc_data.hpp"
#include "helpers/palettes.hpp"
#include "helpers/routed_npc.hpp"
#include "helpers/shipped.hpp"
#include "helpers/tile_positions.hpp"
#include "helpers/tiles.hpp"
#include "navigation/footing.hpp"
#include "navigation/named_navigation_graph.hpp"
#include "navigation/navigation_edge.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/navigation_path.hpp"
#include "navigation/navigation_profile_builder.hpp"
#include "navigation/route_walker.hpp"
#include "player/player_data.hpp"

namespace
{
    int endOfTheRow(const NavigationGraph &graph, float y)
    {
        std::optional<int> furthest;
        for (const auto &[id, node] : graph.getNodes())
            if (node.feet.y == y && (!furthest || node.feet.x > graph.getNode(*furthest).feet.x))
                furthest = id;

        return furthest.value();
    }

    bool tookA(EdgeType type, const NavigationGraph &graph, const std::vector<int> &passedThrough)
    {
        for (std::size_t leg = 1; leg < passedThrough.size(); ++leg)
            for (const NavigationEdge &edge : graph.getOutgoingEdges(passedThrough[leg - 1]))
                if (edge.toId == passedThrough[leg] && edge.type == type)
                    return true;

        return false;
    }

    bool passedAlong(const std::vector<int> &passedThrough, int fromId, int toId)
    {
        for (std::size_t leg = 1; leg < passedThrough.size(); ++leg)
            if (passedThrough[leg - 1] == fromId && passedThrough[leg] == toId)
                return true;

        return false;
    }
}

TEST_CASE(
    "A walker takes a generated route off a ledge and on along the floor below",
    "[RouteWalker]")
{
    constexpr int Ledge = 3;
    constexpr int Floor = 9;
    Placed laid;
    layRow(laid, Ledge, 0, 3);
    layRow(laid, Floor, 0, 11);
    ActorData walker = setupNpcData().actorData;
    Level level(
        aLevelPlacing(laid, 12, 12, {1, Ledge - 1}, {}),
        theOnlyPalette(aPaletteWithASolidTile()),
        PlayerData(),
        {{"routed", routedBy(walker)}},
        {});
    const NavigationGraph &graph = level.graphFor(buildNavigationProfile(walker));
    float ledgeTop = surfaceOf(Ledge);
    float floorTop = surfaceOf(Floor);
    glm::vec2 takeOff = graph.getNode(endOfTheRow(graph, ledgeTop)).feet;
    int destination = endOfTheRow(graph, floorTop);
    float reach = walker.physicsBodyData.colliderSize.x * 0.5f + WalkerArrivesWithin;

    glm::vec2 from = takeOff;
    SECTION("From the far end of the ledge")
    {
        from = glm::vec2(8.0f, ledgeTop);
    }
    SECTION("From a little short of where the fall leaves")
    {
        from = takeOff - glm::vec2(5.0f, 0.0f);
    }
    SECTION("From a little past where the fall leaves")
    {
        from = takeOff + glm::vec2(1.0f, 0.0f);
    }

    RouteTaken route = takeTheRoute(level, walker, from, destination);

    INFO("ended at " << route.feet.x << "," << route.feet.y);
    REQUIRE(route.arrived);
    REQUIRE(tookA(EdgeType::Fall, graph, route.passedThrough));
    REQUIRE(route.feet.y == floorTop);
    REQUIRE(std::abs(route.feet.x - graph.getNode(destination).feet.x) <= reach);
}

namespace
{
    constexpr int PillarFloor = 13;
    constexpr int PillarTop = 3;

    struct WayDown
    {
        RouteTaken route;
        bool climbed = false;
        bool fell = false;
        bool leaptOffTheWall = false;
    };

    WayDown wayDownThePillar(const PlayerData &playerData, const NpcData &spiderData, bool player)
    {
        Placed laid;
        layRow(laid, PillarFloor, 0, 9);
        for (int row = PillarTop; row < PillarFloor; ++row)
            layRow(laid, row, 4, 5);
        Level level(
            aLevelPlacing(laid, 10, 16, {1, PillarFloor - 1}, {}),
            theOnlyPalette(aPaletteWithASolidTile()),
            playerData,
            {{"spider", spiderData}},
            {});
        const ActorData &actorData = player ? playerData.actorData : spiderData.actorData;
        const NavigationGraph &graph = level.graphFor(buildNavigationProfile(actorData));
        int ledgeEnd = endOfTheRow(graph, surfaceOf(PillarTop));
        int farEndOfTheFloor = endOfTheRow(graph, surfaceOf(PillarFloor));

        WayDown wayDown;
        wayDown.route =
            takeTheRoute(level, actorData, graph.getNode(ledgeEnd).feet, farEndOfTheFloor);
        wayDown.climbed = tookA(EdgeType::Climb, graph, wayDown.route.passedThrough);
        wayDown.fell = tookA(EdgeType::Fall, graph, wayDown.route.passedThrough);
        const std::vector<int> &passed = wayDown.route.passedThrough;
        for (std::size_t leg = 1; leg < passed.size(); ++leg)
            for (const NavigationEdge &edge : graph.getOutgoingEdges(passed[leg - 1]))
                if (edge.toId == passed[leg] && edge.type == EdgeType::Jump &&
                    graph.getNode(edge.fromId).kind == NodeKind::OnWall)
                    wayDown.leaptOffTheWall = true;

        return wayDown;
    }

    PlayerData withoutAWallJump(PlayerData playerData)
    {
        playerData.actorData.abilities.wallJump.reset();
        return playerData;
    }

    NpcData withoutAWallJump(NpcData npcData)
    {
        npcData.actorData.abilities.wallJump.reset();
        return npcData;
    }
}

TEST_CASE(
    "Off a tall wall, a body that slides down it climbs down, and one that does not drops",
    "[RouteWalker]")
{
    PlayerData playerData = withoutAWallJump(loadGameData().playerData);
    NpcData spiderData = withoutAWallJump(shippedNpcData().at("spider"));
    REQUIRE(playerData.actorData.abilities.wallSlide);
    REQUIRE_FALSE(spiderData.actorData.abilities.wallSlide);

    WayDown player = wayDownThePillar(playerData, spiderData, true);
    REQUIRE(player.route.arrived);
    REQUIRE(player.climbed);
    REQUIRE_FALSE(player.fell);

    WayDown spider = wayDownThePillar(playerData, spiderData, false);
    REQUIRE(spider.route.arrived);
    REQUIRE_FALSE(spider.climbed);
    REQUIRE(spider.fell);
}

TEST_CASE(
    "Off a tall wall, a body that can wall jump lowers onto it and leaps clear, sooner",
    "[RouteWalker]")
{
    PlayerData playerData = loadGameData().playerData;
    NpcData spiderData = shippedNpcData().at("spider");
    REQUIRE(playerData.actorData.abilities.wallJump);
    REQUIRE(spiderData.actorData.abilities.wallJump);

    for (bool player : {true, false})
    {
        INFO((player ? "player" : "spider"));
        WayDown leaping = wayDownThePillar(playerData, spiderData, player);
        WayDown without =
            wayDownThePillar(withoutAWallJump(playerData), withoutAWallJump(spiderData), player);

        REQUIRE(leaping.route.arrived);
        REQUIRE(leaping.leaptOffTheWall);
        REQUIRE(leaping.route.seconds < without.route.seconds);
    }
}

TEST_CASE(
    "A walker climbs a wall and leaps from it onto a shelf nothing else reaches",
    "[RouteWalker]")
{
    ActorData leaper = setupNpcData().actorData;
    leaper.physicsBodyData.colliderSize = glm::vec2(8.0f, 13.0f);
    leaper.abilities = wallJumperAbilities();
    glm::vec2 onTheFloor = feetOf({ShelfWallX + 2, ShelfFloorRow - 1});
    Level level(
        aLevelPlacing(
            aWallAcrossFromAShelf(), ShelfSceneTiles, ShelfSceneTiles, {1, ShelfFloorRow - 1}, {}),
        theOnlyPalette(aPaletteWithSlipperyTiles()),
        PlayerData(),
        {{"routed", routedBy(leaper)}},
        {});
    const NavigationGraph &graph = level.graphFor(buildNavigationProfile(leaper));
    int shelfEnd = endOfTheRow(graph, surfaceOf(ShelfRow));

    RouteTaken route = takeTheRoute(level, leaper, onTheFloor, shelfEnd);

    INFO("ended at " << route.feet.x << "," << route.feet.y);
    REQUIRE(route.arrived);
    REQUIRE(tookA(EdgeType::Climb, graph, route.passedThrough));
    REQUIRE(tookA(EdgeType::Jump, graph, route.passedThrough));
}

TEST_CASE(
    "Every edge in a shipped level gets a walker where it ends, in about the time it says",
    "[RouteWalker]")
{
    constexpr float AboutItsDuration = 0.15f;
    PlayerData playerData = loadGameData().playerData;
    std::map<EdgeType, int> taken;
    int leapsOffWalls = 0;

    for (const auto &entry : std::filesystem::directory_iterator(assetPath("levels")))
    {
        if (entry.path().extension() != ".json")
            continue;

        Level level(
            readLevelData(entry.path().string()),
            shippedPalettes(),
            playerData,
            shippedNpcData(),
            shippedPickupData());

        for (const NamedNavigationGraph &named : level.getGraphs())
        {
            std::string walker = named.name.substr(0, named.name.find(','));
            const ActorData &actorData =
                walker == "player" ? playerData.actorData : shippedNpcData().at(walker).actorData;

            for (const NavigationEdge &edge : named.graph.getEdges())
            {
                if (findPath(named.graph, edge.fromId, edge.toId) !=
                    std::vector{edge.fromId, edge.toId})
                {
                    RouteTaken around = takeTheRoute(
                        level, actorData, named.graph.getNode(edge.fromId).feet, edge.toId);
                    INFO(
                        entry.path().filename().string()
                        << " " << named.name << " around to node " << edge.toId);
                    REQUIRE(around.arrived);
                    continue;
                }

                bool replayed = edge.type == EdgeType::Jump || edge.type == EdgeType::Fall;
                NavigationNode takeOff = named.graph.getNode(edge.fromId);
                bool offTheWall = replayed && takeOff.kind == NodeKind::OnWall;
                glm::vec2 along = offTheWall ? glm::vec2(0.0f, ClimbArrivesWithin)
                                             : glm::vec2(TakeOffReach, 0.0f);
                for (float offset : replayed ? std::vector{-1.0f, 0.0f, 1.0f} : std::vector{0.0f})
                {
                    glm::vec2 from = takeOff.feet + along * offset;
                    RouteTaken route = takeTheRoute(level, actorData, from, edge.toId);

                    INFO(
                        entry.path().filename().string()
                        << " " << named.name << " from " << from.x << "," << from.y << " to node "
                        << edge.toId << ", ended at " << route.feet.x << "," << route.feet.y);
                    REQUIRE(route.arrived);
                    REQUIRE(passedAlong(route.passedThrough, edge.fromId, edge.toId));
                    REQUIRE(
                        std::abs(route.seconds - costOf(named.graph, edge)) <= AboutItsDuration);
                    ++taken[edge.type];
                    leapsOffWalls += offTheWall ? 1 : 0;
                }
            }
        }
    }

    for (EdgeType type : {EdgeType::Walk, EdgeType::Jump, EdgeType::Fall, EdgeType::Climb})
        REQUIRE(taken[type] > 0);
    REQUIRE(leapsOffWalls > 0);
}
