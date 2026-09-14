#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/actor_data.hpp"
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "game/level_data_file.hpp"
#include "helpers/asset_path.hpp"
#include "helpers/levels.hpp"
#include "helpers/npc_fixtures.hpp"
#include "helpers/palettes.hpp"
#include "helpers/routed_npc.hpp"
#include "helpers/shipped.hpp"
#include "helpers/tile_positions.hpp"
#include "helpers/tiles.hpp"
#include "navigation/named_navigation_graph.hpp"
#include "navigation/navigation_edge.hpp"
#include "navigation/navigation_graph.hpp"
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

TEST_CASE("Every edge in a shipped level is taken by a walker to where it ends", "[RouteWalker]")
{
    PlayerData playerData = loadGameData().playerData;
    std::map<EdgeType, int> taken;

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
                bool replayed = edge.type == EdgeType::Jump || edge.type == EdgeType::Fall;
                for (float offset : replayed ? std::vector{-1.0f, 0.0f, 1.0f} : std::vector{0.0f})
                {
                    glm::vec2 from =
                        named.graph.getNode(edge.fromId).feet + glm::vec2(offset, 0.0f);
                    RouteTaken route = takeTheRoute(level, actorData, from, edge.toId);

                    INFO(
                        entry.path().filename().string()
                        << " " << named.name << " from " << from.x << "," << from.y << " to node "
                        << edge.toId << ", ended at " << route.feet.x << "," << route.feet.y);
                    REQUIRE(route.arrived);
                    REQUIRE(passedAlong(route.passedThrough, edge.fromId, edge.toId));
                    ++taken[edge.type];
                }
            }
        }
    }

    for (EdgeType type : {EdgeType::Walk, EdgeType::Jump, EdgeType::Fall, EdgeType::Climb})
        REQUIRE(taken[type] > 0);
}
