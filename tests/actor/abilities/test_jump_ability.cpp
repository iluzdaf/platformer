#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "actor/actor_motion_state.hpp"
#include "actor/abilities/jump_ability.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

TEST_CASE("JumpAbility basic movement behaviour", "[JumpAbility]")
{
    InputIntentions inputIntentions;
    ActorMotionState state;
    JumpAbilityData jumpAbilityData;
    JumpAbility jumpAbility(jumpAbilityData);

    SECTION("Can jump")
    {
        state.contacts.onGround = true;
        inputIntentions.jumpRequested = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, state);
        state.contacts.onGround = false;
        REQUIRE(state.jump.velocity.y == Approx(jumpAbilityData.jumpSpeed));
        REQUIRE(state.jump.active);
        inputIntentions = InputIntentions();
        inputIntentions.jumpHeld = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.jump.velocity.y == Approx(jumpAbilityData.jumpSpeed));
    }

    SECTION("Cannot jump if not on ground")
    {
        state.contacts.onGround = false;
        inputIntentions.jumpRequested = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.jump.active);
        REQUIRE(state.jump.velocity.y == Approx(0.0f));
    }

    SECTION("Can jump if jump request is buffered")
    {
        state.contacts.onGround = false;
        inputIntentions.jumpRequested = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, state);
        state.contacts.onGround = false;
        REQUIRE_FALSE(state.jump.active);
        REQUIRE(state.jump.velocity.y == Approx(0.0f));
        state.contacts.onGround = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.jump.active);
        REQUIRE(state.jump.velocity.y == Approx(jumpAbilityData.jumpSpeed));
    }

    SECTION("Can jump during coyote time")
    {
        state.contacts.onGround = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, state);
        state.contacts.onGround = false;
        inputIntentions.jumpRequested = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.jump.active);
        REQUIRE(state.jump.velocity.y == Approx(jumpAbilityData.jumpSpeed));
    }

    SECTION("Jump ends after duration")
    {
        state.contacts.onGround = true;
        inputIntentions.jumpRequested = true;
        jumpAbility.applyMovement(jumpAbilityData.jumpDuration + 0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.jump.active);
        REQUIRE(state.jump.velocity.y == Approx(0.0f));
    }

    SECTION("Requesting to jump mid-jump should not change jumpHoldTime")
    {
        state.contacts.onGround = true;
        inputIntentions.jumpRequested = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, state);
        inputIntentions = InputIntentions();
        inputIntentions.jumpHeld = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.jump.velocity.y == Approx(jumpAbilityData.jumpSpeed));
        REQUIRE(state.jump.active);
        REQUIRE(state.jump.holdTime == Approx(0.02f));
    }

    SECTION("Cannot jump if jumpHeld while landing")
    {
        state.contacts.onGround = true;
        inputIntentions.jumpRequested = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, state);
        state.contacts.onGround = false;
        inputIntentions = InputIntentions();
        inputIntentions.jumpHeld = true;
        jumpAbility.applyMovement(jumpAbilityData.jumpDuration, inputIntentions, state);
        REQUIRE_FALSE(state.jump.active);
        REQUIRE(state.jump.velocity.y == Approx(0.0f));
        state.contacts.onGround = true;
        inputIntentions = InputIntentions();
        inputIntentions.jumpHeld = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.jump.active);
    }
}