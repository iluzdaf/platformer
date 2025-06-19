#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/player/movement_abilities/wall_jump_ability.hpp"
#include "game/player/movement_abilities/wall_jump_ability_data.hpp"
#include "game/player/player_state.hpp"
#include "test_helpers/mock_player.hpp"
using Catch::Approx;

TEST_CASE("WallJumpAbility basic movement behaviour", "WallJumpAbility")
{
    PlayerState playerState;
    MockPlayer movementContext;
    WallJumpAbilityData wallJumpAbilityData;
    WallJumpAbility wallJumpAbility(wallJumpAbilityData);

    SECTION("Can wall jump when touching wall and airborne")
    {
        movementContext.facingLeft = true;
        playerState.onGround = false;
        playerState.touchingLeftWall = true;
        wallJumpAbility.tryJump(movementContext, playerState);
        wallJumpAbility.fixedUpdate(movementContext, playerState, 0.1f);
        wallJumpAbility.syncState(playerState);
        wallJumpAbility.update(movementContext, playerState, 0.1f);
        REQUIRE(movementContext.velocity.y == Approx(-280.0f));
        REQUIRE(movementContext.velocity.x == Approx(200.0f));
        REQUIRE(movementContext.facingLeft == false);
    }

    SECTION("Cannot wall jump if not touching wall")
    {
        playerState.onGround = false;
        wallJumpAbility.tryJump(movementContext, playerState);
        wallJumpAbility.fixedUpdate(movementContext, playerState, 0.1f);
        wallJumpAbility.syncState(playerState);
        wallJumpAbility.update(movementContext, playerState, 0.1f);
        REQUIRE_FALSE(playerState.wallJumping);
        REQUIRE(movementContext.velocity.y == Approx(0.0f));
        REQUIRE(movementContext.velocity.x == Approx(0.0f));
    }

    SECTION("Wall jump cancels if wall side switches")
    {
        playerState.onGround = false;
        playerState.touchingLeftWall = true;
        wallJumpAbility.tryJump(movementContext, playerState);
        playerState.touchingLeftWall = false;
        playerState.touchingRightWall = true;
        wallJumpAbility.fixedUpdate(movementContext, playerState, 0.1f);
        wallJumpAbility.syncState(playerState);
        wallJumpAbility.update(movementContext, playerState, 0.1f);
        REQUIRE_FALSE(playerState.wallJumping);
        REQUIRE(movementContext.velocity.y == Approx(0.0f));
        REQUIRE(movementContext.velocity.x == Approx(0.0f));
    }

    SECTION("Wall jump maintains jump for duration if no side switch")
    {
        playerState.onGround = false;
        playerState.touchingLeftWall = true;
        wallJumpAbility.tryJump(movementContext, playerState);

        for (int i = 0; i < 5; ++i)
        {
            wallJumpAbility.fixedUpdate(movementContext, playerState, 0.02f);
            wallJumpAbility.syncState(playerState);
            REQUIRE(playerState.wallJumping);
        }

        wallJumpAbility.fixedUpdate(movementContext, playerState, 1.0f);
        wallJumpAbility.syncState(playerState);
        wallJumpAbility.update(movementContext, playerState, 1.1f);
        REQUIRE_FALSE(playerState.wallJumping);
    }

    SECTION("Wall jump respects maxJumpCount")
    {
        playerState.onGround = false;
        playerState.touchingLeftWall = true;

        wallJumpAbility.tryJump(movementContext, playerState);
        wallJumpAbility.fixedUpdate(movementContext, playerState, 0.1f);
        wallJumpAbility.syncState(playerState);
        wallJumpAbility.update(movementContext, playerState, 0.1f);
        REQUIRE(playerState.wallJumping);

        wallJumpAbility.fixedUpdate(movementContext, playerState, 1.0f);
        wallJumpAbility.syncState(playerState);
        wallJumpAbility.update(movementContext, playerState, 1.0f);

        playerState.touchingLeftWall = true;
        wallJumpAbility.tryJump(movementContext, playerState);
        wallJumpAbility.fixedUpdate(movementContext, playerState, 0.1f);
        wallJumpAbility.syncState(playerState);
        wallJumpAbility.update(movementContext, playerState, 0.1f);
        REQUIRE(playerState.wallJumping);

        wallJumpAbility.fixedUpdate(movementContext, playerState, 1.0f);
        wallJumpAbility.syncState(playerState);
        wallJumpAbility.update(movementContext, playerState, 1.0f);

        playerState.touchingLeftWall = true;
        wallJumpAbility.tryJump(movementContext, playerState);
        wallJumpAbility.fixedUpdate(movementContext, playerState, 0.1f);
        wallJumpAbility.syncState(playerState);
        wallJumpAbility.update(movementContext, playerState, 0.1f);
        REQUIRE_FALSE(playerState.wallJumping);

        playerState.onGround = true;
        wallJumpAbility.fixedUpdate(movementContext, playerState, 0.01f);
        wallJumpAbility.update(movementContext, playerState, 0.01f);

        playerState.onGround = false;
        playerState.touchingLeftWall = true;
        wallJumpAbility.tryJump(movementContext, playerState);
        wallJumpAbility.fixedUpdate(movementContext, playerState, 0.1f);
        wallJumpAbility.syncState(playerState);
        wallJumpAbility.update(movementContext, playerState, 0.1f);
        REQUIRE(playerState.wallJumping);
    }

    SECTION("Wall jump is buffered and consumed on touching wall")
    {
        playerState.onGround = false;
        playerState.touchingLeftWall = false;
        wallJumpAbility.tryJump(movementContext, playerState);
        playerState.touchingLeftWall = true;
        wallJumpAbility.fixedUpdate(movementContext, playerState, 0.09f);
        wallJumpAbility.syncState(playerState);
        wallJumpAbility.update(movementContext, playerState, 0.09f);
        REQUIRE(playerState.wallJumping);
        REQUIRE(movementContext.velocity.y < 0.0f);
        REQUIRE(movementContext.velocity.x != 0.0f);
    }

    SECTION("Can wall jump during coyote time")
    {
        playerState.touchingLeftWall = true;
        wallJumpAbility.fixedUpdate(movementContext, playerState, 0.01f);
        playerState.touchingLeftWall = false;
        wallJumpAbility.fixedUpdate(movementContext, playerState, 0.08f);
        wallJumpAbility.update(movementContext, playerState, 0.09f);
        wallJumpAbility.tryJump(movementContext, playerState);
        wallJumpAbility.fixedUpdate(movementContext, playerState, 0.1f);
        wallJumpAbility.syncState(playerState);
        wallJumpAbility.update(movementContext, playerState, 0.09f);
        REQUIRE(playerState.wallJumping);
        REQUIRE(movementContext.velocity.y < 0.0f);
        REQUIRE(movementContext.velocity.x != 0.0f);
    }
}