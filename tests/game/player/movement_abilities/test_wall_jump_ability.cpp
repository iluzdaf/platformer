#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/player/movement_abilities/wall_jump_ability.hpp"
#include "game/player/movement_abilities/wall_jump_ability_data.hpp"
#include "game/player/player_state.hpp"
#include "test_helpers/mock_player.hpp"
using Catch::Approx;

TEST_CASE("WallJumpAbility performs correctly", "WallJumpAbility")
{
    PlayerState playerState;
    MockPlayer movementContext;
    WallJumpAbilityData wallJumpAbilityData;
    WallJumpAbility wallJumpAbility(wallJumpAbilityData);

    SECTION("WallJumpAbility performs a wall jump when wall sliding and airborne")
    {
        movementContext.facingLeft = true;
        playerState.onGround = false;
        playerState.touchingLeftWall = true;
        playerState.wallSliding = true;
        wallJumpAbility.tryJump(movementContext, playerState);
        wallJumpAbility.fixedUpdate(movementContext, playerState, 0.1f);
        wallJumpAbility.update(movementContext, playerState, 0.1f);
        wallJumpAbility.syncState(playerState);
        REQUIRE(movementContext.velocity.y == Approx(-280.0f));
        REQUIRE(movementContext.velocity.x == Approx(200.0f));
        REQUIRE(movementContext.facingLeft == false);
    }

    SECTION("WallJumpAbility does not activate if not wall sliding")
    {
        playerState.onGround = false;
        playerState.touchingLeftWall = true;
        playerState.wallSliding = false;
        wallJumpAbility.tryJump(movementContext, playerState);
        wallJumpAbility.fixedUpdate(movementContext, playerState, 0.1f);
        wallJumpAbility.syncState(playerState);
        REQUIRE_FALSE(playerState.wallJumping);
        REQUIRE(movementContext.velocity.y == Approx(0.0f));
        REQUIRE(movementContext.velocity.x == Approx(0.0f));
    }

    SECTION("WallJumpAbility ends early if wall side switches")
    {
        playerState.onGround = false;
        playerState.touchingLeftWall = true;
        playerState.wallSliding = true;
        wallJumpAbility.tryJump(movementContext, playerState);

        playerState.touchingLeftWall = false;
        playerState.touchingRightWall = true;
        wallJumpAbility.fixedUpdate(movementContext, playerState, 0.1f);

        REQUIRE_FALSE(playerState.wallJumping);
        REQUIRE(movementContext.velocity.y == Approx(0.0f));
        REQUIRE(movementContext.velocity.x == Approx(0.0f));
    }

    SECTION("WallJumpAbility maintains jump for duration if no side switch")
    {
        playerState.onGround = false;
        playerState.touchingLeftWall = true;
        playerState.wallSliding = true;
        wallJumpAbility.tryJump(movementContext, playerState);

        for (int i = 0; i < 5; ++i)
        {
            wallJumpAbility.fixedUpdate(movementContext, playerState, 0.02f);
            wallJumpAbility.syncState(playerState);
            REQUIRE(playerState.wallJumping);
        }

        wallJumpAbility.fixedUpdate(movementContext, playerState, 1.0f);
        wallJumpAbility.syncState(playerState);
        REQUIRE_FALSE(playerState.wallJumping);
    }
}