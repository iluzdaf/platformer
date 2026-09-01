#include <cmath>
#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <optional>
#include <utility>
#include <cstddef>
#include <vector>
#include "actor/actor_behavior_context.hpp"
#include "actor/actor_contact_state.hpp"
#include "actor/behaviors/patrol_behavior.hpp"
#include "navigation/navigation_edge.hpp"
#include "actor/behaviors/patrol_behavior_data.hpp"
#include "input/input_intentions.hpp"
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
        navigationGraph.addEdge({0, 1, EdgeType::Jump, {}, 0.2f});
        navigationGraph.addEdge({1, 0, EdgeType::Jump, {}, 0.2f});
        return navigationGraph;
    }

    NavigationGraph setupLedge()
    {
        NavigationGraph navigationGraph;
        navigationGraph.addNode(0, {0.0f, 192.0f});
        navigationGraph.addNode(1, {96.0f, 160.0f});
        navigationGraph.addEdge({0, 1, EdgeType::Jump, {}, 0.2f});
        navigationGraph.addEdge({1, 0, EdgeType::Jump, {}, 0.2f});
        return navigationGraph;
    }

    ActorContactState standing()
    {
        ActorContactState contacts;
        contacts.onGround = true;
        return contacts;
    }

    ActorBehaviorContext at(const NavigationGraph &navigationGraph, glm::vec2 worldPosition)
    {
        return {
            navigationGraph,
            worldPosition,
            glm::vec2(8.0f, 13.0f),
            std::nullopt,
            ActorContactState{}};
    }

    ActorBehaviorContext standingAt(const NavigationGraph &navigationGraph, glm::vec2 worldPosition)
    {
        return {navigationGraph, worldPosition, glm::vec2(8.0f, 13.0f), std::nullopt, standing()};
    }

    PatrolBehaviorData setupData()
    {
        PatrolBehaviorData data;
        data.arrivalThreshold = 2.0f;
        return data;
    }

    std::pair<glm::vec2, glm::vec2> between(glm::vec2 from, glm::vec2 to)
    {
        return {from, to};
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

    bool goesThereAndComesBack(
        PatrolBehavior &behavior,
        const NavigationGraph &navigationGraph,
        glm::vec2 start,
        float farEnd,
        int steps)
    {
        glm::vec2 position = start;
        bool gotThere = false;
        for (int step = 0; step < steps; ++step)
        {
            InputIntentions inputIntentions = behavior.decide(0.01f, at(navigationGraph, position));
            position.x += inputIntentions.direction.x * 2.0f;

            if (std::abs(position.x - farEnd) < 8.0f)
                gotThere = true;

            if (gotThere && std::abs(position.x - start.x) < 8.0f)
                return true;
        }
        return false;
    }

    std::pair<float, float> pacedBetween(
        PatrolBehavior &behavior,
        const NavigationGraph &navigationGraph,
        glm::vec2 start,
        int steps)
    {
        glm::vec2 position = start;
        float leftMost = position.x, rightMost = position.x;
        for (int step = 0; step < steps; ++step)
        {
            InputIntentions inputIntentions = behavior.decide(0.01f, at(navigationGraph, position));
            position.x += inputIntentions.direction.x * 2.0f;
            leftMost = std::min(leftMost, position.x);
            rightMost = std::max(rightMost, position.x);
        }
        return {leftMost, rightMost};
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
    PatrolBehavior behavior(setupData(), between({0.0f, 192.0f}, {96.0f, 192.0f}));

    anchorAt(behavior, navigationGraph, {0.0f, 192.0f});

    REQUIRE(behavior.getCurrentNodeId() == 0);
    REQUIRE(behavior.getTargetNodeId() == 1);
}

TEST_CASE("Asks to jump while crossing a jump edge", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph = setupGap();
    PatrolBehavior behavior(setupData(), between({0.0f, 192.0f}, {96.0f, 192.0f}));

    anchorAt(behavior, navigationGraph, {0.0f, 192.0f});
    InputIntentions inputIntentions = behavior.decide(0.01f, at(navigationGraph, {0.0f, 192.0f}));

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
        InputIntentions inputIntentions = behavior.decide(0.01f, at(navigationGraph, position));
        REQUIRE_FALSE(inputIntentions.jumpRequested);
        position.x += inputIntentions.direction.x * 2.0f;
    }
}

TEST_CASE("Does not arrive at a ledge it is still below", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph = setupLedge();
    PatrolBehavior behavior(setupData(), between({0.0f, 192.0f}, {96.0f, 160.0f}));

    anchorAt(behavior, navigationGraph, {0.0f, 192.0f});
    REQUIRE(behavior.getTargetNodeId() == 1);

    behavior.decide(0.01f, at(navigationGraph, {96.0f, 192.0f}));

    REQUIRE(behavior.getCurrentNodeId() == 0);
}

TEST_CASE("Arrives at a ledge once it stands on it", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph = setupLedge();
    PatrolBehavior behavior(setupData(), between({0.0f, 192.0f}, {96.0f, 160.0f}));

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

TEST_CASE("Stops asking to jump once the hold is spent", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {0.0f, 192.0f});
    navigationGraph.addNode(1, {96.0f, 192.0f});
    navigationGraph.addEdge({0, 1, EdgeType::Jump, {}, 0.05f});
    navigationGraph.addEdge({1, 0, EdgeType::Jump, {}, 0.05f});

    PatrolBehavior behavior(setupData(), between({0.0f, 192.0f}, {96.0f, 192.0f}));
    anchorAt(behavior, navigationGraph, {0.0f, 192.0f});

    glm::vec2 position(0.0f, 192.0f);
    std::vector<bool> asked;
    for (int step = 0; step < 10; ++step)
    {
        asked.push_back(behavior.decide(0.01f, at(navigationGraph, position)).jumpRequested);
        position.x += 1.0f;
    }

    REQUIRE(asked.front());
    REQUIRE_FALSE(asked.back());
}

TEST_CASE("Holds a longer jump for longer", "[PatrolBehavior]")
{
    auto askedFor = [](float holdDuration)
    {
        NavigationGraph navigationGraph;
        navigationGraph.addNode(0, {0.0f, 192.0f});
        navigationGraph.addNode(1, {96.0f, 192.0f});
        navigationGraph.addEdge({0, 1, EdgeType::Jump, {}, holdDuration});
        navigationGraph.addEdge({1, 0, EdgeType::Jump, {}, holdDuration});

        PatrolBehavior behavior(setupData(), between({0.0f, 192.0f}, {96.0f, 192.0f}));
        anchorAt(behavior, navigationGraph, {0.0f, 192.0f});

        int steps = 0;
        glm::vec2 position(0.0f, 192.0f);
        for (int step = 0; step < 40; ++step)
        {
            if (behavior.decide(0.01f, at(navigationGraph, position)).jumpRequested)
                ++steps;
            position.x += 1.0f;
        }
        return steps;
    };

    REQUIRE(askedFor(0.20f) > askedFor(0.05f));
}

namespace
{
    NavigationGraph setupPlatformOverAnother()
    {
        NavigationGraph navigationGraph;
        navigationGraph.addNode(0, {0.0f, 128.0f});
        navigationGraph.addNode(1, {96.0f, 128.0f});
        navigationGraph.addNode(2, {192.0f, 128.0f});
        navigationGraph.addNode(3, {192.0f, 192.0f});
        navigationGraph.addNode(4, {288.0f, 192.0f});

        for (auto [left, right] : {std::pair(0, 1), std::pair(1, 2), std::pair(3, 4)})
        {
            navigationGraph.addEdge(left, right, EdgeType::Walk);
            navigationGraph.addEdge(right, left, EdgeType::Walk);
        }

        navigationGraph.addEdge(2, 3, EdgeType::Fall);
        return navigationGraph;
    }
}

TEST_CASE("Stays on its own platform when it does not roam", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph = setupPlatformOverAnother();
    PatrolBehavior behavior(setupData());

    anchorAt(behavior, navigationGraph, {0.0f, 128.0f});
    std::vector<int> visited = walk(behavior, navigationGraph, {0.0f, 128.0f}, 400);

    REQUIRE_FALSE(visited.empty());
    for (int id : visited)
        REQUIRE(navigationGraph.getNode(id).position.y == 128.0f);
}

TEST_CASE("Will not roam somewhere it cannot get back from", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph = setupPlatformOverAnother();
    PatrolBehavior behavior(setupData(), between({0.0f, 128.0f}, {96.0f, 128.0f}));

    anchorAt(behavior, navigationGraph, {0.0f, 128.0f});
    std::vector<int> visited = walk(behavior, navigationGraph, {0.0f, 128.0f}, 400);

    REQUIRE_FALSE(visited.empty());
    for (int id : visited)
        REQUIRE(navigationGraph.getNode(id).position.y == 128.0f);
}

TEST_CASE("Roams to the far platform when it can get back", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph = setupPlatformOverAnother();
    navigationGraph.addEdge({3, 2, EdgeType::Jump, {}, 0.2f});

    PatrolBehavior behavior(setupData(), between({0.0f, 128.0f}, {288.0f, 192.0f}));
    anchorAt(behavior, navigationGraph, {0.0f, 128.0f});

    REQUIRE(behavior.getCurrentNodeId() == 0);
    REQUIRE(behavior.getTargetNodeId() == 1);
}

TEST_CASE("Picks itself up again after coming off its route", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph = setupPlatformOverAnother();
    navigationGraph.addEdge({3, 2, EdgeType::Jump, {}, 0.2f});

    PatrolBehavior behavior(setupData(), between({0.0f, 128.0f}, {288.0f, 192.0f}));

    anchorAt(behavior, navigationGraph, {0.0f, 128.0f});
    REQUIRE(navigationGraph.getNode(*behavior.getCurrentNodeId()).position.y == 128.0f);

    ActorBehaviorContext knockedDown = standingAt(navigationGraph, {288.0f, 192.0f});
    behavior.decide(0.01f, knockedDown);

    REQUIRE(navigationGraph.getNode(*behavior.getCurrentNodeId()).position.y == 192.0f);
    REQUIRE(behavior.decide(0.01f, knockedDown).direction.x != 0.0f);
}

TEST_CASE("Does not steer while falling", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {0.0f, 128.0f});
    navigationGraph.addNode(1, {96.0f, 128.0f});
    navigationGraph.addNode(2, {101.0f, 400.0f});
    navigationGraph.addEdge(0, 1, EdgeType::Walk);
    navigationGraph.addEdge(1, 0, EdgeType::Walk);
    navigationGraph.addEdge(1, 2, EdgeType::Fall);
    navigationGraph.addEdge({2, 1, EdgeType::Jump, {}, 0.2f});

    PatrolBehavior behavior(setupData(), between({101.0f, 400.0f}, {0.0f, 128.0f}));
    anchorAt(behavior, navigationGraph, {96.0f, 128.0f});
    REQUIRE(behavior.getCurrentNodeId() == 1);
    REQUIRE(behavior.getTargetNodeId() == 2);

    REQUIRE(
        behavior.decide(0.01f, standingAt(navigationGraph, {96.0f, 128.0f})).direction.x != 0.0f);

    for (float x : {99.0f, 101.0f, 103.0f, 100.0f, 102.0f})
    {
        INFO("falling past x " << x);
        REQUIRE(behavior.decide(0.01f, at(navigationGraph, {x, 300.0f})).direction.x == 0.0f);
    }
}

TEST_CASE("Notices it is on a different platform at the same height", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {48.0f, 128.0f});
    navigationGraph.addNode(1, {160.0f, 128.0f});
    navigationGraph.addNode(2, {208.0f, 128.0f});
    navigationGraph.addNode(3, {304.0f, 128.0f});
    navigationGraph.addEdge(0, 1, EdgeType::Walk);
    navigationGraph.addEdge(1, 0, EdgeType::Walk);
    navigationGraph.addEdge(2, 3, EdgeType::Walk);
    navigationGraph.addEdge(3, 2, EdgeType::Walk);
    navigationGraph.addEdge({1, 2, EdgeType::Jump, {}, 0.2f});
    navigationGraph.addEdge({2, 1, EdgeType::Jump, {}, 0.2f});

    PatrolBehavior behavior(setupData(), between({48.0f, 128.0f}, {304.0f, 128.0f}));
    anchorAt(behavior, navigationGraph, {48.0f, 128.0f});
    REQUIRE(behavior.getCurrentNodeId() == 0);

    ActorBehaviorContext elsewhere = standingAt(navigationGraph, {304.0f, 128.0f});
    behavior.decide(0.01f, elsewhere);

    REQUIRE(behavior.getCurrentNodeId() == 3);
    REQUIRE(behavior.decide(0.01f, elsewhere).direction.x != 0.0f);
}

TEST_CASE("Tries the jump again after coming up short", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {0.0f, 128.0f});
    navigationGraph.addNode(1, {96.0f, 64.0f});
    navigationGraph.addEdge({0, 1, EdgeType::Jump, {}, 0.2f});
    navigationGraph.addEdge({1, 0, EdgeType::Jump, {}, 0.2f});

    PatrolBehavior behavior(setupData(), between({0.0f, 128.0f}, {96.0f, 64.0f}));
    anchorAt(behavior, navigationGraph, {0.0f, 128.0f});
    REQUIRE(behavior.getCurrentNodeId() == 0);
    REQUIRE(behavior.getTargetNodeId() == 1);

    ActorBehaviorContext backWhereItStarted = standingAt(navigationGraph, {0.0f, 128.0f});
    for (int step = 0; step < 40; ++step)
        behavior.decide(0.01f, backWhereItStarted);

    bool asksAgain = false;
    for (int step = 0; step < 60 && !asksAgain; ++step)
        asksAgain = behavior.decide(0.01f, backWhereItStarted).jumpRequested;

    REQUIRE(asksAgain);
}

TEST_CASE("Told nowhere to walk, it paces what it is standing on", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph = setupPlatform(3, 64.0f);
    PatrolBehavior behavior(setupData());

    anchorAt(behavior, navigationGraph, {64.0f, 192.0f});
    std::vector<int> visited = walk(behavior, navigationGraph, {64.0f, 192.0f}, 900);

    REQUIRE(std::ranges::find(visited, 0) != visited.end());
    REQUIRE(std::ranges::find(visited, 2) != visited.end());
}

TEST_CASE("Told where to walk, it walks between those two and turns round", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph = setupPlatform(4, 64.0f);

    PatrolBehavior behavior(setupData(), between({64.0f, 192.0f}, {128.0f, 192.0f}));
    anchorAt(behavior, navigationGraph, {64.0f, 192.0f});

    std::vector<int> visited = walk(behavior, navigationGraph, {64.0f, 192.0f}, 900);

    REQUIRE(std::ranges::find(visited, 1) != visited.end());
    REQUIRE(std::ranges::find(visited, 2) != visited.end());
    REQUIRE(std::ranges::find(visited, 3) == visited.end());
}

TEST_CASE("A beat may end between two nodes rather than at one", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph = setupPlatform(2, 192.0f);
    PatrolBehavior behavior(setupData(), between({0.0f, 192.0f}, {96.0f, 192.0f}));
    anchorAt(behavior, navigationGraph, {0.0f, 192.0f});

    auto [leftMost, rightMost] = pacedBetween(behavior, navigationGraph, {0.0f, 192.0f}, 900);

    REQUIRE(rightMost < 192.0f - 32.0f);
    REQUIRE(rightMost > 96.0f - 32.0f);
    REQUIRE(leftMost < 32.0f);
}

TEST_CASE("A beat ending between nodes still turns round and comes back", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph = setupPlatform(2, 192.0f);
    PatrolBehavior behavior(setupData(), between({48.0f, 192.0f}, {144.0f, 192.0f}));
    anchorAt(behavior, navigationGraph, {48.0f, 192.0f});

    auto [leftMost, rightMost] = pacedBetween(behavior, navigationGraph, {48.0f, 192.0f}, 1800);

    REQUIRE(rightMost > 144.0f - 32.0f);
    REQUIRE(rightMost < 192.0f - 16.0f);
    REQUIRE(leftMost < 48.0f + 32.0f);
}

TEST_CASE("A beat end past the end of a run means the end of the run", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph = setupPlatform(2, 192.0f);
    PatrolBehavior behavior(setupData(), between({0.0f, 192.0f}, {4000.0f, 192.0f}));
    anchorAt(behavior, navigationGraph, {0.0f, 192.0f});

    REQUIRE(goesThereAndComesBack(behavior, navigationGraph, {0.0f, 192.0f}, 192.0f, 1800));
}

TEST_CASE("A beat end several nodes along is still stopped at", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph = setupPlatform(4, 64.0f);
    PatrolBehavior behavior(setupData(), between({32.0f, 192.0f}, {160.0f, 192.0f}));
    anchorAt(behavior, navigationGraph, {32.0f, 192.0f});

    auto [leftMost, rightMost] = pacedBetween(behavior, navigationGraph, {32.0f, 192.0f}, 1800);

    REQUIRE(rightMost > 160.0f - 16.0f);
    REQUIRE(rightMost < 192.0f - 8.0f);
    REQUIRE(leftMost < 32.0f + 16.0f);
    REQUIRE(leftMost > 0.0f + 8.0f);
}

TEST_CASE("A beat end several nodes along is reached and left again", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph = setupPlatform(4, 64.0f);
    PatrolBehavior behavior(setupData(), between({32.0f, 192.0f}, {160.0f, 192.0f}));
    anchorAt(behavior, navigationGraph, {32.0f, 192.0f});

    REQUIRE(goesThereAndComesBack(behavior, navigationGraph, {32.0f, 192.0f}, 160.0f, 2400));
}

TEST_CASE("A beat inside one half of a single edge is not overshot", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph = setupPlatform(2, 320.0f);
    PatrolBehavior behavior(setupData(), between({32.0f, 192.0f}, {128.0f, 192.0f}));
    anchorAt(behavior, navigationGraph, {32.0f, 192.0f});

    auto [leftMost, rightMost] = pacedBetween(behavior, navigationGraph, {32.0f, 192.0f}, 3000);

    INFO("paced " << leftMost << " .. " << rightMost << " for a beat of 32 .. 128");
    REQUIRE(rightMost < 128.0f + 16.0f);
    REQUIRE(leftMost > 32.0f - 16.0f);
}

TEST_CASE("A beat in the far half of a single edge is not overshot either", "[PatrolBehavior]")
{
    NavigationGraph navigationGraph = setupPlatform(2, 320.0f);
    PatrolBehavior behavior(setupData(), between({200.0f, 192.0f}, {250.0f, 192.0f}));
    anchorAt(behavior, navigationGraph, {200.0f, 192.0f});

    auto [leftMost, rightMost] = pacedBetween(behavior, navigationGraph, {200.0f, 192.0f}, 3000);

    INFO("paced " << leftMost << " .. " << rightMost << " for a beat of 200 .. 250");
    REQUIRE(rightMost < 250.0f + 16.0f);
    REQUIRE(leftMost > 200.0f - 16.0f);
}
