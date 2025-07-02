#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/player/movement_abilities/wall_jump_ability.hpp"
#include "game/player/movement_abilities/movement_context.hpp"
#include "game/player/player_state.hpp"

using Catch::Approx;

TEST_CASE("WallJumpAbility basic movement behaviour", "WallJumpAbility")
{
    PlayerState playerState;
    MovementContext movementContext;
    WallJumpAbilityData wallJumpAbilityData;
    WallJumpAbility wallJumpAbility(wallJumpAbilityData);

    SECTION("Can wall jump")
    {
        playerState.onGround = false;
        playerState.touchingLeftWall = true;
        movementContext.inputIntentions.jumpHeld = true;
        movementContext.inputIntentions.direction.x = 1.0f;
        wallJumpAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE(movementContext.wallJumpVelocity.y == Approx(wallJumpAbilityData.wallJumpSpeed));
        REQUIRE(movementContext.wallJumpVelocity.x == Approx(wallJumpAbilityData.wallJumpHorizontalSpeed));
        REQUIRE(playerState.wallJumping);
        REQUIRE(playerState.wallJumpDirection == 1);
    }

    SECTION("Cannot wall jump if correct direction not pressed")
    {
        playerState.onGround = false;
        playerState.touchingLeftWall = true;
        movementContext.inputIntentions.jumpHeld = true;
        movementContext.inputIntentions.direction.x = 0.0f;
        wallJumpAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE_FALSE(playerState.wallJumping);
        REQUIRE(movementContext.wallJumpVelocity.y == Approx(0.0f));
        REQUIRE(movementContext.wallJumpVelocity.x == Approx(0.0f));
    }

    SECTION("Can wall jump if jump request is buffered")
    {
        playerState.onGround = false;
        playerState.touchingLeftWall = false;
        movementContext.inputIntentions.jumpHeld = true;
        movementContext.inputIntentions.direction.x = 1.0f;
        wallJumpAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE_FALSE(playerState.wallJumping);
        REQUIRE(movementContext.wallJumpVelocity.y == Approx(0.0f));
        REQUIRE(movementContext.wallJumpVelocity.x == Approx(0.0f));
        playerState.touchingLeftWall = true;
        movementContext = MovementContext();
        wallJumpAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE(playerState.wallJumping);
        REQUIRE(movementContext.wallJumpVelocity.y == Approx(wallJumpAbilityData.wallJumpSpeed));
        REQUIRE(movementContext.wallJumpVelocity.x == Approx(wallJumpAbilityData.wallJumpHorizontalSpeed));
    }

    SECTION("Can wall jump during coyote time")
    {
        playerState.onGround = false;
        playerState.touchingLeftWall = true;
        wallJumpAbility.fixedUpdate(movementContext, playerState, 0.01f);
        playerState.touchingLeftWall = false;
        playerState.wasLastWallLeft = true;
        movementContext.inputIntentions.jumpHeld = true;
        movementContext.inputIntentions.direction.x = 1.0f;
        wallJumpAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE(playerState.wallJumping);
        REQUIRE(movementContext.wallJumpVelocity.y == Approx(wallJumpAbilityData.wallJumpSpeed));
        REQUIRE(movementContext.wallJumpVelocity.x == Approx(wallJumpAbilityData.wallJumpHorizontalSpeed));
    }

    SECTION("Wall jump ends after duration")
    {
        playerState.onGround = false;
        playerState.touchingLeftWall = true;
        movementContext.inputIntentions.jumpHeld = true;
        movementContext.inputIntentions.direction.x = 1.0f;
        wallJumpAbility.fixedUpdate(movementContext, playerState, wallJumpAbilityData.wallJumpDuration + 0.01f);
        REQUIRE_FALSE(playerState.wallJumping);
        REQUIRE(movementContext.wallJumpVelocity.y == Approx(0.0f));
        REQUIRE(movementContext.wallJumpVelocity.x == Approx(0.0f));
    }

    SECTION("Wall jump ends when switching sides")
    {
        playerState.onGround = false;
        playerState.touchingLeftWall = true;
        movementContext.inputIntentions.jumpHeld = true;
        movementContext.inputIntentions.direction.x = 1.0f;
        wallJumpAbility.fixedUpdate(movementContext, playerState, 0.01f);
        playerState.touchingLeftWall = false;
        playerState.touchingRightWall = true;
        movementContext = MovementContext();
        movementContext.inputIntentions.jumpHeld = true;
        movementContext.inputIntentions.direction.x = -1.0f;
        wallJumpAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE_FALSE(playerState.wallJumping);
        REQUIRE(movementContext.wallJumpVelocity.y == Approx(0.0f));
        REQUIRE(movementContext.wallJumpVelocity.x == Approx(0.0f));
    }
}