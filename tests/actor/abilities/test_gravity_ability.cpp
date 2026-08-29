#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cmath>
#include "actor/abilities/gravity_ability_data.hpp"
#include "actor/actor_motion_state.hpp"
#include "actor/abilities/gravity_ability.hpp"
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
        state.contacts.onGround = false;
        state.wallHang.active = false;
        state.wallSlide.active = false;
        ability.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.gravity.velocity.y == Approx(data.gravity * 0.01f));
        ability.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.gravity.velocity.y == Approx(2 * data.gravity * 0.01f));
    }

    SECTION("Gravity is capped at max fall speed")
    {
        state.contacts.onGround = false;
        state.wallHang.active = false;
        state.wallSlide.active = false;

        int iterationsToMaxFallSpeed =
            static_cast<int>(std::ceil(data.maxFallSpeed / (data.gravity * 0.01f)));
        for (int i = 0; i < iterationsToMaxFallSpeed + 10; ++i)
        {
            ability.applyMovement(0.01f, inputIntentions, state);
        }

        REQUIRE(state.gravity.velocity.y == Approx(data.maxFallSpeed));
    }

    SECTION("Gravity resets to 0 if onGround, climbing or wallSliding")
    {
        state.contacts.onGround = false;
        ability.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.gravity.velocity.y > 0.0f);

        state.contacts.onGround = true;
        ability.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.gravity.velocity.y == 0.0f);

        state.contacts.onGround = false;
        state.wallHang.active = true;
        ability.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.gravity.velocity.y == 0.0f);

        state.wallHang.active = false;
        state.wallSlide.active = true;
        ability.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.gravity.velocity.y == 0.0f);
    }
}