#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "agent/agent_state.hpp"
#include "agent/movement_abilities/move_ability.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

TEST_CASE("MoveAbility basic movement behavior", "[MoveAbility]")
{
    AgentState state;
    InputIntentions inputIntentions;
    MoveAbilityData moveAbilityData;
    MoveAbility moveAbility(moveAbilityData);

    SECTION("Can move left")
    {
        inputIntentions.direction.x = -1;
        moveAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.moveVelocity.x == Approx(-moveAbilityData.moveSpeed));
    }

    SECTION("Can move right")
    {
        inputIntentions.direction.x = 1;
        moveAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.moveVelocity.x == Approx(moveAbilityData.moveSpeed));
    }

    SECTION("If no direction requested, no movement applied")
    {
        moveAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.moveVelocity.x == Approx(0.0f));
    }
}