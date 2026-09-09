#include <cstddef>
#include <string>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include "ui/graph_shown.hpp"
#include "ui/state_machine_shown.hpp"
#include "actor/actor_animation_data.hpp"
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

    ActorAnimationData aWalkerWithADeath()
    {
        ActorAnimationData animations;
        animations.clips["walk"] = FrameAnimationData{{1, 2}, 0.1f};
        animations.clips["idle"] = FrameAnimationData{{0}, 0.5f};
        animations.clips["dead"] = FrameAnimationData{{9}, 1.0f};
        animations.clips.at("dead").loops = false;
        AnimationWhen dead;
        dead.alive = false;
        AnimationWhen moving;
        moving.onGround = true;
        moving.moving = true;
        AnimationWhen finished;
        finished.finished = true;
        animations.ladder =
            AnimatorData{{{"", "dead", dead}, {"", "walk", moving}, {"walk", "idle", finished}}};
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
    near.threatWithin = 40.0f;
    StateMachineBehaviorData machine{{idling, chasing}, {near}};

    GraphShown graph = graphOf(machine);

    REQUIRE(namesOf(graph) == std::vector<std::string>{"idle", "chase"});
    REQUIRE(graph.nodes[1].words == "chase, standoff 28");
    REQUIRE(graph.edges == std::vector<GraphEdge>{{"idle", "chase", "threat within 40"}});
}

TEST_CASE(
    "An animator draws idle first, then its clips by name, then any if a rung leaves from anywhere",
    "[GraphShown]")
{
    GraphShown graph = graphOf(aWalkerWithADeath());

    REQUIRE(namesOf(graph) == std::vector<std::string>{"idle", "dead", "walk", "any"});
    REQUIRE(graph.nodes[0].words == "1 frame, 0.5 s each");
    REQUIRE(graph.nodes[1].words == "1 frame, 1 s each, once");
    REQUIRE(graph.nodes[2].words == "2 frames, 0.1 s each");
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
    ActorAnimationData animations;
    animations.clips["idle"] = FrameAnimationData{{0}, 0.5f};
    animations.clips["walk"] = FrameAnimationData{{1}, 0.5f};
    animations.ladder = AnimatorData{{{"idle", "walk", AnimationWhen{}}}};

    REQUIRE(namesOf(graphOf(animations)) == std::vector<std::string>{"idle", "walk"});
}

TEST_CASE("A rung's words say every parameter it asks about", "[GraphShown]")
{
    AnimationWhen when;
    when.alive = true;
    when.knockback = false;
    when.onGround = false;
    when.rising = true;
    when.onWall = false;
    when.finished = false;
    REQUIRE(
        whenOf(when) == "alive, not knocked back, in the air, off the wall, rising, clip playing");
    REQUIRE(whenOf(AnimationWhen{}) == "always");
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
