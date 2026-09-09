#include <string>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include "animations/animation_parameters.hpp"
#include "animations/animator_data.hpp"

TEST_CASE("A condition that asks nothing always holds", "[AnimatorData]")
{
    REQUIRE(holds(AnimationWhen{}, AnimationParameters{}));
}

TEST_CASE("A condition holds only when every parameter it asks about agrees", "[AnimatorData]")
{
    AnimationWhen airborneAndRising;
    airborneAndRising.onGround = false;
    airborneAndRising.rising = true;

    AnimationParameters rising;
    rising.onGround = false;
    rising.rising = true;
    REQUIRE(holds(airborneAndRising, rising));

    AnimationParameters risingOnGround = rising;
    risingOnGround.onGround = true;
    REQUIRE_FALSE(holds(airborneAndRising, risingOnGround));

    AnimationParameters airborneStill = rising;
    airborneStill.rising = false;
    REQUIRE_FALSE(holds(airborneAndRising, airborneStill));
}

TEST_CASE(
    "The usual ladder leaves from any state, in the order the picture has always been chosen",
    "[AnimatorData]")
{
    AnimatorData ladder = theUsualLadder();

    std::vector<std::string> order;
    for (const AnimationTransitionData &transition : ladder.transitions)
    {
        REQUIRE(transition.from.empty());
        order.push_back(transition.to);
    }

    REQUIRE(
        order == std::vector<std::string>{
                     "dead",
                     "knockback",
                     "attack",
                     "dash",
                     "climb",
                     "wallSlide",
                     "jump",
                     "fall",
                     "walk",
                     "idle"});
}

TEST_CASE("The usual ladder asks the ground before it asks about the air", "[AnimatorData]")
{
    AnimatorData ladder = theUsualLadder();

    for (const AnimationTransitionData &transition : ladder.transitions)
    {
        if (transition.to == "climb" || transition.to == "wallSlide" || transition.to == "jump" ||
            transition.to == "fall")
            REQUIRE(transition.when.onGround == false);
        if (transition.to == "walk" || transition.to == "idle")
            REQUIRE(transition.when.onGround == true);
    }
}
