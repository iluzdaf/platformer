#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cmath>
#include "game/actor/actor_motion_state.hpp"
#include "game/actor/abilities/gravity_ability.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

TEST_CASE("GravityAbility basic movement behaviour", "[GravityAbility]")
{
    InputIntentions inputIntentions;
    ActorMotionState state;
    GravityAbilityData data;
    GravityAbility ability(data);

    SECTION("Gravity accumulates when airborne")
    {
        state.onGround = false;
        state.climbing = false;
        state.wallSliding = false;
        ability.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.gravityVelocity.y == Approx(data.gravity * 0.01f));
        ability.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.gravityVelocity.y == Approx(2 * data.gravity * 0.01f));
    }

    SECTION("Gravity is capped at max fall speed")
    {
        state.onGround = false;
        state.climbing = false;
        state.wallSliding = false;

        int iterationsToMaxFallSpeed = static_cast<int>(std::ceil(data.maxFallSpeed / (data.gravity * 0.01f)));
        for (int i = 0; i < iterationsToMaxFallSpeed + 10; ++i)
        {
            ability.applyMovement(0.01f, inputIntentions, state);
        }

        REQUIRE(state.gravityVelocity.y == Approx(data.maxFallSpeed));
    }

    SECTION("Gravity resets to 0 if onGround, climbing or wallSliding")
    {
        state.onGround = false;
        ability.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.gravityVelocity.y > 0.0f);

        state.onGround = true;
        ability.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.gravityVelocity.y == 0.0f);

        state.onGround = false;
        state.climbing = true;
        ability.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.gravityVelocity.y == 0.0f);

        state.climbing = false;
        state.wallSliding = true;
        ability.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.gravityVelocity.y == 0.0f);
    }
}