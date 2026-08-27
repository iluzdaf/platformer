#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "actor/actor_motion_state.hpp"
#include "actor/abilities/wall_jump_ability.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

TEST_CASE("WallJumpAbility basic movement behaviour", "[WallJumpAbility]")
{
    ActorMotionState state;
    InputIntentions inputIntentions;
    WallJumpAbilityData wallJumpAbilityData;
    WallJumpAbility wallJumpAbility(wallJumpAbilityData);

    SECTION("Can wall jump")
    {
        state.contacts.onGround = false;
        state.contacts.touchingLeftWall = true;
        inputIntentions.jumpHeld = true;
        inputIntentions.direction.x = 1.0f;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.wallJump.velocity.y == Approx(wallJumpAbilityData.wallJumpSpeed));
        REQUIRE(state.wallJump.velocity.x == Approx(wallJumpAbilityData.wallJumpHorizontalSpeed));
        REQUIRE(state.wallJump.active);
        REQUIRE(state.wallJump.direction == 1);
    }

    SECTION("Cannot wall jump if correct direction not pressed")
    {
        state.contacts.onGround = false;
        state.contacts.touchingLeftWall = true;
        inputIntentions.jumpHeld = true;
        inputIntentions.direction.x = 0.0f;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.wallJump.active);
        REQUIRE(state.wallJump.velocity.y == Approx(0.0f));
        REQUIRE(state.wallJump.velocity.x == Approx(0.0f));
    }

    SECTION("Can wall jump if jump request is buffered")
    {
        state.contacts.onGround = false;
        state.contacts.touchingLeftWall = false;
        inputIntentions.jumpHeld = true;
        inputIntentions.direction.x = 1.0f;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.wallJump.active);
        REQUIRE(state.wallJump.velocity.y == Approx(0.0f));
        REQUIRE(state.wallJump.velocity.x == Approx(0.0f));
        state.contacts.touchingLeftWall = true;
        inputIntentions = InputIntentions();
        wallJumpAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.wallJump.active);
        REQUIRE(state.wallJump.velocity.y == Approx(wallJumpAbilityData.wallJumpSpeed));
        REQUIRE(state.wallJump.velocity.x == Approx(wallJumpAbilityData.wallJumpHorizontalSpeed));
    }

    SECTION("Can wall jump during coyote time")
    {
        state.contacts.onGround = false;
        state.contacts.touchingLeftWall = true;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, state);
        state.contacts.touchingLeftWall = false;
        state.contacts.wasLastWallLeft = true;
        inputIntentions.jumpHeld = true;
        inputIntentions.direction.x = 1.0f;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.wallJump.active);
        REQUIRE(state.wallJump.velocity.y == Approx(wallJumpAbilityData.wallJumpSpeed));
        REQUIRE(state.wallJump.velocity.x == Approx(wallJumpAbilityData.wallJumpHorizontalSpeed));
    }

    SECTION("Wall jump ends after duration")
    {
        state.contacts.onGround = false;
        state.contacts.touchingLeftWall = true;
        inputIntentions.jumpHeld = true;
        inputIntentions.direction.x = 1.0f;
        wallJumpAbility.applyMovement(wallJumpAbilityData.wallJumpDuration + 0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.wallJump.active);
        REQUIRE(state.wallJump.velocity.y == Approx(0.0f));
        REQUIRE(state.wallJump.velocity.x == Approx(0.0f));
    }

    SECTION("Wall jump ends when switching sides")
    {
        state.contacts.onGround = false;
        state.contacts.touchingLeftWall = true;
        inputIntentions.jumpHeld = true;
        inputIntentions.direction.x = 1.0f;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, state);
        state.contacts.touchingLeftWall = false;
        state.contacts.touchingRightWall = true;
        inputIntentions = InputIntentions();
        inputIntentions.jumpHeld = true;
        inputIntentions.direction.x = -1.0f;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.wallJump.active);
        REQUIRE(state.wallJump.velocity.y == Approx(0.0f));
        REQUIRE(state.wallJump.velocity.x == Approx(0.0f));
    }
}