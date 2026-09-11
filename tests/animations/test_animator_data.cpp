#include <string>
#include <catch2/catch_test_macros.hpp>
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "animations/animator_facts.hpp"
#include "conditions/asked.hpp"
#include "conditions/fact_rows.hpp"

namespace
{
    AnimatorFacts factsOf(const Decided &decided, const Observed &observed)
    {
        return AnimatorFacts{decided, observed, false, ""};
    }
}

TEST_CASE("A condition that asks nothing always holds", "[AnimationLadderData]")
{
    Decided decided;
    Observed observed;

    REQUIRE(holds(AnimationWhenData{}, animatorRows(), factsOf(decided, observed)));
}

TEST_CASE("A condition holds only when every fact it asks about agrees", "[AnimationLadderData]")
{
    AnimationWhenData airborneAndRising;
    airborneAndRising["onGround"] = false;
    airborneAndRising["rising"] = true;
    Decided decided;

    Observed rising;
    rising.contacts.onGround = false;
    rising.velocity.y = -40.0f;
    REQUIRE(holds(airborneAndRising, animatorRows(), factsOf(decided, rising)));

    Observed risingOnGround = rising;
    risingOnGround.contacts.onGround = true;
    REQUIRE_FALSE(holds(airborneAndRising, animatorRows(), factsOf(decided, risingOnGround)));

    Observed airborneStill = rising;
    airborneStill.velocity.y = 0.0f;
    REQUIRE_FALSE(holds(airborneAndRising, animatorRows(), factsOf(decided, airborneStill)));
}

TEST_CASE("A condition may ask which state the machine is in", "[AnimationLadderData]")
{
    AnimationWhenData asleep;
    asleep["inState"] = std::string("sleep");
    Decided decided;
    Observed observed;

    REQUIRE(holds(asleep, animatorRows(), AnimatorFacts{decided, observed, false, "sleep"}));
    REQUIRE_FALSE(holds(asleep, animatorRows(), AnimatorFacts{decided, observed, false, "charge"}));
}
