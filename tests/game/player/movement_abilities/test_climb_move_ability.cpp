#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/player/movement_abilities/climb_move_ability.hpp"
#include "game/player/movement_abilities/movement_context.hpp"
#include "game/player/player_state.hpp"

using Catch::Approx;

TEST_CASE("ClimbMoveAbility basic movement behaviour", "[ClimbMoveAbility]")
{
    PlayerState playerState;
    MovementContext movementContext;
    ClimbMoveAbilityData climbMoveAbilityData;
    ClimbMoveAbility climbMoveAbility(climbMoveAbilityData);

    SECTION("Can climb up")
    {
        playerState.climbing = true;
        movementContext.inputIntentions.direction.y = -1;
        climbMoveAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE(movementContext.climbMoveVelocity.y == Approx(-climbMoveAbilityData.climbSpeed));
    }

    SECTION("Can climb down")
    {
        playerState.climbing = true;
        movementContext.inputIntentions.direction.y = 1;
        climbMoveAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE(movementContext.climbMoveVelocity.y == Approx(climbMoveAbilityData.climbSpeed));
    }

    SECTION("Cannot climb up if not climbing")
    {
        movementContext.inputIntentions.direction.y = -1;
        climbMoveAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE(movementContext.climbMoveVelocity.y == Approx(0.0f));
    }

    SECTION("Cannot climb down if not climbing")
    {
        movementContext.inputIntentions.direction.y = 1;
        climbMoveAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE(movementContext.climbMoveVelocity.y == Approx(0.0f));
    }

    SECTION("If no direction requested, no movement applied")
    {
        playerState.climbing = true;
        climbMoveAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE(movementContext.climbMoveVelocity.y == Approx(0.0f));
    }
}