#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "agent/agent_state.hpp"
#include "agent/movement_abilities/climb_move_ability.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

TEST_CASE("ClimbMoveAbility basic movement behaviour", "[ClimbMoveAbility]")
{
    AgentState state;
    InputIntentions inputIntentions;
    ClimbMoveAbilityData climbMoveAbilityData;
    ClimbMoveAbility climbMoveAbility(climbMoveAbilityData);

    SECTION("Can climb up")
    {
        state.climbing = true;
        inputIntentions.direction.y = -1;
        climbMoveAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.climbMoveVelocity.y == Approx(-climbMoveAbilityData.climbSpeed));
    }

    SECTION("Can climb down")
    {
        state.climbing = true;
        inputIntentions.direction.y = 1;
        climbMoveAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.climbMoveVelocity.y == Approx(climbMoveAbilityData.climbSpeed));
    }

    SECTION("Cannot climb up if not climbing")
    {
        inputIntentions.direction.y = -1;
        climbMoveAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.climbMoveVelocity.y == Approx(0.0f));
    }

    SECTION("Cannot climb down if not climbing")
    {
        inputIntentions.direction.y = 1;
        climbMoveAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.climbMoveVelocity.y == Approx(0.0f));
    }

    SECTION("If no direction requested, no movement applied")
    {
        state.climbing = true;
        climbMoveAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.climbMoveVelocity.y == Approx(0.0f));
    }
}