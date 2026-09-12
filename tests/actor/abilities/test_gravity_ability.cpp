#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include "actor/abilities/gravity_ability.hpp"
#include "actor/abilities/gravity_ability_data.hpp"
#include "actor/decided.hpp"
#include "helpers/abilities.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

namespace
{
    Decided fallingFor(GravityAbility &gravity, int ticks)
    {
        Decided decided;
        tick(gravity, InputIntentions{}, inTheAir(), decided, ticks);
        return decided;
    }
}

TEST_CASE("Gravity pulls harder every tick in the air", "[GravityAbility]")
{
    GravityAbilityData data;
    GravityAbility gravity(data);
    Decided decided;

    tick(gravity, InputIntentions{}, inTheAir(), decided);
    REQUIRE(decided.gravity.velocity.y == Approx(data.gravity * Step));

    tick(gravity, InputIntentions{}, inTheAir(), decided);
    REQUIRE(decided.gravity.velocity.y == Approx(2.0f * data.gravity * Step));
}

TEST_CASE("A fall gets no faster than its most", "[GravityAbility]")
{
    GravityAbilityData data;
    GravityAbility gravity(data);

    Decided decided = fallingFor(gravity, 1000);

    REQUIRE(decided.gravity.velocity.y == Approx(data.maxFallSpeed));
}

TEST_CASE(
    "On the ground gravity pulls nothing, and a fall after starts from still",
    "[GravityAbility]")
{
    GravityAbilityData data;
    GravityAbility gravity(data);
    Decided decided = fallingFor(gravity, 10);

    tick(gravity, InputIntentions{}, onTheGround(), decided);
    REQUIRE(decided.gravity.velocity.y == 0.0f);

    tick(gravity, InputIntentions{}, inTheAir(), decided);
    REQUIRE(decided.gravity.velocity.y == Approx(data.gravity * Step));
}

TEST_CASE(
    "Gravity pulls nothing while hanging, sliding, pulling up a ledge or knocked back",
    "[GravityAbility]")
{
    GravityAbility gravity(GravityAbilityData{});

    Decided hanging = fallingFor(gravity, 10);
    hanging.wallHang.active = true;
    tick(gravity, InputIntentions{}, inTheAir(), hanging);
    REQUIRE(hanging.gravity.velocity.y == 0.0f);

    Decided sliding = fallingFor(gravity, 10);
    sliding.wallSlide.active = true;
    tick(gravity, InputIntentions{}, inTheAir(), sliding);
    REQUIRE(sliding.gravity.velocity.y == 0.0f);

    Decided mantling = fallingFor(gravity, 10);
    mantling.mantle.active = true;
    tick(gravity, InputIntentions{}, inTheAir(), mantling);
    REQUIRE(mantling.gravity.velocity.y == 0.0f);

    Decided knockedBack = fallingFor(gravity, 10);
    knockedBack.knockback.active = true;
    tick(gravity, InputIntentions{}, inTheAir(), knockedBack);
    REQUIRE(knockedBack.gravity.velocity.y == 0.0f);
}
