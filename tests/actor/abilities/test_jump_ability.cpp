#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "actor/abilities/jump_ability_data.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/jump_ability.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

TEST_CASE("JumpAbility basic movement behaviour", "[JumpAbility]")
{
    InputIntentions inputIntentions;
    Decided decided;
    Observed observed;
    JumpAbilityData jumpAbilityData;
    JumpAbility jumpAbility(jumpAbilityData);

    SECTION("Can jump")
    {
        observed.contacts.onGround = true;
        inputIntentions.jumpRequested = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        observed.contacts.onGround = false;
        REQUIRE(decided.jump.velocity.y == Approx(jumpAbilityData.jumpSpeed));
        REQUIRE(decided.jump.active);
        inputIntentions = InputIntentions();
        inputIntentions.jumpHeld = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.jump.velocity.y == Approx(jumpAbilityData.jumpSpeed));
    }

    SECTION("A head against a ceiling ends the jump rather than pushing on")
    {
        observed.contacts.onGround = true;
        inputIntentions.jumpRequested = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        observed.contacts.onGround = false;
        REQUIRE(decided.jump.active);

        inputIntentions = InputIntentions();
        inputIntentions.jumpHeld = true;
        observed.contacts.hitCeiling = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);

        REQUIRE_FALSE(decided.jump.active);
        REQUIRE(decided.jump.velocity.y == Approx(0.0f));

        observed.contacts.hitCeiling = false;
        jumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.jump.velocity.y == Approx(0.0f));
    }

    SECTION("Cannot jump if not on ground")
    {
        observed.contacts.onGround = false;
        inputIntentions.jumpRequested = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        REQUIRE_FALSE(decided.jump.active);
        REQUIRE(decided.jump.velocity.y == Approx(0.0f));
    }

    SECTION("Can jump if jump request is buffered")
    {
        observed.contacts.onGround = false;
        inputIntentions.jumpRequested = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        observed.contacts.onGround = false;
        REQUIRE_FALSE(decided.jump.active);
        REQUIRE(decided.jump.velocity.y == Approx(0.0f));
        observed.contacts.onGround = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.jump.active);
        REQUIRE(decided.jump.velocity.y == Approx(jumpAbilityData.jumpSpeed));
    }

    SECTION("Can jump during coyote time")
    {
        observed.contacts.onGround = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        observed.contacts.onGround = false;
        inputIntentions.jumpRequested = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.jump.active);
        REQUIRE(decided.jump.velocity.y == Approx(jumpAbilityData.jumpSpeed));
    }

    SECTION("Jump ends after duration")
    {
        observed.contacts.onGround = true;
        inputIntentions.jumpRequested = true;
        jumpAbility.applyMovement(
            jumpAbilityData.jumpDuration + 0.01f, inputIntentions, observed, decided);
        REQUIRE_FALSE(decided.jump.active);
        REQUIRE(decided.jump.velocity.y == Approx(0.0f));
    }

    SECTION("Requesting to jump mid-jump should not change jumpHoldTime")
    {
        observed.contacts.onGround = true;
        inputIntentions.jumpRequested = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        inputIntentions = InputIntentions();
        inputIntentions.jumpHeld = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.jump.velocity.y == Approx(jumpAbilityData.jumpSpeed));
        REQUIRE(decided.jump.active);
        REQUIRE(decided.jump.holdTime == Approx(0.02f));
    }

    SECTION("Cannot jump if jumpHeld while landing")
    {
        observed.contacts.onGround = true;
        inputIntentions.jumpRequested = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        observed.contacts.onGround = false;
        inputIntentions = InputIntentions();
        inputIntentions.jumpHeld = true;
        jumpAbility.applyMovement(jumpAbilityData.jumpDuration, inputIntentions, observed, decided);
        REQUIRE_FALSE(decided.jump.active);
        REQUIRE(decided.jump.velocity.y == Approx(0.0f));
        observed.contacts.onGround = true;
        inputIntentions = InputIntentions();
        inputIntentions.jumpHeld = true;
        jumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        REQUIRE_FALSE(decided.jump.active);
    }
}

TEST_CASE("A jump that does not go up is refused", "[JumpAbility]")
{
    JumpAbilityData downwards;
    downwards.jumpSpeed = 0.0f;
    REQUIRE_THROWS(JumpAbility(downwards));
}
