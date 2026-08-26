#include <catch2/catch_test_macros.hpp>
#include "navigation/navigation_graph_builder.hpp"
#include "game/tile_map/tile_map.hpp"
#include "test_helpers/test_tile_map_utils.hpp"

namespace
{
    constexpr int FloorRow = 6;
    constexpr int CeilingRow = 4;
    constexpr int MapWidthTiles = 10;

    NavigationProfile profileOfHeight(float height)
    {
        NavigationProfile profile;
        profile.colliderSize = glm::vec2(8.0f, height);
        return profile;
    }

    NavigationProfile standardProfile()
    {
        return profileOfHeight(13.0f);
    }

    TileMap setupFloor()
    {
        TileMap tileMap = setupTileMap();
        for (int x = 0; x < MapWidthTiles; ++x)
            tileMap.setTileIndex(glm::ivec2(x, FloorRow), 1);
        return tileMap;
    }

    TileMap setupFloorUnderOneTileOfHeadroom()
    {
        TileMap tileMap = setupFloor();
        for (int x = 0; x < MapWidthTiles; ++x)
            tileMap.setTileIndex(glm::ivec2(x, CeilingRow), 1);
        return tileMap;
    }

    size_t nodesOnTheFloor(const NavigationGraph &graph, const TileMap &tileMap)
    {
        float floorY = static_cast<float>(FloorRow * tileMap.getTileSize());
        size_t count = 0;
        for (const auto &[id, node] : graph.getNodes())
            if (node.position.y == floorY)
                ++count;
        return count;
    }
}

TEST_CASE("A floor gives a profile somewhere to walk", "[NavigationGraphBuilder]")
{
    TileMap tileMap = setupFloor();

    NavigationGraph graph = buildNavigationGraph(tileMap, standardProfile());

    REQUIRE(graph.getNodes().size() == 2);
    REQUIRE_FALSE(graph.getEdges().empty());
    for (const auto &edge : graph.getEdges())
        REQUIRE(edge.type == EdgeType::Walk);
}

TEST_CASE("A profile too tall for the headroom has nowhere to stand", "[NavigationGraphBuilder]")
{
    TileMap tileMap = setupFloorUnderOneTileOfHeadroom();

    NavigationGraph graph = buildNavigationGraph(tileMap, profileOfHeight(20.0f));

    REQUIRE(nodesOnTheFloor(graph, tileMap) == 0);
}

TEST_CASE("A profile that fits the headroom still walks under it", "[NavigationGraphBuilder]")
{
    TileMap tileMap = setupFloorUnderOneTileOfHeadroom();

    NavigationGraph graph = buildNavigationGraph(tileMap, standardProfile());

    REQUIRE(nodesOnTheFloor(graph, tileMap) == 2);
}

TEST_CASE("Headroom rounds up to whole tiles", "[NavigationGraphBuilder]")
{
    TileMap tileMap = setupFloorUnderOneTileOfHeadroom();
    float tileSize = static_cast<float>(tileMap.getTileSize());

    SECTION("a collider exactly one tile tall needs one tile")
    {
        NavigationGraph graph = buildNavigationGraph(tileMap, profileOfHeight(tileSize));
        REQUIRE(nodesOnTheFloor(graph, tileMap) == 2);
    }

    SECTION("a collider one pixel taller needs two")
    {
        NavigationGraph graph = buildNavigationGraph(tileMap, profileOfHeight(tileSize + 1.0f));
        REQUIRE(nodesOnTheFloor(graph, tileMap) == 0);
    }
}
