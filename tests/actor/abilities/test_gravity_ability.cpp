#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "actor/abilities/gravity_ability.hpp"
#include "actor/abilities/gravity_ability_data.hpp"
#include "actor/abilities/ability_states.hpp"
#include "helpers/abilities.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

namespace
{
    AbilityStates fallingFor(GravityAbility &gravity, int ticks)
    {
        AbilityStates states;
        tick(gravity, InputIntentions{}, inTheAir(), states, ticks);
        return states;
    }
}

TEST_CASE("Gravity pulls harder every tick in the air", "[GravityAbility]")
{
    GravityAbilityData data;
    GravityAbility gravity(data);
    AbilityStates states;

    tick(gravity, InputIntentions{}, inTheAir(), states);
    REQUIRE(states.gravity.velocity.y == Approx(data.gravity * Step));

    tick(gravity, InputIntentions{}, inTheAir(), states);
    REQUIRE(states.gravity.velocity.y == Approx(2.0f * data.gravity * Step));
}

TEST_CASE("A fall gets no faster than its most", "[GravityAbility]")
{
    GravityAbilityData data;
    GravityAbility gravity(data);

    AbilityStates states = fallingFor(gravity, 1000);

    REQUIRE(states.gravity.velocity.y == Approx(data.maxFallSpeed));
}

TEST_CASE(
    "On the ground gravity pulls nothing, and a fall after starts from still",
    "[GravityAbility]")
{
    GravityAbilityData data;
    GravityAbility gravity(data);
    AbilityStates states = fallingFor(gravity, 10);

    tick(gravity, InputIntentions{}, onTheGround(), states);
    REQUIRE(states.gravity.velocity.y == 0.0f);

    tick(gravity, InputIntentions{}, inTheAir(), states);
    REQUIRE(states.gravity.velocity.y == Approx(data.gravity * Step));
}

TEST_CASE(
    "Gravity pulls nothing while hanging, sliding, pulling up a ledge or knocked back",
    "[GravityAbility]")
{
    GravityAbility gravity(GravityAbilityData{});

    AbilityStates hanging = fallingFor(gravity, 10);
    hanging.wallHang.active = true;
    tick(gravity, InputIntentions{}, inTheAir(), hanging);
    REQUIRE(hanging.gravity.velocity.y == 0.0f);

    AbilityStates sliding = fallingFor(gravity, 10);
    sliding.wallSlide.active = true;
    tick(gravity, InputIntentions{}, inTheAir(), sliding);
    REQUIRE(sliding.gravity.velocity.y == 0.0f);

    AbilityStates mantling = fallingFor(gravity, 10);
    mantling.mantle.active = true;
    tick(gravity, InputIntentions{}, inTheAir(), mantling);
    REQUIRE(mantling.gravity.velocity.y == 0.0f);

    AbilityStates knockedBack = fallingFor(gravity, 10);
    knockedBack.knockback.active = true;
    tick(gravity, InputIntentions{}, inTheAir(), knockedBack);
    REQUIRE(knockedBack.gravity.velocity.y == 0.0f);
}

TEST_CASE("Gravity that does not pull, or a fall that goes nowhere, is refused", "[GravityAbility]")
{
    GravityAbilityData noPull;
    noPull.gravity = 0.0f;
    REQUIRE_THROWS_WITH(
        GravityAbility(noPull), Catch::Matchers::ContainsSubstring("Gravity needs a pull above 0"));

    GravityAbilityData noFall;
    noFall.maxFallSpeed = 0.0f;
    REQUIRE_THROWS_WITH(
        GravityAbility(noFall),
        Catch::Matchers::ContainsSubstring("Gravity needs a fastest fall above 0"));
}
