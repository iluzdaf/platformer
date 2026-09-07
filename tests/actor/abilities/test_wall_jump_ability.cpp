#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "actor/abilities/wall_jump_ability_data.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/wall_jump_ability.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

TEST_CASE("WallJumpAbility basic movement behaviour", "[WallJumpAbility]")
{
    Decided decided;
    Observed observed;
    InputIntentions inputIntentions;
    WallJumpAbilityData wallJumpAbilityData;
    WallJumpAbility wallJumpAbility(wallJumpAbilityData);

    SECTION("Can wall jump")
    {
        observed.contacts.onGround = false;
        observed.contacts.touchingLeftWall = true;
        observed.contacts.grippableLeftWall = true;
        inputIntentions.jumpHeld = true;
        inputIntentions.direction.x = 1.0f;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.wallJump.velocity.y == Approx(wallJumpAbilityData.wallJumpSpeed));
        REQUIRE(decided.wallJump.velocity.x == Approx(wallJumpAbilityData.wallJumpHorizontalSpeed));
        REQUIRE(decided.wallJump.active);
        REQUIRE(decided.wallJump.direction == 1);
    }

    SECTION("Cannot wall jump if correct direction not pressed")
    {
        observed.contacts.onGround = false;
        observed.contacts.touchingLeftWall = true;
        observed.contacts.grippableLeftWall = true;
        inputIntentions.jumpHeld = true;
        inputIntentions.direction.x = 0.0f;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        REQUIRE_FALSE(decided.wallJump.active);
        REQUIRE(decided.wallJump.velocity.y == Approx(0.0f));
        REQUIRE(decided.wallJump.velocity.x == Approx(0.0f));
    }

    SECTION("Can wall jump if jump request is buffered")
    {
        observed.contacts.onGround = false;
        observed.contacts.touchingLeftWall = false;
        inputIntentions.jumpHeld = true;
        inputIntentions.direction.x = 1.0f;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        REQUIRE_FALSE(decided.wallJump.active);
        REQUIRE(decided.wallJump.velocity.y == Approx(0.0f));
        REQUIRE(decided.wallJump.velocity.x == Approx(0.0f));
        observed.contacts.touchingLeftWall = true;
        observed.contacts.grippableLeftWall = true;
        inputIntentions = InputIntentions();
        wallJumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.wallJump.active);
        REQUIRE(decided.wallJump.velocity.y == Approx(wallJumpAbilityData.wallJumpSpeed));
        REQUIRE(decided.wallJump.velocity.x == Approx(wallJumpAbilityData.wallJumpHorizontalSpeed));
    }

    SECTION("Can wall jump during coyote time")
    {
        observed.contacts.onGround = false;
        observed.contacts.touchingLeftWall = true;
        observed.contacts.grippableLeftWall = true;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        observed.contacts.touchingLeftWall = false;
        observed.contacts.grippableLeftWall = false;
        observed.contacts.wasLastWallLeft = true;
        inputIntentions.jumpHeld = true;
        inputIntentions.direction.x = 1.0f;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.wallJump.active);
        REQUIRE(decided.wallJump.velocity.y == Approx(wallJumpAbilityData.wallJumpSpeed));
        REQUIRE(decided.wallJump.velocity.x == Approx(wallJumpAbilityData.wallJumpHorizontalSpeed));
    }

    SECTION("Wall jump ends after duration")
    {
        observed.contacts.onGround = false;
        observed.contacts.touchingLeftWall = true;
        observed.contacts.grippableLeftWall = true;
        inputIntentions.jumpHeld = true;
        inputIntentions.direction.x = 1.0f;
        wallJumpAbility.applyMovement(
            wallJumpAbilityData.wallJumpDuration + 0.01f, inputIntentions, observed, decided);
        REQUIRE_FALSE(decided.wallJump.active);
        REQUIRE(decided.wallJump.velocity.y == Approx(0.0f));
        REQUIRE(decided.wallJump.velocity.x == Approx(0.0f));
    }

    SECTION("Wall jump ends when switching sides")
    {
        observed.contacts.onGround = false;
        observed.contacts.touchingLeftWall = true;
        observed.contacts.grippableLeftWall = true;
        inputIntentions.jumpHeld = true;
        inputIntentions.direction.x = 1.0f;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        observed.contacts.touchingLeftWall = false;
        observed.contacts.grippableLeftWall = false;
        observed.contacts.touchingRightWall = true;
        inputIntentions = InputIntentions();
        inputIntentions.jumpHeld = true;
        inputIntentions.direction.x = -1.0f;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        REQUIRE_FALSE(decided.wallJump.active);
        REQUIRE(decided.wallJump.velocity.y == Approx(0.0f));
        REQUIRE(decided.wallJump.velocity.x == Approx(0.0f));
    }

    SECTION("Cannot wall jump from a wall it cannot grip")
    {
        observed.contacts.onGround = false;
        observed.contacts.touchingLeftWall = true;
        observed.contacts.grippableLeftWall = false;
        inputIntentions.jumpHeld = true;
        inputIntentions.direction.x = 1.0f;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        REQUIRE_FALSE(decided.wallJump.active);
        REQUIRE(decided.wallJump.velocity.y == Approx(0.0f));
        REQUIRE(decided.wallJump.velocity.x == Approx(0.0f));
    }

    SECTION("Coyote time does not arm on a wall it cannot grip")
    {
        observed.contacts.onGround = false;
        observed.contacts.touchingLeftWall = true;
        observed.contacts.grippableLeftWall = false;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        observed.contacts.touchingLeftWall = false;
        observed.contacts.wasLastWallLeft = true;
        inputIntentions.jumpHeld = true;
        inputIntentions.direction.x = 1.0f;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        REQUIRE_FALSE(decided.wallJump.active);
    }

    SECTION("Cannot wall jump from a wall it cannot grip even facing away from it")
    {
        observed.contacts.onGround = false;
        observed.contacts.touchingLeftWall = true;
        observed.contacts.grippableLeftWall = false;
        observed.contacts.wasLastWallLeft = true;
        inputIntentions.jumpHeld = true;
        inputIntentions.direction.x = 1.0f;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        REQUIRE_FALSE(decided.wallJump.active);
    }

    SECTION("A coyote jump pushes away from the wall it remembers, not one it cannot grip")
    {
        observed.contacts.onGround = false;
        observed.contacts.touchingLeftWall = true;
        observed.contacts.grippableLeftWall = true;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);

        observed.contacts.touchingLeftWall = false;
        observed.contacts.grippableLeftWall = false;
        observed.contacts.touchingRightWall = true;
        observed.contacts.wasLastWallLeft = true;
        inputIntentions.jumpHeld = true;
        inputIntentions.direction.x = 1.0f;
        wallJumpAbility.applyMovement(0.01f, inputIntentions, observed, decided);

        REQUIRE(decided.wallJump.emit);
        REQUIRE(decided.wallJump.direction == 1);
    }
}

TEST_CASE("A wall jump that does not go up and away is refused", "[WallJumpAbility]")
{
    WallJumpAbilityData downwards;
    downwards.wallJumpSpeed = 0.0f;
    REQUIRE_THROWS(WallJumpAbility(downwards));

    WallJumpAbilityData intoTheWall;
    intoTheWall.wallJumpHorizontalSpeed = 0.0f;
    REQUIRE_THROWS(WallJumpAbility(intoTheWall));
}
