#include <cstddef>
#include <string>
#include <vector>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ui/graph_shown.hpp"
#include "animations/animation_rule_data.hpp"
#include "ui/state_machine_shown.hpp"
#include "actor/behaviors/chase_behavior_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "state_machines/state_machine_data.hpp"
#include "conditions/when_data.hpp"

namespace
{
    std::vector<std::string> namesOf(const GraphShown &graph)
    {
        std::vector<std::string> names;
        for (const GraphNode &node : graph.nodes)
            names.push_back(node.name);

        return names;
    }
}

TEST_CASE("A machine draws as its states and its transitions, in words", "[GraphShown]")
{
    BehaviorStateData chasing;
    chasing.name = "chase";
    ChaseBehaviorData chase;
    chase.standoff = 28.0f;
    chasing.does = chase;
    BehaviorStateData idling;
    idling.name = "idle";
    TransitionData near;
    near.from = "idle";
    near.to = "chase";
    near.when["threatNear"] = true;
    StateMachineBehaviorData machine{{idling, chasing}, {near}};

    GraphShown graph = graphOf(machine);

    REQUIRE(namesOf(graph) == std::vector<std::string>{"idle", "chase"});
    REQUIRE(graph.nodes[1].words == "chase, standoff 28");
    REQUIRE(graph.edges == std::vector<GraphEdge>{{"idle", "chase", "threatNear"}});
}

TEST_CASE("A rule's words say every fact it asks about", "[GraphShown]")
{
    WhenData when;
    when["alive"] = true;
    when["knockback"] = false;
    when["onGround"] = false;
    when["rising"] = true;
    when["onWall"] = false;
    when["finished"] = false;
    REQUIRE(
        whenOf(AnimationRuleData{"jump", when}) ==
        "alive, not knocked back, in the air, off the wall, rising, clip playing");
    REQUIRE(whenOf(AnimationRuleData{"idle", {}}) == "always");

    WhenData asleep;
    asleep["inState"] = std::string("sleep");
    REQUIRE(whenOf(AnimationRuleData{"sleep", asleep}) == "in state \"sleep\"");
}

TEST_CASE(
    "A node is found by its name, and an edge knows whether its reverse exists",
    "[GraphShown]")
{
    GraphShown graph{
        {{"a", ""}, {"b", ""}, {"c", ""}},
        {{"a", "b", ""}, {"b", "a", ""}, {"a", "a", ""}, {"a", "c", ""}}};

    REQUIRE(indexOfNode(graph, "b") == 1);
    REQUIRE_FALSE(indexOfNode(graph, "d").has_value());
    REQUIRE(goesBothWays(graph, graph.edges[0]));
    REQUIRE(goesBothWays(graph, graph.edges[2]));
    REQUIRE_FALSE(goesBothWays(graph, graph.edges[3]));
}

TEST_CASE("A selection past the end of the graph is no selection", "[GraphShown]")
{
    GraphShown graph{{{"a", ""}}, {{"a", "a", ""}}};

    REQUIRE(stillAmong(showingState(0), graph) == showingState(0));
    REQUIRE(stillAmong(showingState(1), graph) == MachineShown{});
    REQUIRE(stillAmong(showingTransition(1), graph) == MachineShown{});
}

TEST_CASE("A hub sits in the middle and the rest go round it on an ellipse", "[GraphShown]")
{
    GraphShown graph{
        {{"idle", ""}, {"walk", ""}, {"any", "", true}, {"jump", ""}, {"fall", ""}}, {}};

    std::vector<glm::vec2> placed =
        placedAround(graph, glm::vec2(100.0f, 200.0f), glm::vec2(80.0f, 40.0f));

    REQUIRE(placed.size() == 5);
    REQUIRE(placed[2] == glm::vec2(100.0f, 200.0f));
    REQUIRE(placed[0].x == Catch::Approx(100.0f).margin(0.001f));
    REQUIRE(placed[0].y == Catch::Approx(160.0f).margin(0.001f));
    REQUIRE(placed[1].x == Catch::Approx(180.0f).margin(0.001f));
    REQUIRE(placed[1].y == Catch::Approx(200.0f).margin(0.001f));
    REQUIRE(placed[3].y == Catch::Approx(240.0f).margin(0.001f));
    REQUIRE(placed[4].x == Catch::Approx(20.0f).margin(0.001f));
    REQUIRE(nodesAroundIn(graph) == 4);
}

TEST_CASE("A graph grows taller as more nodes go round", "[GraphShown]")
{
    REQUIRE(graphHeightFor(3) == 220.0f);
    REQUIRE(graphHeightFor(5) == 220.0f);
    REQUIRE(graphHeightFor(10) == 440.0f);
    REQUIRE(graphHeightFor(11) > graphHeightFor(10));
}
