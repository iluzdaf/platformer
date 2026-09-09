#include <catch2/catch_test_macros.hpp>
#include "actor/abilities/pounce_ability_data.hpp"
#include "actor/abilities/swing_ability_data.hpp"
#include <catch2/matchers/catch_matchers_string.hpp>
#include <optional>
#include "actor/actor_behavior_context.hpp"
#include "actor/behaviors/attack_behavior.hpp"
#include "actor/behaviors/attack_behavior_data.hpp"
#include "helpers/behaviour_context.hpp"
#include "input/input_intentions.hpp"
#include "navigation/navigation_graph.hpp"

namespace
{
    AttackBehaviorData attackingWith(const char *with)
    {
        AttackBehaviorData data;
        data.with = with;
        return data;
    }
}

TEST_CASE("An attack with a pounce asks to leap towards the threat", "[AttackBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    AttackBehavior behavior(attackingWith("pounce"));

    InputIntentions leap = behavior.decide(
        0.01f, standingAt(navigationGraph, {96.0f, 192.0f}, glm::vec2(160.0f, 192.0f)));

    REQUIRE(leap.attack == PounceAttack);
    REQUIRE(leap.direction.x == 1.0f);
}

TEST_CASE("An attack with a swing asks to swing towards the threat", "[AttackBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    AttackBehavior behavior(attackingWith("swing"));

    InputIntentions swing = behavior.decide(
        0.01f, standingAt(navigationGraph, {96.0f, 192.0f}, glm::vec2(40.0f, 192.0f)));

    REQUIRE(swing.attack == SwingAttack);
    REQUIRE(swing.direction.x == -1.0f);
}

TEST_CASE("An attack asks once until it is reset", "[AttackBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    AttackBehavior behavior(attackingWith("pounce"));
    behavior.decide(0.01f, standingAt(navigationGraph, {96.0f, 192.0f}, glm::vec2(160.0f, 192.0f)));

    InputIntentions again = behavior.decide(
        0.01f, standingAt(navigationGraph, {120.0f, 192.0f}, glm::vec2(160.0f, 192.0f)));
    REQUIRE(again.attack.empty());

    behavior.reset();
    InputIntentions afterReset = behavior.decide(
        0.01f, standingAt(navigationGraph, {120.0f, 192.0f}, glm::vec2(160.0f, 192.0f)));
    REQUIRE(afterReset.attack == PounceAttack);
}

TEST_CASE("An attack does not ask from the air", "[AttackBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    AttackBehavior behavior(attackingWith("pounce"));
    ActorBehaviorContext inTheAir = airborneAt(navigationGraph, {110.0f, 170.0f});
    inTheAir.threatFeet = glm::vec2(160.0f, 192.0f);

    InputIntentions nothing = behavior.decide(0.01f, inTheAir);

    REQUIRE(nothing.attack.empty());
}

TEST_CASE("An attack with nothing to attack asks for nothing", "[AttackBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    AttackBehavior behavior(attackingWith("pounce"));

    InputIntentions nothing = behavior.decide(0.01f, standingAt(navigationGraph, {96.0f, 192.0f}));

    REQUIRE(nothing.attack.empty());
    REQUIRE(nothing.direction.x == 0.0f);
}

TEST_CASE("An attack made with nothing is refused", "[AttackBehavior]")
{
    REQUIRE_THROWS_WITH(
        AttackBehavior(AttackBehaviorData{}), Catch::Matchers::ContainsSubstring("made with"));
}

TEST_CASE("An attack hands the name of what it is made with straight through", "[AttackBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    AttackBehavior behavior(attackingWith("headbutt"));

    InputIntentions asked = behavior.decide(
        0.01f, standingAt(navigationGraph, {96.0f, 192.0f}, glm::vec2(160.0f, 192.0f)));

    REQUIRE(asked.attack == "headbutt");
}
