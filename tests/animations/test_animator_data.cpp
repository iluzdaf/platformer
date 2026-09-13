#include <string>
#include <catch2/catch_test_macros.hpp>
#include "actor/abilities/ability_states.hpp"
#include "actor/observed.hpp"
#include "animations/animator_facts.hpp"
#include "conditions/asked.hpp"
#include "conditions/fact_rows.hpp"
#include "helpers/actor_facts.hpp"

TEST_CASE("A condition that asks nothing always holds", "[AnimationRules]")
{
    AbilityStates states;
    Observed observed;

    REQUIRE(holds(AnimationWhenData{}, animatorRows(), factsOf(states, observed)));
}

TEST_CASE("A condition holds only when every fact it asks about agrees", "[AnimationRules]")
{
    AnimationWhenData airborneAndRising;
    airborneAndRising["onGround"] = false;
    airborneAndRising["rising"] = true;
    AbilityStates states;

    Observed rising;
    rising.contacts.onGround = false;
    rising.velocity.y = -40.0f;
    REQUIRE(holds(airborneAndRising, animatorRows(), factsOf(states, rising)));

    Observed risingOnGround = rising;
    risingOnGround.contacts.onGround = true;
    REQUIRE_FALSE(holds(airborneAndRising, animatorRows(), factsOf(states, risingOnGround)));

    Observed airborneStill = rising;
    airborneStill.velocity.y = 0.0f;
    REQUIRE_FALSE(holds(airborneAndRising, animatorRows(), factsOf(states, airborneStill)));
}

TEST_CASE("A condition may ask which state the machine is in", "[AnimationRules]")
{
    AnimationWhenData asleep;
    asleep["inState"] = std::string("sleep");
    AbilityStates states;
    Observed observed;

    REQUIRE(holds(asleep, animatorRows(), factsOf(states, observed, "sleep")));
    REQUIRE_FALSE(holds(asleep, animatorRows(), factsOf(states, observed, "charge")));
}
