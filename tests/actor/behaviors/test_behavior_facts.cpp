#include <span>
#include <string>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/actor_behavior_context.hpp"
#include "actor/behaviors/behavior_facts.hpp"
#include "conditions/asked.hpp"
#include "conditions/fact_rows.hpp"
#include "game/noise.hpp"
#include "helpers/behaviour_context.hpp"
#include "navigation/navigation_graph.hpp"

namespace
{
    bool fact(
        const char *name,
        const Asked &asked,
        const NavigationGraph &graph,
        glm::vec2 feet,
        const std::vector<Noise> &noises)
    {
        ActorBehaviorContext context = standingAt(graph, feet);
        context.noises = noises;
        return rowNamed(behaviorRows(), name)->holds(asked, context);
    }
}

TEST_CASE("A landing is heard within a distance, and only a landing", "[BehaviorFacts]")
{
    NavigationGraph graph = aWalkRun();
    glm::vec2 feet(96.0f, 192.0f);

    REQUIRE(
        fact("landingWithin", 64.0f, graph, feet, {{std::string(LandingNoise), {140.0f, 192.0f}}}));
    REQUIRE_FALSE(
        fact("landingWithin", 32.0f, graph, feet, {{std::string(LandingNoise), {140.0f, 192.0f}}}));
    REQUIRE_FALSE(fact("landingWithin", 64.0f, graph, feet, {{"strike", {100.0f, 192.0f}}}));
    REQUIRE_FALSE(fact("landingWithin", 64.0f, graph, feet, {}));
}

TEST_CASE("A landing on my surface is one on the run I stand on", "[BehaviorFacts]")
{
    NavigationGraph graph = aWalkRun();
    graph.addNode(5, {192.0f, 288.0f});
    glm::vec2 feet(96.0f, 192.0f);

    REQUIRE(fact(
        "landingOnMySurface", true, graph, feet, {{std::string(LandingNoise), {288.0f, 192.0f}}}));
    REQUIRE_FALSE(fact(
        "landingOnMySurface", true, graph, feet, {{std::string(LandingNoise), {192.0f, 288.0f}}}));
    REQUIRE_FALSE(fact("landingOnMySurface", true, graph, feet, {{"strike", {96.0f, 192.0f}}}));
    REQUIRE(fact("landingOnMySurface", false, graph, feet, {}));
    REQUIRE_FALSE(fact(
        "landingOnMySurface", false, graph, feet, {{std::string(LandingNoise), {96.0f, 192.0f}}}));
}
