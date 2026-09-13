#include <concepts>
#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <glaze/glaze.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/ability_states.hpp"
#include "actor/abilities/swing_ability_state.hpp"
#include "actor/actor_facts.hpp"
#include "actor/actor_fact_rows.hpp"
#include "actor/behaviors/senses_data.hpp"
#include "actor/observed.hpp"
#include "conditions/asked.hpp"
#include "conditions/fact_rows.hpp"
#include "helpers/actor_facts.hpp"
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

    ActorFacts sensing(ActorFacts context, const SensesData &senses)
    {
        context.senses = &senses;
        return context;
    }

    bool holds(std::string_view fact, const ActorFacts &context)
    {
        return rowNamed(actorRows(), fact)->holds(true, context);
    }

    bool fact(
        const char *name,
        const Asked &asked,
        const AbilityStates &states,
        const Observed &observed)
    {
        return rowNamed(actorRows(), name)->holds(asked, factsOf(states, observed));
    }
}

TEST_CASE(
    "An actor's rows are the facts anything may ask about it, in the order they are worded",
    "[ActorFactRows]")
{
    std::string names;
    for (const FactRow<ActorFacts> &row : actorRows())
        names += std::string(row.name) + " ";

    REQUIRE(
        names == "alive knockback swinging dashing pouncing charging onGround climbing onWall "
                 "rising falling moving threatOnMySurface cornered threatClose threatInReach ");
    REQUIRE(rowNamed(actorRows(), "somersault") == nullptr);
}

TEST_CASE("With no threat, nothing about the threat holds", "[ActorFactRows]")
{
    NavigationGraph navigationGraph = aRunAndALedgeBelow();
    ActorFacts alone = sensing(standingAt(navigationGraph, {192.0f, 192.0f}), RatSenses);

    REQUIRE_FALSE(holds("threatOnMySurface", alone));
    REQUIRE_FALSE(holds("cornered", alone));
    REQUIRE_FALSE(holds("threatClose", alone));
    REQUIRE_FALSE(holds("threatInReach", alone));
}

TEST_CASE(
    "A threat on the run under my feet is on my surface, and one on a ledge below is not",
    "[ActorFactRows]")
{
    NavigationGraph navigationGraph = aRunAndALedgeBelow();

    REQUIRE(holds(
        "threatOnMySurface",
        standingAt(navigationGraph, {0.0f, 192.0f}, glm::vec2(384.0f, 192.0f))));
    REQUIRE_FALSE(holds(
        "threatOnMySurface",
        standingAt(navigationGraph, {96.0f, 192.0f}, glm::vec2(100.0f, 288.0f))));
}

TEST_CASE("A threat is close within my close, and in reach within my reach", "[ActorFactRows]")
{
    NavigationGraph navigationGraph = aRunAndALedgeBelow();
    glm::vec2 here{192.0f, 192.0f};

    ActorFacts thirtyAway =
        sensing(standingAt(navigationGraph, here, here + glm::vec2(30.0f, 0.0f)), RatSenses);
    REQUIRE(holds("threatClose", thirtyAway));
    REQUIRE_FALSE(holds("threatInReach", thirtyAway));

    ActorFacts twentyAway =
        sensing(standingAt(navigationGraph, here, here - glm::vec2(20.0f, 0.0f)), RatSenses);
    REQUIRE(holds("threatClose", twentyAway));
    REQUIRE(holds("threatInReach", twentyAway));

    ActorFacts fiftyAway =
        sensing(standingAt(navigationGraph, here, here + glm::vec2(50.0f, 0.0f)), RatSenses);
    REQUIRE_FALSE(holds("threatClose", fiftyAway));
}

TEST_CASE("Cornered is having nowhere further from the threat to go", "[ActorFactRows]")
{
    NavigationGraph navigationGraph = aRunAndALedgeBelow();

    REQUIRE(
        holds("cornered", standingAt(navigationGraph, {0.0f, 192.0f}, glm::vec2(40.0f, 192.0f))));
    REQUIRE_FALSE(holds(
        "cornered", standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(240.0f, 192.0f))));
}

TEST_CASE("Each fact follows the one thing it watches", "[ActorFactRows]")
{
    AbilityStates states;
    Observed observed;

    REQUIRE(fact("alive", true, states, observed));
    observed.alive = false;
    REQUIRE(fact("alive", false, states, observed));
    observed.alive = true;

    states.knockback.active = true;
    REQUIRE(fact("knockback", true, states, observed));
    states.knockback.active = false;

    states.swing.phase = SwingPhase::Windup;
    REQUIRE(fact("swinging", true, states, observed));
    states.swing.phase = SwingPhase::Idle;

    states.dash.active = true;
    REQUIRE(fact("dashing", true, states, observed));
    states.dash.active = false;

    states.pounce.active = true;
    REQUIRE(fact("pouncing", true, states, observed));
    REQUIRE(fact("dashing", false, states, observed));
    states.pounce.active = false;

    observed.velocity = glm::vec2(0.0f, -40.0f);
    REQUIRE(fact("rising", true, states, observed));
    REQUIRE(fact("falling", false, states, observed));

    observed.velocity = glm::vec2(0.0f, 40.0f);
    REQUIRE(fact("falling", true, states, observed));
    REQUIRE(fact("rising", false, states, observed));

    observed.velocity = glm::vec2(0.2f, 0.0f);
    REQUIRE(fact("moving", true, states, observed));
    observed.velocity = glm::vec2(0.05f, 0.0f);
    REQUIRE(fact("moving", false, states, observed));
}

TEST_CASE("Climbing is a hang that is moving, and a wall is a slide or a hang", "[ActorFactRows]")
{
    AbilityStates states;
    Observed observed;

    states.wallSlide.active = true;
    REQUIRE(fact("onWall", true, states, observed));
    REQUIRE(fact("climbing", false, states, observed));
    states.wallSlide.active = false;

    states.wallHang.active = true;
    REQUIRE(fact("onWall", true, states, observed));
    REQUIRE(fact("climbing", false, states, observed));

    states.wallClimb.velocity.y = -40.0f;
    REQUIRE(fact("climbing", true, states, observed));

    states.wallHang.active = false;
    states.wallSlide.active = true;
    REQUIRE(fact("climbing", false, states, observed));
}

TEST_CASE("Without its ability states, an actor is doing none of them", "[ActorFactRows]")
{
    NavigationGraph navigationGraph = aWalkRun();
    ActorFacts unknown = standingAt(navigationGraph, {0.0f, 192.0f});

    REQUIRE_FALSE(holds("charging", unknown));
    REQUIRE_FALSE(holds("onWall", unknown));
}

namespace
{
    template <class T>
    concept CanBeActive = requires(const T &state) {
        { state.active } -> std::convertible_to<bool>;
    };

    template <std::size_t... I>
    std::vector<std::string_view> abilitiesThatCanBeActive(std::index_sequence<I...>)
    {
        std::vector<std::string_view> names;
        (
            [&]
            {
                if constexpr (CanBeActive<typename glz::reflect<AbilityStates>::template type<I>>)
                    names.push_back(glz::reflect<AbilityStates>::keys[I]);
            }(),
            ...);
        return names;
    }
}

TEST_CASE("Every ability that can be active is a fact, or says why it is not", "[ActorFactRows]")
{
    const std::map<std::string_view, std::string_view> notAFactBecause{
        {"jump", "rising already says it"},
        {"wallJump", "it is a jump"},
        {"mantle", "nothing asks about a mantle yet; add a row when something does"},
        {"bite", "a bite is always ready, so asking would never tell anything apart"}};

    std::vector<std::string_view> flagged =
        abilitiesThatCanBeActive(std::make_index_sequence<glz::reflect<AbilityStates>::size>{});
    REQUIRE_FALSE(flagged.empty());

    for (std::string_view ability : flagged)
    {
        bool covered = false;
        for (const FactRow<ActorFacts> &row : actorRows())
            covered = covered || row.covers == ability;

        INFO("ability \"" << ability << "\" can be active, and no row watches it nor says why not");
        REQUIRE((covered || notAFactBecause.contains(ability)));
    }
}

TEST_CASE("A row that says it watches a flag really reads it", "[ActorFactRows]")
{
    for (const FactRow<ActorFacts> &row : actorRows())
    {
        if (row.covers.empty())
            continue;

        AbilityStates off;
        AbilityStates on;
        if (row.covers == "dash")
            on.dash.active = true;
        else if (row.covers == "knockback")
            on.knockback.active = true;
        else if (row.covers == "pounce")
            on.pounce.active = true;
        else if (row.covers == "charge")
            on.charge.active = true;
        else if (row.covers == "wallSlide")
            on.wallSlide.active = true;
        else if (row.covers == "wallHang")
        {
            on.wallHang.active = true;
            on.wallClimb.velocity.y = -1.0f;
        }
        else
            FAIL(
                "row \"" << row.name << "\" covers \"" << row.covers
                         << "\", which this test does not know how to switch on");

        Observed observed;
        INFO("row \"" << row.name << "\"");
        REQUIRE(row.holds(true, factsOf(on, observed)));
        REQUIRE_FALSE(row.holds(true, factsOf(off, observed)));
    }
}
