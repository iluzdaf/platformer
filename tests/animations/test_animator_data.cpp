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
