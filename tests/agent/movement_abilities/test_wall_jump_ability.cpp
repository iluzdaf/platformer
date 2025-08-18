#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "agent/agent_state.hpp"
#include "agent/movement_abilities/wall_jump_ability.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

TEST_CASE("WallJumpAbility basic movement behaviour", "[WallJumpAbility]")
{
    AgentState state;
    InputIntentions inputIntentions;
    WallJumpAbilityData wallJumpAbilityData;
    WallJumpAbility wallJumpAbility(wallJumpAbilityData);

    SECTION("Can wall jump")
    {
        state.onGround = false;
        state.touchingLeftWall = true;
        inputIntentions.jumpHeld = true;
        inputIntentions.direction.x = 1.0f;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.wallJumpVelocity.y == Approx(wallJumpAbilityData.wallJumpSpeed));
        REQUIRE(state.wallJumpVelocity.x == Approx(wallJumpAbilityData.wallJumpHorizontalSpeed));
        REQUIRE(state.wallJumping);
        REQUIRE(state.wallJumpDirection == 1);
    }

    SECTION("Cannot wall jump if correct direction not pressed")
    {
        state.onGround = false;
        state.touchingLeftWall = true;
        inputIntentions.jumpHeld = true;
        inputIntentions.direction.x = 0.0f;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.wallJumping);
        REQUIRE(state.wallJumpVelocity.y == Approx(0.0f));
        REQUIRE(state.wallJumpVelocity.x == Approx(0.0f));
    }

    SECTION("Can wall jump if jump request is buffered")
    {
        state.onGround = false;
        state.touchingLeftWall = false;
        inputIntentions.jumpHeld = true;
        inputIntentions.direction.x = 1.0f;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.wallJumping);
        REQUIRE(state.wallJumpVelocity.y == Approx(0.0f));
        REQUIRE(state.wallJumpVelocity.x == Approx(0.0f));
        state.touchingLeftWall = true;
        inputIntentions = InputIntentions();
        wallJumpAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.wallJumping);
        REQUIRE(state.wallJumpVelocity.y == Approx(wallJumpAbilityData.wallJumpSpeed));
        REQUIRE(state.wallJumpVelocity.x == Approx(wallJumpAbilityData.wallJumpHorizontalSpeed));
    }

    SECTION("Can wall jump during coyote time")
    {
        state.onGround = false;
        state.touchingLeftWall = true;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, state);
        state.touchingLeftWall = false;
        state.wasLastWallLeft = true;
        inputIntentions.jumpHeld = true;
        inputIntentions.direction.x = 1.0f;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.wallJumping);
        REQUIRE(state.wallJumpVelocity.y == Approx(wallJumpAbilityData.wallJumpSpeed));
        REQUIRE(state.wallJumpVelocity.x == Approx(wallJumpAbilityData.wallJumpHorizontalSpeed));
    }

    SECTION("Wall jump ends after duration")
    {
        state.onGround = false;
        state.touchingLeftWall = true;
        inputIntentions.jumpHeld = true;
        inputIntentions.direction.x = 1.0f;
        wallJumpAbility.applyMovement(wallJumpAbilityData.wallJumpDuration + 0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.wallJumping);
        REQUIRE(state.wallJumpVelocity.y == Approx(0.0f));
        REQUIRE(state.wallJumpVelocity.x == Approx(0.0f));
    }

    SECTION("Wall jump ends when switching sides")
    {
        state.onGround = false;
        state.touchingLeftWall = true;
        inputIntentions.jumpHeld = true;
        inputIntentions.direction.x = 1.0f;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, state);
        state.touchingLeftWall = false;
        state.touchingRightWall = true;
        inputIntentions = InputIntentions();
        inputIntentions.jumpHeld = true;
        inputIntentions.direction.x = -1.0f;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.wallJumping);
        REQUIRE(state.wallJumpVelocity.y == Approx(0.0f));
        REQUIRE(state.wallJumpVelocity.x == Approx(0.0f));
    }
}