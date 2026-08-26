#include <catch2/catch_test_macros.hpp>
#include <unordered_set>
#include <vector>
#include "game/actor/behaviors/patrol_behavior.hpp"
#include "navigation/navigation_graph.hpp"

namespace
{
    NavigationGraph setupPlatform(int nodeCount = 2, float spacing = 96.0f)
    {
        NavigationGraph navigationGraph;
        for (int id = 0; id < nodeCount; ++id)
            navigationGraph.addNode(id, {id * spacing, 192.0f});

        for (int id = 1; id < nodeCount; ++id)
        {
            navigationGraph.addEdge(id - 1, id, EdgeType::Walk);
            navigationGraph.addEdge(id, id - 1, EdgeType::Walk);
        }
        return navigationGraph;
    }

    NavigationGraph setupGap()
    {
        NavigationGraph navigationGraph;
        navigationGraph.addNode(0, {0.0f, 192.0f});
        navigationGraph.addNode(1, {96.0f, 192.0f});
        navigationGraph.addEdge(0, 1, EdgeType::Jump);
        navigationGraph.addEdge(1, 0, EdgeType::Jump);
        return navigationGraph;
    }

    NavigationGraph setupLedge()
    {
        NavigationGraph navigationGraph;
        navigationGraph.addNode(0, {0.0f, 192.0f});
        navigationGraph.addNode(1, {96.0f, 160.0f});
        navigationGraph.addEdge(0, 1, EdgeType::Jump);
        navigationGraph.addEdge(1, 0, EdgeType::Jump);
        return navigationGraph;
    }

    ActorBehaviorContext at(const NavigationGraph &navigationGraph, glm::vec2 worldPosition)
    {
        return {navigationGraph, worldPosition, glm::vec2(8.0f, 13.0f)};
    }

    PatrolBehaviorData setupData()
    {
        PatrolBehaviorData data;
        data.arrivalThreshold = 2.0f;
        return data;
    }

    void anchorAt(
        PatrolBehavior &behavior,
        const NavigationGraph &navigationGraph,
        glm::vec2 position)
    {
        behavior.reset();
        behavior.decide(0.01f, at(navigationGraph, position));
    }

    std::vector<int> walk(
        PatrolBehavior &behavior,
        const NavigationGraph &navigationGraph,
        glm::vec2 start,
        int steps)
    {
        std::vector<int> visited;
        glm::vec2 position = start;
        for (int step = 0; step < steps; ++step)
        {
            InputIntentions inputIntentions = behavior.decide(0.01f, at(navigationGraph, position));
            if (visited.empty() || visited.back() != *behavior.getCurrentNodeId())
                visited.push_back(*behavior.getCurrentNodeId());
            position.x += inputIntentions.direction.x * 2.0f;
        }
        return visited;
    }
}

TEST_CASE("Starts at the nearest node with somewhere to walk", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph = setupPlatform();
    PatrolBehavior behavior(setupData());

    anchorAt(behavior, navigationGraph, {90.0f, 192.0f});

    REQUIRE(behavior.getCurrentNodeId() == 1);
}

TEST_CASE("Has nothing to do on a graph with no edges at all", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {0, 0});
    navigationGraph.addNode(1, {96, 0});

    PatrolBehavior behavior(setupData());
    anchorAt(behavior, navigationGraph, {0, 0});

    REQUIRE_FALSE(behavior.getCurrentNodeId().has_value());
    REQUIRE(behavior.decide(0.01f, at(navigationGraph, {0, 0})).direction.x == 0.0f);
}

TEST_CASE("Takes a jump edge when it is the only way on", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph = setupGap();
    PatrolBehavior behavior(setupData());

    anchorAt(behavior, navigationGraph, {0.0f, 192.0f});

    REQUIRE(behavior.getCurrentNodeId() == 0);
    REQUIRE(behavior.getTargetNodeId() == 1);
}

TEST_CASE("Asks to jump while crossing a jump edge", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph = setupGap();
    PatrolBehavior behavior(setupData());

    anchorAt(behavior, navigationGraph, {0.0f, 192.0f});
    InputIntentions inputIntentions =
        behavior.decide(0.01f, at(navigationGraph, {0.0f, 192.0f}));

    REQUIRE(inputIntentions.jumpRequested);
    REQUIRE(inputIntentions.direction.x > 0.0f);
}

TEST_CASE("Never asks to jump on a platform it can walk", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph = setupPlatform(4);
    PatrolBehavior behavior(setupData());
    glm::vec2 position(0.0f, 192.0f);

    for (int step = 0; step < 1200; ++step)
    {
        InputIntentions inputIntentions =
            behavior.decide(0.01f, at(navigationGraph, position));
        REQUIRE_FALSE(inputIntentions.jumpRequested);
        position.x += inputIntentions.direction.x * 2.0f;
    }
}

TEST_CASE("Does not arrive at a ledge it is still below", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph = setupLedge();
    PatrolBehavior behavior(setupData());

    anchorAt(behavior, navigationGraph, {0.0f, 192.0f});
    REQUIRE(behavior.getTargetNodeId() == 1);

    behavior.decide(0.01f, at(navigationGraph, {96.0f, 192.0f}));

    REQUIRE(behavior.getCurrentNodeId() == 0);
}

TEST_CASE("Arrives at a ledge once it stands on it", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph = setupLedge();
    PatrolBehavior behavior(setupData());

    anchorAt(behavior, navigationGraph, {0.0f, 192.0f});
    behavior.decide(0.01f, at(navigationGraph, {96.0f, 160.0f}));

    REQUIRE(behavior.getCurrentNodeId() == 1);
}

TEST_CASE("Patrols a two node platform end to end", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph = setupPlatform();
    PatrolBehavior behavior(setupData());
    std::vector<int> visited = walk(behavior, navigationGraph, {0.0f, 192.0f}, 600);

    REQUIRE(visited.size() > 4);
    for (size_t index = 0; index < visited.size(); ++index)
        REQUIRE(visited[index] == static_cast<int>(index % 2));
}

TEST_CASE("Walks to the far end of a longer run before turning", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph = setupPlatform(4);
    PatrolBehavior behavior(setupData());
    std::vector<int> visited = walk(behavior, navigationGraph, {0.0f, 192.0f}, 1200);

    REQUIRE(visited.size() > 8);
    std::vector<int> expected{0, 1, 2, 3, 2, 1, 0, 1};
    for (size_t index = 0; index < expected.size(); ++index)
        REQUIRE(visited[index] == expected[index]);
}

TEST_CASE("Emits no input other than a walk direction", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph = setupPlatform();
    PatrolBehavior behavior(setupData());
    InputIntentions inputIntentions = behavior.decide(0.01f, at(navigationGraph, {0.0f, 192.0f}));

    REQUIRE(inputIntentions.direction.x != 0.0f);
    REQUIRE(inputIntentions.direction.y == 0.0f);
    REQUIRE_FALSE(inputIntentions.jumpRequested);
    REQUIRE_FALSE(inputIntentions.dashRequested);
    REQUIRE_FALSE(inputIntentions.climbRequested);
}

TEST_CASE("Anchors to the run underfoot, not a nearer one above", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {48.0f, 128.0f});
    navigationGraph.addNode(1, {160.0f, 128.0f});
    navigationGraph.addEdge(0, 1, EdgeType::Walk);
    navigationGraph.addEdge(1, 0, EdgeType::Walk);

    navigationGraph.addNode(2, {16.0f, 192.0f});
    navigationGraph.addNode(3, {304.0f, 192.0f});
    navigationGraph.addEdge(2, 3, EdgeType::Walk);
    navigationGraph.addEdge(3, 2, EdgeType::Walk);

    PatrolBehavior behavior(setupData());

    SECTION("standing on the lower run, with the upper run nearer in 2d")
    {
        anchorAt(behavior, navigationGraph, {104.0f, 192.0f});
        REQUIRE(navigationGraph.getNode(*behavior.getCurrentNodeId()).position.y == 192.0f);
    }

    SECTION("standing on the upper run")
    {
        anchorAt(behavior, navigationGraph, {104.0f, 128.0f});
        REQUIRE(navigationGraph.getNode(*behavior.getCurrentNodeId()).position.y == 128.0f);
    }

    SECTION("falling towards a run below")
    {
        anchorAt(behavior, navigationGraph, {104.0f, 60.0f});
        REQUIRE(navigationGraph.getNode(*behavior.getCurrentNodeId()).position.y == 128.0f);
    }
}
