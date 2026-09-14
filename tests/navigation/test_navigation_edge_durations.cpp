#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/mantle_ability_data.hpp"
#include "actor/abilities/move_ability_data.hpp"
#include "actor/abilities/wall_climb_ability_data.hpp"
#include "navigation/navigation_edge.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_graph_steps.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/navigation_profile.hpp"
#include "timing/fixed_time_step.hpp"

using Catch::Approx;

namespace
{
    NavigationProfile movingAt(float moveSpeed)
    {
        NavigationProfile profile;
        profile.abilities.move = MoveAbilityData{moveSpeed};
        return profile;
    }

    NavigationProfile climbingAt(float climbSpeed)
    {
        NavigationProfile profile = movingAt(100.0f);
        profile.abilities.wallClimb = WallClimbAbilityData{climbSpeed};
        return profile;
    }

    NavigationEdge an(EdgeType type, std::vector<glm::vec2> path = {})
    {
        return {0, 1, type, path, {}};
    }
}

TEST_CASE("A walk takes as long as it goes across at the move speed", "[NavigationEdgeDurations]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {0.0f, 100.0f});
    navigationGraph.addNode(1, {50.0f, 97.0f});

    NavigationEdge walk = navigation::timed(an(EdgeType::Walk), navigationGraph, movingAt(100.0f));

    REQUIRE(walk.duration == Approx(0.5f));
}

TEST_CASE("A body that cannot move has no time for a walk", "[NavigationEdgeDurations]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {0.0f, 100.0f});
    navigationGraph.addNode(1, {50.0f, 100.0f});

    REQUIRE_FALSE(
        navigation::timed(an(EdgeType::Walk), navigationGraph, NavigationProfile{}).duration);
}

TEST_CASE(
    "A climb up a wall, or down onto the floor, takes its height at the climb speed",
    "[NavigationEdgeDurations]")
{
    NavigationProfile profile = climbingAt(80.0f);
    profile.abilities.mantle = MantleAbilityData{90.0f, 0.3f};

    NavigationGraph upTheWall;
    upTheWall.addNode(0, {0.0f, 100.0f}, NodeKind::OnWall);
    upTheWall.addNode(1, {2.0f, 20.0f}, NodeKind::OnWall);
    REQUIRE(navigation::timed(an(EdgeType::Climb), upTheWall, profile).duration == Approx(1.0f));

    NavigationGraph downOntoTheFloor;
    downOntoTheFloor.addNode(0, {0.0f, 20.0f}, NodeKind::OnWall);
    downOntoTheFloor.addNode(1, {2.0f, 100.0f});
    REQUIRE(
        navigation::timed(an(EdgeType::Climb), downOntoTheFloor, profile).duration == Approx(1.0f));
}

TEST_CASE(
    "A climb onto the ledge above is the mantle, and the climb its pull-up leaves",
    "[NavigationEdgeDurations]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {0.0f, 20.0f}, NodeKind::OnWall);
    navigationGraph.addNode(1, {4.0f, 4.0f});
    NavigationProfile profile = climbingAt(80.0f);
    profile.abilities.mantle = MantleAbilityData{90.0f, 0.3f};

    float pulledUp = 90.0f * 0.3f * 0.5f;
    REQUIRE(
        navigation::timed(an(EdgeType::Climb), navigationGraph, profile).duration ==
        Approx((16.0f - pulledUp) / 80.0f + 0.3f));

    profile.abilities.mantle.reset();
    REQUIRE(
        navigation::timed(an(EdgeType::Climb), navigationGraph, profile).duration ==
        Approx(16.0f / 80.0f));
}

TEST_CASE(
    "A jump or fall takes its ticks in the air and the walk on to its node",
    "[NavigationEdgeDurations]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {0.0f, 100.0f});
    navigationGraph.addNode(1, {50.0f, 150.0f});
    std::vector<glm::vec2> path(11, glm::vec2(0.0f, 100.0f));
    path.back() = glm::vec2(40.0f, 150.0f);

    for (EdgeType type : {EdgeType::Jump, EdgeType::Fall})
        REQUIRE(
            navigation::timed(an(type, path), navigationGraph, movingAt(100.0f)).duration ==
            Approx(10.0f * PhysicsStep + 0.1f));
}

TEST_CASE(
    "A ledge no higher than the mantle pulls up takes just the mantle",
    "[NavigationEdgeDurations]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {0.0f, 20.0f}, NodeKind::OnWall);
    navigationGraph.addNode(1, {4.0f, 10.0f});
    NavigationProfile profile = climbingAt(80.0f);
    profile.abilities.mantle = MantleAbilityData{90.0f, 0.3f};

    REQUIRE(
        navigation::timed(an(EdgeType::Climb), navigationGraph, profile).duration == Approx(0.3f));
}
