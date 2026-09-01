#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <utility>
#include "ui/navigation_shown.hpp"

namespace
{
    NavigationShown everything()
    {
        return NavigationShown{2, 7, std::pair<int, int>{7, 9}};
    }
}

TEST_CASE("Showing nothing is what a panel starts with", "[NavigationShown]")
{
    NavigationShown shown;

    REQUIRE_FALSE(shown.graphIndex);
    REQUIRE_FALSE(shown.nodeId);
    REQUIRE_FALSE(shown.edge);
}

TEST_CASE("Picking a graph forgets the node and the edge under it", "[NavigationShown]")
{
    NavigationShown shown = showingGraph(1);

    REQUIRE(shown.graphIndex == 1);
    REQUIRE_FALSE(shown.nodeId);
    REQUIRE_FALSE(shown.edge);
}

TEST_CASE("Picking a graph while showing one forgets what was under the old", "[NavigationShown]")
{
    NavigationShown shown = showingGraph(1);

    REQUIRE(shown != everything());
    REQUIRE(showingGraph(2).nodeId == std::nullopt);
}

TEST_CASE("Picking no graph forgets everything", "[NavigationShown]")
{
    NavigationShown shown = showingGraph(std::nullopt);

    REQUIRE(shown == NavigationShown{});
}

TEST_CASE("Picking a node keeps the graph and forgets the edge", "[NavigationShown]")
{
    NavigationShown shown = showingNode(everything(), 3);

    REQUIRE(shown.graphIndex == 2);
    REQUIRE(shown.nodeId == 3);
    REQUIRE_FALSE(shown.edge);
}

TEST_CASE("Picking every node forgets the edge too", "[NavigationShown]")
{
    NavigationShown shown = showingNode(everything(), std::nullopt);

    REQUIRE(shown.graphIndex == 2);
    REQUIRE_FALSE(shown.nodeId);
    REQUIRE_FALSE(shown.edge);
}

TEST_CASE("Picking an edge keeps the graph and the node it belongs to", "[NavigationShown]")
{
    NavigationShown shown = showingEdge(showingNode(showingGraph(2), 7), std::pair<int, int>{7, 9});

    REQUIRE(shown == everything());
}

TEST_CASE("A graph the level no longer has takes the selection with it", "[NavigationShown]")
{
    REQUIRE(stillAmong(everything(), 1) == NavigationShown{});
    REQUIRE(stillAmong(everything(), 0) == NavigationShown{});
}

TEST_CASE("A graph the level still has is left alone", "[NavigationShown]")
{
    REQUIRE(stillAmong(everything(), 3) == everything());
    REQUIRE(stillAmong(everything(), 99) == everything());
}

TEST_CASE("Showing nothing survives a level with no graphs at all", "[NavigationShown]")
{
    REQUIRE(stillAmong(NavigationShown{}, 0) == NavigationShown{});
}
