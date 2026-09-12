#include <cstddef>
#include <string>
#include <vector>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ui/graph_shown.hpp"
#include "ui/state_machine_shown.hpp"
#include "animations/animator_data.hpp"
#include "actor/behaviors/chase_behavior_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "animations/animator_data.hpp"
#include "animations/frame_animation_data.hpp"

namespace
{
    std::vector<std::string> namesOf(const GraphShown &graph)
    {
        std::vector<std::string> names;
        for (const GraphNode &node : graph.nodes)
            names.push_back(node.name);

        return names;
    }

    std::vector<std::string> startsOf(const GraphShown &graph)
    {
        std::vector<std::string> starts;
        for (const GraphNode &node : graph.nodes)
            if (node.start)
                starts.push_back(node.name);

        return starts;
    }

    AnimatorData aWalkerWithADeath()
    {
        AnimatorData animations;
        animations.clips["walk"] = FrameAnimationData{{1, 2}, 0.1f};
        animations.clips["idle"] = FrameAnimationData{{0}, 0.5f};
        animations.clips["dead"] = FrameAnimationData{{9}, 1.0f};
        animations.clips.at("dead").loops = false;
        animations.startClip = "idle";
        AnimationWhenData dead;
        dead["alive"] = false;
        AnimationWhenData moving;
        moving["onGround"] = true;
        moving["moving"] = true;
        AnimationWhenData finished;
        finished["finished"] = true;
        animations.ladder = AnimationLadderData{
            {{"", "dead", dead}, {"", "walk", moving}, {"walk", "idle", finished}}};
        return animations;
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
    BehaviorTransitionData near;
    near.from = "idle";
    near.to = "chase";
    near.when["threatNear"] = true;
    StateMachineBehaviorData machine{{idling, chasing}, {near}};

    GraphShown graph = graphOf(machine);

    REQUIRE(namesOf(graph) == std::vector<std::string>{"idle", "chase"});
    REQUIRE(graph.nodes[1].words == "chase, standoff 28");
    REQUIRE(graph.edges == std::vector<GraphEdge>{{"idle", "chase", "threatNear"}});
}

TEST_CASE(
    "An animator draws its clips by name, then any if a rung leaves from anywhere",
    "[GraphShown]")
{
    GraphShown graph = graphOf(aWalkerWithADeath());

    REQUIRE(namesOf(graph) == std::vector<std::string>{"dead", "idle", "walk", "any"});
    REQUIRE(graph.nodes[0].words == "1 frame, 1 s each, once");
    REQUIRE(graph.nodes[1].words == "1 frame, 0.5 s each");
    REQUIRE(graph.nodes[2].words == "2 frames, 0.1 s each");
}

TEST_CASE("An animator marks the clip it starts in, and only that one", "[GraphShown]")
{
    REQUIRE(startsOf(graphOf(aWalkerWithADeath())) == std::vector<std::string>{"idle"});
}

TEST_CASE("An animator whose start clip names nothing marks no clip", "[GraphShown]")
{
    AnimatorData animations = aWalkerWithADeath();
    animations.startClip = "gone";

    REQUIRE(startsOf(graphOf(animations)).empty());
}

TEST_CASE(
    "A rung from anywhere is an edge from any, and its words are its conditions",
    "[GraphShown]")
{
    GraphShown graph = graphOf(aWalkerWithADeath());

    REQUIRE(
        graph.edges == std::vector<GraphEdge>{
                           {"any", "dead", "dead"},
                           {"any", "walk", "on ground, moving"},
                           {"walk", "idle", "clip finished"}});
}

TEST_CASE("An animator with no rung from anywhere has no any node", "[GraphShown]")
{
    AnimatorData animations;
    animations.clips["idle"] = FrameAnimationData{{0}, 0.5f};
    animations.clips["walk"] = FrameAnimationData{{1}, 0.5f};
    animations.ladder = AnimationLadderData{{{"idle", "walk", AnimationWhenData{}}}};

    REQUIRE(namesOf(graphOf(animations)) == std::vector<std::string>{"idle", "walk"});
}

TEST_CASE("A rung's words say every parameter it asks about", "[GraphShown]")
{
    AnimationWhenData when;
    when["alive"] = true;
    when["knockback"] = false;
    when["onGround"] = false;
    when["rising"] = true;
    when["onWall"] = false;
    when["finished"] = false;
    REQUIRE(
        whenOf(when) == "alive, not knocked back, in the air, off the wall, rising, clip playing");
    REQUIRE(whenOf(AnimationWhenData{}) == "always");

    AnimationWhenData asleep;
    asleep["inState"] = std::string("sleep");
    REQUIRE(whenOf(asleep) == "in state \"sleep\"");
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

TEST_CASE("The any node of an animator is its hub", "[GraphShown]")
{
    GraphShown graph = graphOf(aWalkerWithADeath());

    REQUIRE(graph.nodes.back().name == "any");
    REQUIRE(graph.nodes.back().hub);
    REQUIRE_FALSE(graph.nodes.front().hub);
}

TEST_CASE("A graph grows taller as more nodes go round", "[GraphShown]")
{
    REQUIRE(graphHeightFor(3) == 220.0f);
    REQUIRE(graphHeightFor(5) == 220.0f);
    REQUIRE(graphHeightFor(10) == 440.0f);
    REQUIRE(graphHeightFor(11) > graphHeightFor(10));
}
