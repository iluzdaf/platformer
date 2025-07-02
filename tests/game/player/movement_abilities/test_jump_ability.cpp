#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/player/movement_abilities/jump_ability.hpp"
#include "game/player/movement_abilities/movement_context.hpp"
#include "game/player/player_state.hpp"
#include "physics/fixed_time_step.hpp"

using Catch::Approx;

TEST_CASE("JumpAbility basic movement behaviour", "[JumpAbility]")
{
    MovementContext movementContext;
    PlayerState playerState;
    JumpAbilityData jumpAbilityData;
    JumpAbility jumpAbility(jumpAbilityData);

    SECTION("Can jump")
    {
        playerState.onGround = true;
        movementContext.inputIntentions.jumpRequested = true;
        jumpAbility.fixedUpdate(movementContext, playerState, 0.01f);
        playerState.onGround = false;
        REQUIRE(movementContext.jumpVelocity.y == Approx(jumpAbilityData.jumpSpeed));
        REQUIRE(playerState.jumping);
        movementContext = MovementContext();
        movementContext.inputIntentions.jumpHeld = true;
        jumpAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE(movementContext.jumpVelocity.y == Approx(jumpAbilityData.jumpSpeed));
    }

    SECTION("Cannot jump if not on ground")
    {
        playerState.onGround = false;
        movementContext.inputIntentions.jumpRequested = true;
        jumpAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE_FALSE(playerState.jumping);
        REQUIRE(movementContext.jumpVelocity.y == Approx(0.0f));
    }

    SECTION("Can jump if jump request is buffered")
    {
        playerState.onGround = false;
        movementContext.inputIntentions.jumpRequested = true;
        jumpAbility.fixedUpdate(movementContext, playerState, 0.01f);
        playerState.onGround = false;
        REQUIRE_FALSE(playerState.jumping);
        REQUIRE(movementContext.jumpVelocity.y == Approx(0.0f));
        playerState.onGround = true;
        jumpAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE(playerState.jumping);
        REQUIRE(movementContext.jumpVelocity.y == Approx(jumpAbilityData.jumpSpeed));
    }

    SECTION("Can jump during coyote time")
    {
        playerState.onGround = true;
        jumpAbility.fixedUpdate(movementContext, playerState, 0.01f);
        playerState.onGround = false;
        movementContext.inputIntentions.jumpRequested = true;
        jumpAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE(playerState.jumping);
        REQUIRE(movementContext.jumpVelocity.y == Approx(jumpAbilityData.jumpSpeed));
    }

    SECTION("Jump ends after duration")
    {
        playerState.onGround = true;
        movementContext.inputIntentions.jumpRequested = true;
        jumpAbility.fixedUpdate(movementContext, playerState, jumpAbilityData.jumpDuration + 0.01f);
        REQUIRE_FALSE(playerState.jumping);
        REQUIRE(movementContext.jumpVelocity.y == Approx(0.0f));
    }

    SECTION("Requesting to jump mid-jump should not change jumpHoldTime")
    {
        playerState.onGround = true;
        movementContext.inputIntentions.jumpRequested = true;
        jumpAbility.fixedUpdate(movementContext, playerState, 0.01f);
        movementContext = MovementContext();
        movementContext.inputIntentions.jumpHeld = true;
        jumpAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE(movementContext.jumpVelocity.y == Approx(jumpAbilityData.jumpSpeed));
        REQUIRE(playerState.jumping);
        REQUIRE(playerState.jumpHoldTime == Approx(0.02f));
    }

    SECTION("Cannot jump if jumpHeld while landing")
    {
        playerState.onGround = true;
        movementContext.inputIntentions.jumpRequested = true;
        jumpAbility.fixedUpdate(movementContext, playerState, 0.01f);
        playerState.onGround = false;
        movementContext = MovementContext();
        movementContext.inputIntentions.jumpHeld = true;
        jumpAbility.fixedUpdate(movementContext, playerState, jumpAbilityData.jumpDuration);
        REQUIRE_FALSE(playerState.jumping);
        REQUIRE(movementContext.jumpVelocity.y == Approx(0.0f));
        playerState.onGround = true;
        movementContext = MovementContext();
        movementContext.inputIntentions.jumpHeld = true;
        jumpAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE_FALSE(playerState.jumping);
    }
}