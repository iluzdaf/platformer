#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "agent/agent_state.hpp"
#include "agent/movement_abilities/dash_ability.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

TEST_CASE("DashAbility basic movement behavior", "[DashAbility]")
{
    AgentState state;
    InputIntentions inputIntentions;
    DashAbilityData dashAbilityData;
    DashAbility dashAbility(dashAbilityData);

    SECTION("Can dash left")
    {
        inputIntentions.direction.x = -1;
        inputIntentions.dashRequested = true;
        dashAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.dashVelocity.x == Approx(-dashAbilityData.dashSpeed));
        REQUIRE(state.dashing);
        REQUIRE(state.dashDirection == -1);
        inputIntentions = InputIntentions();
        dashAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.dashVelocity.x == Approx(-dashAbilityData.dashSpeed));
        REQUIRE(state.dashing);
        REQUIRE(state.dashDirection == -1);
    }

    SECTION("Can dash right")
    {
        inputIntentions.direction.x = 1;
        inputIntentions.dashRequested = true;
        dashAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.dashVelocity.x == Approx(dashAbilityData.dashSpeed));
        REQUIRE(state.dashing);
        REQUIRE(state.dashDirection == 1);
        inputIntentions = InputIntentions();
        dashAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.dashVelocity.x == Approx(dashAbilityData.dashSpeed));
        REQUIRE(state.dashing);
        REQUIRE(state.dashDirection == 1);
    }

    SECTION("Cannot dash if no direction given")
    {
        inputIntentions.dashRequested = true;
        dashAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.dashing);
        REQUIRE(state.dashVelocity.x == Approx(0.0f));
    }

    SECTION("Dash ends after duration")
    {
        inputIntentions.direction.x = -1;
        inputIntentions.dashRequested = true;
        dashAbility.applyMovement(dashAbilityData.dashDuration + 0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.dashing);
        REQUIRE(state.dashVelocity.x == Approx(0.0f));
    }

    SECTION("Requesting to dash mid-dash should not change dashTimeLeft")
    {
        inputIntentions.direction.x = 1;
        inputIntentions.dashRequested = true;
        dashAbility.applyMovement(0.01f, inputIntentions, state);
        inputIntentions = InputIntentions();
        inputIntentions.dashRequested = true;
        dashAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.dashVelocity.x == Approx(dashAbilityData.dashSpeed));
        REQUIRE(state.dashing);
        REQUIRE(state.dashTimeLeft == Approx(dashAbilityData.dashDuration - 0.02f));
    }

    SECTION("Dash cancels when touching wall")
    {
        inputIntentions.direction.x = -1;
        inputIntentions.dashRequested = true;
        dashAbility.applyMovement(0.01f, inputIntentions, state);
        state.touchingLeftWall = true;
        inputIntentions = InputIntentions();
        dashAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.dashVelocity.x == Approx(0.0f));
        REQUIRE_FALSE(state.dashing);
    }

    SECTION("Cannot dash while touching wall")
    {
        state.touchingLeftWall = true;
        inputIntentions.direction.x = 1;
        inputIntentions.dashRequested = true;
        dashAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.dashVelocity.x == Approx(0.0f));
        REQUIRE_FALSE(state.dashing);
    }
}