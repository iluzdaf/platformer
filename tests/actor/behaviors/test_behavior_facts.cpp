#include <optional>
#include <string_view>
#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/actor_behavior_context.hpp"
#include "actor/behaviors/behavior_facts.hpp"
#include "actor/behaviors/senses_data.hpp"
#include "conditions/fact_rows.hpp"
#include "helpers/behaviour_context.hpp"
#include "navigation/navigation_graph.hpp"

namespace
{
    const SensesData RatSenses{40.0f, 24.0f};

    NavigationGraph aRunAndALedgeBelow()
    {
        NavigationGraph navigationGraph = aWalkRun();
        navigationGraph.addNode(10, {100.0f, 288.0f});
        return navigationGraph;
    }

    ActorBehaviorContext sensing(ActorBehaviorContext context, const SensesData &senses)
    {
        context.senses = &senses;
        return context;
    }

    bool holds(std::string_view fact, const ActorBehaviorContext &context)
    {
        return rowNamed(behaviorRows(), fact)->holds(true, context);
    }
}

TEST_CASE("With no threat, nothing about the threat holds", "[BehaviorFacts]")
{
    NavigationGraph navigationGraph = aRunAndALedgeBelow();
    ActorBehaviorContext alone = sensing(standingAt(navigationGraph, {192.0f, 192.0f}), RatSenses);

    REQUIRE_FALSE(holds("threatOnMySurface", alone));
    REQUIRE_FALSE(holds("cornered", alone));
    REQUIRE_FALSE(holds("threatClose", alone));
    REQUIRE_FALSE(holds("threatInReach", alone));
}

TEST_CASE(
    "A threat on the run under my feet is on my surface, and one on a ledge below is not",
    "[BehaviorFacts]")
{
    NavigationGraph navigationGraph = aRunAndALedgeBelow();

    REQUIRE(holds(
        "threatOnMySurface",
        standingAt(navigationGraph, {0.0f, 192.0f}, glm::vec2(384.0f, 192.0f))));
    REQUIRE_FALSE(holds(
        "threatOnMySurface",
        standingAt(navigationGraph, {96.0f, 192.0f}, glm::vec2(100.0f, 288.0f))));
}

TEST_CASE("A threat is close within my close, and in reach within my reach", "[BehaviorFacts]")
{
    NavigationGraph navigationGraph = aRunAndALedgeBelow();
    glm::vec2 here{192.0f, 192.0f};

    ActorBehaviorContext thirtyAway =
        sensing(standingAt(navigationGraph, here, here + glm::vec2(30.0f, 0.0f)), RatSenses);
    REQUIRE(holds("threatClose", thirtyAway));
    REQUIRE_FALSE(holds("threatInReach", thirtyAway));

    ActorBehaviorContext twentyAway =
        sensing(standingAt(navigationGraph, here, here - glm::vec2(20.0f, 0.0f)), RatSenses);
    REQUIRE(holds("threatClose", twentyAway));
    REQUIRE(holds("threatInReach", twentyAway));

    ActorBehaviorContext fiftyAway =
        sensing(standingAt(navigationGraph, here, here + glm::vec2(50.0f, 0.0f)), RatSenses);
    REQUIRE_FALSE(holds("threatClose", fiftyAway));
}

TEST_CASE("Cornered is having nowhere further from the threat to go", "[BehaviorFacts]")
{
    NavigationGraph navigationGraph = aRunAndALedgeBelow();

    REQUIRE(
        holds("cornered", standingAt(navigationGraph, {0.0f, 192.0f}, glm::vec2(40.0f, 192.0f))));
    REQUIRE_FALSE(holds(
        "cornered", standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(240.0f, 192.0f))));
}
