#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/player/movement_abilities/dash_ability.hpp"
#include "game/player/movement_abilities/movement_context.hpp"
#include "game/player/player_state.hpp"

using Catch::Approx;

TEST_CASE("DashAbility basic movement behavior", "[DashAbility]")
{
    PlayerState playerState;
    MovementContext movementContext;
    DashAbilityData dashAbilityData;
    DashAbility dashAbility(dashAbilityData);

    SECTION("Can dash left")
    {
        playerState.facingLeft = true;
        movementContext.inputIntentions.dashRequested = true;
        dashAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE(movementContext.dashVelocity.x == Approx(-dashAbilityData.dashSpeed));
        REQUIRE(playerState.dashing);
        REQUIRE(playerState.dashDirection == -1);
        movementContext = MovementContext();
        dashAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE(movementContext.dashVelocity.x == Approx(-dashAbilityData.dashSpeed));
        REQUIRE(playerState.dashing);
        REQUIRE(playerState.dashDirection == -1);
    }

    SECTION("Can dash right")
    {
        movementContext.inputIntentions.dashRequested = true;
        dashAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE(movementContext.dashVelocity.x == Approx(dashAbilityData.dashSpeed));
        REQUIRE(playerState.dashing);
        REQUIRE(playerState.dashDirection == 1);
        movementContext = MovementContext();
        dashAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE(movementContext.dashVelocity.x == Approx(dashAbilityData.dashSpeed));
        REQUIRE(playerState.dashing);
        REQUIRE(playerState.dashDirection == 1);        
    }

    SECTION("Dash ends after duration")
    {
        movementContext.inputIntentions.dashRequested = true;
        dashAbility.fixedUpdate(movementContext, playerState, dashAbilityData.dashDuration + 0.01f);
        REQUIRE_FALSE(playerState.dashing);
        REQUIRE(movementContext.dashVelocity.x == Approx(0.0f));
        REQUIRE_FALSE(playerState.dashing);
    }

    SECTION("Requesting to dash mid-dash should not change dashTimeLeft")
    {
        movementContext.inputIntentions.dashRequested = true;
        dashAbility.fixedUpdate(movementContext, playerState, 0.01f);
        movementContext = MovementContext();
        movementContext.inputIntentions.dashRequested = true;
        dashAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE(movementContext.dashVelocity.x == Approx(dashAbilityData.dashSpeed));
        REQUIRE(playerState.dashing);
        REQUIRE(playerState.dashTimeLeft == Approx(dashAbilityData.dashDuration - 0.02f));
    }

    SECTION("Dash cancels when touching wall")
    {
        movementContext.inputIntentions.dashRequested = true;
        dashAbility.fixedUpdate(movementContext, playerState, 0.01f);
        playerState.touchingLeftWall = true;
        movementContext = MovementContext();
        dashAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE(movementContext.dashVelocity.x == Approx(0.0f));
        REQUIRE_FALSE(playerState.dashing);
    }

    SECTION("Cannot dash while touching wall")
    {
        playerState.touchingLeftWall = true;
        movementContext.inputIntentions.dashRequested = true;
        dashAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE(movementContext.dashVelocity.x == Approx(0.0f));
        REQUIRE_FALSE(playerState.dashing);
    }
}