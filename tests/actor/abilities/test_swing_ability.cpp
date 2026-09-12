#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/swing_ability.hpp"
#include "actor/abilities/swing_ability_data.hpp"
#include "actor/abilities/swing_ability_state.hpp"
#include "actor/ability_states.hpp"
#include "actor/observed.hpp"
#include "input/input_intentions.hpp"

namespace
{
    constexpr float Step = 0.01f;

    InputIntentions pressingAttack(float x = 0.0f)
    {
        InputIntentions intentions;
        intentions.attack = std::string(SwingAttack);
        intentions.direction.x = x;
        return intentions;
    }

    SwingAbilityData timedAs(float windup, float strike, float recovery)
    {
        SwingAbilityData data;
        data.windupDuration = windup;
        data.strikeDuration = strike;
        data.recoveryDuration = recovery;
        return data;
    }

    void wait(SwingAbility &ability, AbilityStates &states, int steps)
    {
        for (int step = 0; step < steps; ++step)
            ability.decide(Step, InputIntentions{}, Observed{}, states);
    }

    Observed facingLeft()
    {
        Observed observed;
        observed.facingLeft = true;
        return observed;
    }
}

TEST_CASE(
    "A swing winds up, strikes, recovers and rests, each for as long as it says",
    "[SwingAbility]")
{
    AbilityStates states;
    SwingAbility ability(timedAs(0.045f, 0.1f, 0.03f));

    ability.decide(Step, pressingAttack(), Observed{}, states);
    REQUIRE(states.swing.phase == SwingPhase::Windup);
    REQUIRE(states.swing.swinging());
    REQUIRE_FALSE(states.swing.striking());

    wait(ability, states, 4);
    REQUIRE(states.swing.phase == SwingPhase::Windup);
    wait(ability, states, 1);
    REQUIRE(states.swing.striking());

    wait(ability, states, 9);
    REQUIRE(states.swing.striking());
    wait(ability, states, 1);
    REQUIRE(states.swing.phase == SwingPhase::Recovery);
    REQUIRE(states.swing.swinging());

    wait(ability, states, 2);
    REQUIRE(states.swing.phase == SwingPhase::Recovery);
    wait(ability, states, 1);
    REQUIRE(states.swing.phase == SwingPhase::Idle);
    REQUIRE_FALSE(states.swing.swinging());
}

TEST_CASE("A swing with no windup strikes on the tick after it starts", "[SwingAbility]")
{
    AbilityStates states;
    SwingAbility ability(timedAs(0.0f, 0.1f, 0.1f));

    ability.decide(Step, pressingAttack(), Observed{}, states);
    REQUIRE(states.swing.phase == SwingPhase::Windup);

    wait(ability, states, 1);
    REQUIRE(states.swing.striking());
}

TEST_CASE("A long tick carries a swing through every phase it covers", "[SwingAbility]")
{
    AbilityStates states;
    SwingAbility ability(timedAs(0.05f, 0.05f, 0.05f));
    ability.decide(Step, pressingAttack(), Observed{}, states);

    ability.decide(0.12f, InputIntentions{}, Observed{}, states);

    REQUIRE(states.swing.phase == SwingPhase::Recovery);
}

TEST_CASE("A swing says so once, and pressing again mid-swing starts nothing", "[SwingAbility]")
{
    AbilityStates states;
    SwingAbility ability(SwingAbilityData{});

    ability.decide(Step, pressingAttack(), Observed{}, states);
    REQUIRE(states.swing.emit);

    for (int step = 0; step < 15; ++step)
    {
        ability.decide(Step, pressingAttack(), Observed{}, states);
        REQUIRE_FALSE(states.swing.emit);
    }
    REQUIRE(states.swing.striking());
}

TEST_CASE("A swing goes where it was asked, else where the actor faces", "[SwingAbility]")
{
    SwingAbility ability(SwingAbilityData{});

    AbilityStates asked;
    ability.decide(Step, pressingAttack(-1.0f), Observed{}, asked);
    REQUIRE(asked.swing.direction == -1.0f);

    AbilityStates facing;
    ability.decide(Step, pressingAttack(), facingLeft(), facing);
    REQUIRE(facing.swing.direction == -1.0f);

    AbilityStates ahead;
    ability.decide(Step, pressingAttack(), Observed{}, ahead);
    REQUIRE(ahead.swing.direction == 1.0f);
}

TEST_CASE("A swing carries its reach and damage", "[SwingAbility]")
{
    SwingAbilityData data;
    data.reach = glm::vec2(20.0f, 6.0f);
    data.damage = 3;
    SwingAbility ability(data);
    AbilityStates states;

    ability.decide(Step, pressingAttack(), Observed{}, states);

    REQUIRE(states.swing.reach == glm::vec2(20.0f, 6.0f));
    REQUIRE(states.swing.damage == 3);
}

TEST_CASE("A swing cannot start while dashing, and a knockback cuts one short", "[SwingAbility]")
{
    SwingAbility ability(SwingAbilityData{});

    AbilityStates dashing;
    dashing.dash.active = true;
    ability.decide(Step, pressingAttack(), Observed{}, dashing);
    REQUIRE(dashing.swing.phase == SwingPhase::Idle);

    AbilityStates knocked;
    ability.decide(Step, pressingAttack(), Observed{}, knocked);
    wait(ability, knocked, 11);
    REQUIRE(knocked.swing.striking());
    knocked.knockback.active = true;
    ability.decide(Step, InputIntentions{}, Observed{}, knocked);
    REQUIRE(knocked.swing.phase == SwingPhase::Idle);
}

TEST_CASE("A swing with nothing to it is refused", "[SwingAbility]")
{
    SwingAbilityData noReach;
    noReach.reach = glm::vec2(0.0f, 10.0f);
    REQUIRE_THROWS_WITH(SwingAbility(noReach), Catch::Matchers::ContainsSubstring("reach"));

    SwingAbilityData noDamage;
    noDamage.damage = 0;
    REQUIRE_THROWS_WITH(SwingAbility(noDamage), Catch::Matchers::ContainsSubstring("damage"));

    REQUIRE_THROWS_WITH(
        SwingAbility(timedAs(0.1f, 0.0f, 0.1f)), Catch::Matchers::ContainsSubstring("strike"));
    REQUIRE_THROWS_WITH(
        SwingAbility(timedAs(-0.1f, 0.1f, 0.1f)), Catch::Matchers::ContainsSubstring("wind up"));
    REQUIRE_THROWS_WITH(
        SwingAbility(timedAs(0.1f, 0.1f, -0.1f)), Catch::Matchers::ContainsSubstring("recover"));
}

TEST_CASE("A swing is not started by an attack that is not a swing", "[SwingAbility]")
{
    SwingAbility ability(SwingAbilityData{});
    AbilityStates states;
    InputIntentions pouncing;
    pouncing.attack = "pounce";

    ability.decide(Step, pouncing, Observed{}, states);

    REQUIRE(states.swing.phase == SwingPhase::Idle);
}
