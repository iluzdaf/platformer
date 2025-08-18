#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "agent/agent_state.hpp"
#include "agent/movement_abilities/gravity_ability.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

TEST_CASE("GravityAbility basic movement behaviour", "[GravityAbility]")
{
    InputIntentions inputIntentions;
    AgentState state;
    GravityAbilityData data;
    GravityAbility ability(data);

    SECTION("Gravity accumulates when airborne")
    {
        state.onGround = false;
        state.climbing = false;
        state.wallSliding = false;
        ability.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.gravityVelocity.y == Approx(data.gravity));
        ability.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.gravityVelocity.y == Approx(2 * data.gravity));
    }

    SECTION("Gravity is capped at max fall speed")
    {
        state.onGround = false;
        state.climbing = false;
        state.wallSliding = false;

        int iterationsToMaxFallSpeed = static_cast<int>(std::ceil(data.maxFallSpeed / data.gravity));
        for (int i = 0; i < iterationsToMaxFallSpeed + 10; ++i)
        {
            ability.applyMovement(0.01f, inputIntentions, state);
        }

        REQUIRE(state.gravityVelocity.y == Approx(data.maxFallSpeed));
    }

    SECTION("Gravity resets to 0 if onGround, climbing or wallSliding")
    {
    }
}