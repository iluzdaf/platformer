#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/player/movement_abilities/jump_ability.hpp"
#include "game/player/movement_abilities/jump_ability_data.hpp"
#include "game/player/player_state.hpp"
#include "test_helpers/mock_player.hpp"
using Catch::Approx;

TEST_CASE("JumpAbility basic movement behaviour", "[JumpAbility]")
{
    MockPlayer movementContext;
    PlayerState playerState;
    JumpAbilityData jumpAbilityData;
    JumpAbility jumpAbility(jumpAbilityData);

    SECTION("Player can jump twice but not more")
    {
        playerState.onGround = true;
        jumpAbility.tryJump(movementContext, playerState);
        REQUIRE(movementContext.getVelocity().y == Approx(jumpAbility.getJumpSpeed()));
        playerState.onGround = false;
        movementContext.setVelocity({0, 0});
        jumpAbility.tryJump(movementContext, playerState);
        REQUIRE(movementContext.getVelocity().y == Approx(jumpAbility.getJumpSpeed()));
        movementContext.setVelocity({0, 0});
        jumpAbility.tryJump(movementContext, playerState);
        REQUIRE(movementContext.getVelocity().y == Approx(0.0f));
    }

    SECTION("Jump count resets when on ground")
    {
        playerState.onGround = true;
        jumpAbility.tryJump(movementContext, playerState);
        jumpAbility.tryJump(movementContext, playerState);
        playerState.onGround = true;
        jumpAbility.update(movementContext, playerState, 0.01f);
        movementContext.setVelocity({0, 0});
        jumpAbility.tryJump(movementContext, playerState);
        REQUIRE(movementContext.getVelocity().y == Approx(jumpAbility.getJumpSpeed()));
    }

    SECTION("Jump is buffered and consumed on landing")
    {
        playerState.onGround = false;
        jumpAbility.tryJump(movementContext, playerState);
        playerState.onGround = true;
        jumpAbility.fixedUpdate(movementContext, playerState, 0.09f);
        REQUIRE(movementContext.getVelocity().y == Approx(jumpAbility.getJumpSpeed()));
    }

    SECTION("Cannot perform first jump in the air")
    {
        playerState.onGround = false;
        jumpAbility.tryJump(movementContext, playerState);
        REQUIRE(movementContext.getVelocity().y == Approx(0.0f));
        REQUIRE(jumpAbility.getJumpCount() == 0);
    }

    SECTION("Can jump during coyote time")
    {
        playerState.onGround = true;
        jumpAbility.fixedUpdate(movementContext, playerState, 0.1f);
        jumpAbility.update(movementContext, playerState, 0.1f);
        playerState.onGround = false;
        jumpAbility.fixedUpdate(movementContext, playerState, 0.09f);
        jumpAbility.update(movementContext, playerState, 0.09f);
        jumpAbility.tryJump(movementContext, playerState);
        REQUIRE(movementContext.getVelocity().y == Approx(jumpAbility.getJumpSpeed()));
    }

    SECTION("Cannot jump when dashing")
    {
        playerState.onGround = true;
        playerState.dashing = true;
        jumpAbility.tryJump(movementContext, playerState);
        REQUIRE(movementContext.getVelocity().y == Approx(0.0f));
        REQUIRE(jumpAbility.getJumpCount() == 0);
    }

    SECTION("Cannot jump when wallSliding")
    {
        playerState.onGround = true;
        playerState.wallSliding = true;
        jumpAbility.tryJump(movementContext, playerState);
        REQUIRE(movementContext.getVelocity().y == Approx(0.0f));
        REQUIRE(jumpAbility.getJumpCount() == 0);
    }

    SECTION("Cannot jump when wallJumping")
    {
        playerState.onGround = true;
        playerState.wallJumping = true;
        jumpAbility.tryJump(movementContext, playerState);
        REQUIRE(movementContext.getVelocity().y == Approx(0.0f));
        REQUIRE(jumpAbility.getJumpCount() == 0);
    }

    SECTION("Cannot jump when in the air and touching wall")
    {
        playerState.onGround = false;
        playerState.touchingLeftWall = true;
        jumpAbility.tryJump(movementContext, playerState);
        REQUIRE(movementContext.getVelocity().y == Approx(0.0f));
        REQUIRE(jumpAbility.getJumpCount() == 0);
    }
}