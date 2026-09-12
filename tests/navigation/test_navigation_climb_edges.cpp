#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <set>
#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "game/game_data.hpp"
#include "helpers/asset_path.hpp"
#include "helpers/actors.hpp"
#include "helpers/tiles.hpp"
#include "helpers/navigation_maps.hpp"
#include "helpers/shipped.hpp"
#include "helpers/graph_queries.hpp"
#include "helpers/palettes.hpp"
#include "navigation/navigation_edge.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_graph_builder.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/navigation_profile.hpp"
#include "navigation/navigation_profile_builder.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_map.hpp"
#include "tile_map/tile_palette_data.hpp"

namespace
{
    constexpr int ClimbHangRow = ClimbWallTopRow + 1;
}

TEST_CASE(
    "A wall an actor can hang on becomes a way up and back down",
    "[NavigationGraphBuilder][Climb]")
{
    TileMap tileMap = aWallFromTheFloor();

    std::set<std::pair<int, int>> joined =
        rowsJoinedByClimbing(buildNavigationGraph(tileMap, climberProfile()), tileMap);

    REQUIRE(joined.contains({ClimbFloorRow, ClimbHangRow}));
    REQUIRE(joined.contains({ClimbHangRow, ClimbFloorRow}));
    REQUIRE(joined.contains({ClimbHangRow, ClimbWallTopRow}));
    REQUIRE(joined.contains({ClimbWallTopRow, ClimbHangRow}));
}

TEST_CASE("An actor that cannot climb is given no way up a wall", "[NavigationGraphBuilder][Climb]")
{
    TileMap tileMap = aWallFromTheFloor();

    REQUIRE(
        rowsJoinedByClimbing(buildNavigationGraph(tileMap, standardProfile()), tileMap).empty());
}

TEST_CASE("Hanging without climbing is no way up", "[NavigationGraphBuilder][Climb]")
{
    TileMap tileMap = aWallFromTheFloor();
    NavigationProfile hangsOnly = climberProfile();
    hangsOnly.abilities.wallClimb.reset();

    REQUIRE(rowsJoinedByClimbing(buildNavigationGraph(tileMap, hangsOnly), tileMap).empty());
}

TEST_CASE(
    "A wall that does not reach the floor is not climbed from it",
    "[NavigationGraphBuilder][Climb]")
{
    Placed laid = wallFromTheFloor();
    laid.push_back({glm::ivec2(ClimbWallX, ClimbFloorRow - 1), 0});
    TileMap tileMap = aTileMap(laid, 10, 12);

    std::set<std::pair<int, int>> joined =
        rowsJoinedByClimbing(buildNavigationGraph(tileMap, climberProfile()), tileMap);

    REQUIRE_FALSE(joined.contains({ClimbFloorRow, ClimbHangRow}));
}

TEST_CASE(
    "A ledge beside the wall breaks one long climb into two",
    "[NavigationGraphBuilder][Climb]")
{
    Placed laid = wallFromTheFloor();
    laid.push_back({glm::ivec2(ClimbWallX - 1, 6), 1});
    laid.push_back({glm::ivec2(ClimbWallX + 1, 6), 1});
    TileMap tileMap = aTileMap(laid, 10, 12);

    std::set<std::pair<int, int>> joined =
        rowsJoinedByClimbing(buildNavigationGraph(tileMap, climberProfile()), tileMap);

    REQUIRE_FALSE(joined.contains({ClimbFloorRow, ClimbHangRow}));
    REQUIRE(joined.contains({6, ClimbHangRow}));
}

TEST_CASE("A wall you cannot stand on top of is not climbed", "[NavigationGraphBuilder][Climb]")
{
    Placed laid = wallFromTheFloor();
    layRow(laid, ClimbWallTopRow - 1, 0, 9);
    TileMap tileMap = aTileMap(laid, 10, 12);

    std::set<std::pair<int, int>> joined =
        rowsJoinedByClimbing(buildNavigationGraph(tileMap, climberProfile()), tileMap);

    REQUIRE_FALSE(joined.contains({ClimbHangRow, ClimbWallTopRow}));
}

TEST_CASE("A wall an actor can climb gets a node on it", "[NavigationGraphBuilder][Climb]")
{
    TileMap tileMap = aWallFromTheFloor();

    std::vector<NavigationNode> onWalls =
        nodesOnWalls(buildNavigationGraph(tileMap, climberProfile()));

    REQUIRE_FALSE(onWalls.empty());
    for (const NavigationNode &node : onWalls)
    {
        glm::vec2 underfoot(0.0f, 1.0f);
        REQUIRE(tileMap.tileContaining(node.feet + underfoot).y == ClimbHangRow);
    }
}

TEST_CASE("An actor that cannot climb gets no wall nodes", "[NavigationGraphBuilder][Climb]")
{
    TileMap tileMap = aWallFromTheFloor();

    REQUIRE(nodesOnWalls(buildNavigationGraph(tileMap, standardProfile())).empty());
}

TEST_CASE(
    "Climbing goes by way of the wall, never straight onto the ledge",
    "[NavigationGraphBuilder][Climb]")
{
    TileMap tileMap = aWallFromTheFloor();
    NavigationGraph graph = buildNavigationGraph(tileMap, climberProfile());

    int climbs = 0;
    for (const auto &[id, node] : graph.getNodes())
        for (const NavigationEdge &edge : graph.getOutgoingEdges(id))
        {
            if (edge.type != EdgeType::Climb)
                continue;

            climbs++;
            REQUIRE(
                (graph.getNode(edge.fromId).kind == NodeKind::OnWall ||
                 graph.getNode(edge.toId).kind == NodeKind::OnWall));
        }

    REQUIRE(climbs > 0);
}

TEST_CASE(
    "No level leaves a wall node with nothing joined to it",
    "[NavigationGraphBuilder][Climb]")
{
    GameData gameData = loadGameData();
    NavigationProfile player = buildNavigationProfile(gameData.playerData.actorData);

    for (const std::filesystem::directory_entry &entry :
         std::filesystem::directory_iterator(assetPath("levels")))
    {
        TileMap tileMap = tilesOfLevel(entry.path().string());
        NavigationGraph graph = buildNavigationGraph(tileMap, player);

        for (const auto &[id, node] : graph.getNodes())
        {
            if (node.kind != NodeKind::OnWall)
                continue;

            INFO(entry.path().filename().string() << " node " << id);
            REQUIRE_FALSE(graph.getOutgoingEdges(id).empty());
        }
    }
}

TEST_CASE("A wall an actor cannot grip is not climbed", "[NavigationGraphBuilder][Climb]")
{
    TilePaletteData palette = aPaletteWithASolidTile();
    TileData ungrippable;
    ungrippable.solid = true;
    ungrippable.grippable = false;
    palette.tiles[2] = ungrippable;

    Placed laid;
    layRow(laid, ClimbFloorRow, 0, 9);
    for (int y = ClimbWallTopRow; y < ClimbFloorRow; ++y)
        laid.push_back({glm::ivec2(ClimbWallX, y), 2});

    TileMap ungrippableWall = aTileMap(laid, 10, 12, 16, palette);

    REQUIRE(nodesOnWalls(buildNavigationGraph(ungrippableWall, climberProfile())).empty());
}
