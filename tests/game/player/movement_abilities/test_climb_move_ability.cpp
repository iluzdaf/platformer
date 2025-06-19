#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/player/movement_abilities/climb_move_ability.hpp"
#include "game/player/movement_abilities/climb_move_ability_data.hpp"
#include "game/player/player_state.hpp"
#include "test_helpers/mock_player.hpp"
using Catch::Approx;

TEST_CASE("ClimbMoveAbility basic movement behaviour", "[ClimbMoveAbility]")
{
    PlayerState playerState;
    MockPlayer movementContext;
    ClimbMoveAbilityData climbMoveAbilityData;
    ClimbMoveAbility climbMoveAbility(climbMoveAbilityData);

    SECTION("Can climb up")
    {
        playerState.climbing = true;
        climbMoveAbility.tryAscend(movementContext, playerState);
        climbMoveAbility.fixedUpdate(movementContext, playerState, 0.1f);
        REQUIRE(movementContext.getVelocity().y == Approx(-climbMoveAbility.getClimbSpeed()));
    }

    SECTION("Can climb down")
    {
        playerState.climbing = true;
        climbMoveAbility.tryDescend(movementContext, playerState);
        climbMoveAbility.fixedUpdate(movementContext, playerState, 0.1f);
        climbMoveAbility.update(movementContext, playerState, 0.1f);
        REQUIRE(movementContext.getVelocity().y == Approx(climbMoveAbility.getClimbSpeed()));
    }

    SECTION("Cannot climb up if not climbing")
    {
        climbMoveAbility.tryAscend(movementContext, playerState);
        climbMoveAbility.fixedUpdate(movementContext, playerState, 0.01f);
        climbMoveAbility.update(movementContext, playerState, 0.01f);
        REQUIRE(movementContext.getVelocity().y == Approx(0.0f));
    }

    SECTION("Cannot climb down if not climbing")
    {
        climbMoveAbility.tryDescend(movementContext, playerState);
        climbMoveAbility.fixedUpdate(movementContext, playerState, 0.01f);
        climbMoveAbility.update(movementContext, playerState, 0.01f);
        REQUIRE(movementContext.getVelocity().y == Approx(0.0f));
    }

    SECTION("If both directions requested, no movement applied")
    {
        playerState.climbing = true;
        climbMoveAbility.tryAscend(movementContext, playerState);
        climbMoveAbility.tryDescend(movementContext, playerState);
        climbMoveAbility.fixedUpdate(movementContext, playerState, 0.1f);
        climbMoveAbility.update(movementContext, playerState, 0.1f);
        REQUIRE(movementContext.getVelocity().y == Approx(0.0f));
    }

    SECTION("If no direction requested, no movement applied")
    {
        playerState.climbing = true;
        climbMoveAbility.fixedUpdate(movementContext, playerState, 0.1f);
        climbMoveAbility.update(movementContext, playerState, 0.1f);
        REQUIRE(movementContext.getVelocity().y == Approx(0.0f));
    }

    SECTION("Movement requests are cleared after update")
    {
        playerState.climbing = true;
        climbMoveAbility.tryAscend(movementContext, playerState);
        climbMoveAbility.fixedUpdate(movementContext, playerState, 0.1f);
        climbMoveAbility.update(movementContext, playerState, 0.1f);
        movementContext.setVelocity({0, 0});
        climbMoveAbility.fixedUpdate(movementContext, playerState, 0.1f);
        climbMoveAbility.update(movementContext, playerState, 0.1f);
        REQUIRE(movementContext.getVelocity().y == Approx(0.0f));
    }
}