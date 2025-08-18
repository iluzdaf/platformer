#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "agent/agent_state.hpp"
#include "agent/movement_abilities/jump_ability.hpp"
#include "input/input_intentions.hpp"
#include "physics/fixed_time_step.hpp"

using Catch::Approx;

TEST_CASE("JumpAbility basic movement behaviour", "[JumpAbility]")
{
    InputIntentions inputIntentions;
    AgentState state;
    JumpAbilityData jumpAbilityData;
    JumpAbility jumpAbility(jumpAbilityData);

    SECTION("Can jump")
    {
        state.onGround = true;
        inputIntentions.jumpRequested = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, state);
        state.onGround = false;
        REQUIRE(state.jumpVelocity.y == Approx(jumpAbilityData.jumpSpeed));
        REQUIRE(state.jumping);
        inputIntentions = InputIntentions();
        inputIntentions.jumpHeld = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.jumpVelocity.y == Approx(jumpAbilityData.jumpSpeed));
    }

    SECTION("Cannot jump if not on ground")
    {
        state.onGround = false;
        inputIntentions.jumpRequested = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.jumping);
        REQUIRE(state.jumpVelocity.y == Approx(0.0f));
    }

    SECTION("Can jump if jump request is buffered")
    {
        state.onGround = false;
        inputIntentions.jumpRequested = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, state);
        state.onGround = false;
        REQUIRE_FALSE(state.jumping);
        REQUIRE(state.jumpVelocity.y == Approx(0.0f));
        state.onGround = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.jumping);
        REQUIRE(state.jumpVelocity.y == Approx(jumpAbilityData.jumpSpeed));
    }

    SECTION("Can jump during coyote time")
    {
        state.onGround = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, state);
        state.onGround = false;
        inputIntentions.jumpRequested = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.jumping);
        REQUIRE(state.jumpVelocity.y == Approx(jumpAbilityData.jumpSpeed));
    }

    SECTION("Jump ends after duration")
    {
        state.onGround = true;
        inputIntentions.jumpRequested = true;
        jumpAbility.applyMovement(jumpAbilityData.jumpDuration + 0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.jumping);
        REQUIRE(state.jumpVelocity.y == Approx(0.0f));
    }

    SECTION("Requesting to jump mid-jump should not change jumpHoldTime")
    {
        state.onGround = true;
        inputIntentions.jumpRequested = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, state);
        inputIntentions = InputIntentions();
        inputIntentions.jumpHeld = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.jumpVelocity.y == Approx(jumpAbilityData.jumpSpeed));
        REQUIRE(state.jumping);
        REQUIRE(state.jumpHoldTime == Approx(0.02f));
    }

    SECTION("Cannot jump if jumpHeld while landing")
    {
        state.onGround = true;
        inputIntentions.jumpRequested = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, state);
        state.onGround = false;
        inputIntentions = InputIntentions();
        inputIntentions.jumpHeld = true;
        jumpAbility.applyMovement(jumpAbilityData.jumpDuration, inputIntentions, state);
        REQUIRE_FALSE(state.jumping);
        REQUIRE(state.jumpVelocity.y == Approx(0.0f));
        state.onGround = true;
        inputIntentions = InputIntentions();
        inputIntentions.jumpHeld = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.jumping);
    }
}