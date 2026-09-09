#include <catch2/catch_test_macros.hpp>
#include <string>
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

TEST_CASE("A condition may ask which state the machine is in", "[AnimatorData]")
{
    AnimationWhen asleep;
    asleep.inState = "sleep";
    AnimationParameters sleeping;
    sleeping.inState = "sleep";
    AnimationParameters charging;
    charging.inState = "charge";

    REQUIRE(holds(asleep, sleeping));
    REQUIRE_FALSE(holds(asleep, charging));
    REQUIRE(holds(AnimationWhen{}, charging));
}
