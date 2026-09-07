#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "actor/actor_motion_data.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "actor/hit.hpp"
#include "actor/abilities/ability_system.hpp"
#include "actor/abilities/move_ability.hpp"
#include "actor/abilities/jump_ability.hpp"
#include "actor/abilities/dash_ability.hpp"
#include "actor/abilities/wall_slide_ability.hpp"
#include "actor/abilities/wall_jump_ability.hpp"
#include "actor/abilities/wall_hang_ability.hpp"
#include "actor/abilities/wall_climb_ability.hpp"
#include "actor/abilities/gravity_ability.hpp"
#include "actor/abilities/knockback_ability_data.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

namespace
{
    void simulateMovement(
        AbilitySystem &abilitySystem,
        float deltaTime,
        const InputIntentions &inputIntentions,
        Observed &observed,
        Decided &decided)
    {
        abilitySystem.applyMovement(deltaTime, inputIntentions, observed, decided);

        observed.velocity = decided.targetVelocity;
    }
}

TEST_CASE("AbilitySystem basic functionality", "[AbilitySystem]")
{
    Decided decided;
    Observed observed;
    ActorMotionData motionData;
    motionData.moveAbilityData = MoveAbilityData();
    motionData.jumpAbilityData = JumpAbilityData();
    motionData.knockbackAbilityData = KnockbackAbilityData();
    motionData.dashAbilityData = DashAbilityData();
    motionData.wallSlideAbilityData = WallSlideAbilityData();
    motionData.wallJumpAbilityData = WallJumpAbilityData();
    motionData.wallHangAbilityData = WallHangAbilityData();
    motionData.wallClimbAbilityData = WallClimbAbilityData();
    motionData.gravityAbilityData = GravityAbilityData();
    AbilitySystem abilitySystem(motionData);
    InputIntentions inputIntentions;

    SECTION("Can move and jump")
    {
        observed.contacts.onGround = true;
        inputIntentions.direction = glm::vec2(1.0f, 0.0f);
        inputIntentions.jumpRequested = true;
        simulateMovement(abilitySystem, 0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.targetVelocity.x == Approx(motionData.moveAbilityData->moveSpeed));
        REQUIRE(decided.targetVelocity.y == Approx(motionData.jumpAbilityData->jumpSpeed));
        REQUIRE(decided.jump.active == true);
    }

    SECTION("Can jump, wall slide then wall jump")
    {
        observed.contacts.onGround = true;
        observed.contacts.touchingLeftWall = observed.contacts.grippableLeftWall = true;
        inputIntentions.jumpRequested = true;
        simulateMovement(abilitySystem, 0.01f, inputIntentions, observed, decided);
        observed.contacts.onGround = false;
        REQUIRE(decided.jump.active);
        REQUIRE_FALSE(decided.wallJump.active);
        REQUIRE(decided.targetVelocity.y == Approx(motionData.jumpAbilityData->jumpSpeed));
        REQUIRE(decided.targetVelocity.x == Approx(0.0f));
        inputIntentions = InputIntentions();
        simulateMovement(
            abilitySystem,
            motionData.jumpAbilityData->jumpDuration,
            inputIntentions,
            observed,
            decided);
        REQUIRE_FALSE(decided.jump.active);
        simulateMovement(
            abilitySystem,
            motionData.jumpAbilityData->jumpDuration,
            inputIntentions,
            observed,
            decided);
        REQUIRE(decided.wallSlide.active);
        REQUIRE(decided.targetVelocity.y == Approx(motionData.wallSlideAbilityData->slideSpeed));
        REQUIRE(decided.targetVelocity.x == Approx(0.0f));
        inputIntentions.jumpHeld = true;
        inputIntentions.direction = glm::vec2(1.0f, 0.0f);
        simulateMovement(abilitySystem, 0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.wallJump.active);
        REQUIRE(decided.targetVelocity.y == Approx(motionData.wallJumpAbilityData->wallJumpSpeed));
        REQUIRE(
            decided.targetVelocity.x ==
            Approx(
                decided.wallJump.direction *
                motionData.wallJumpAbilityData->wallJumpHorizontalSpeed));
    }

    SECTION("Can dash into wall then wall jump")
    {
        observed.contacts.onGround = false;
        observed.contacts.touchingLeftWall = observed.contacts.grippableLeftWall = false;
        inputIntentions.dashRequested = true;
        inputIntentions.direction.x = 1.0f;
        simulateMovement(abilitySystem, 0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.dash.active);
        REQUIRE(decided.targetVelocity.x == Approx(motionData.dashAbilityData->dashSpeed));
        observed.contacts.touchingLeftWall = observed.contacts.grippableLeftWall = true;
        inputIntentions = InputIntentions();
        simulateMovement(abilitySystem, 0.01f, inputIntentions, observed, decided);
        REQUIRE_FALSE(decided.dash.active);
        REQUIRE(decided.targetVelocity.x == Approx(0.0f));
        simulateMovement(abilitySystem, 0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.wallSlide.active);
        REQUIRE(decided.targetVelocity.y == Approx(motionData.wallSlideAbilityData->slideSpeed));
        inputIntentions.jumpHeld = true;
        inputIntentions.direction = glm::vec2(1.0f, 0.0f);
        simulateMovement(abilitySystem, 0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.wallJump.active);
        REQUIRE(decided.targetVelocity.y == Approx(motionData.wallJumpAbilityData->wallJumpSpeed));
        REQUIRE(
            decided.targetVelocity.x ==
            Approx(
                decided.wallJump.direction *
                motionData.wallJumpAbilityData->wallJumpHorizontalSpeed));
    }

    SECTION("Cannot jump while dashing")
    {
        inputIntentions.dashRequested = true;
        inputIntentions.direction.x = 1.0f;
        simulateMovement(abilitySystem, 0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.dash.active);
        REQUIRE(decided.targetVelocity.x == Approx(motionData.dashAbilityData->dashSpeed));
        inputIntentions.jumpRequested = true;
        simulateMovement(abilitySystem, 0.01f, inputIntentions, observed, decided);
        REQUIRE_FALSE(decided.jump.active);
        REQUIRE(decided.dash.active);
    }

    SECTION("Cannot move while dashing")
    {
        inputIntentions.dashRequested = true;
        inputIntentions.direction.x = 1.0f;
        simulateMovement(abilitySystem, 0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.dash.active);
        REQUIRE(decided.targetVelocity.x == Approx(motionData.dashAbilityData->dashSpeed));
        inputIntentions.direction = glm::vec2(1.0f, 0.0f);
        simulateMovement(abilitySystem, 0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.targetVelocity.x == Approx(motionData.dashAbilityData->dashSpeed));
    }

    SECTION("Can jump and dash")
    {
        observed.contacts.onGround = true;
        inputIntentions.jumpRequested = true;
        simulateMovement(abilitySystem, 0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.jump.active);
        REQUIRE(decided.targetVelocity.y == Approx(motionData.jumpAbilityData->jumpSpeed));
        inputIntentions = InputIntentions();
        inputIntentions.dashRequested = true;
        inputIntentions.direction.x = 1.0f;
        simulateMovement(abilitySystem, 0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.dash.active);
        REQUIRE(decided.targetVelocity.x == Approx(motionData.dashAbilityData->dashSpeed));
        REQUIRE(decided.targetVelocity.y == Approx(0.0f));
    }

    SECTION("Can move right and dash")
    {
        inputIntentions.direction = glm::vec2(1.0f, 0.0f);
        simulateMovement(abilitySystem, 0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.targetVelocity.x == Approx(motionData.moveAbilityData->moveSpeed));
        inputIntentions.dashRequested = true;
        simulateMovement(abilitySystem, 0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.dash.active);
        REQUIRE(decided.targetVelocity.x == Approx(motionData.dashAbilityData->dashSpeed));
    }

    SECTION("Can move left and dash")
    {
        inputIntentions.direction = glm::vec2(-1.0f, 0.0f);
        simulateMovement(abilitySystem, 0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.targetVelocity.x == Approx(-motionData.moveAbilityData->moveSpeed));
        inputIntentions.dashRequested = true;
        simulateMovement(abilitySystem, 0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.dash.active);
        REQUIRE(decided.targetVelocity.x == Approx(-motionData.dashAbilityData->dashSpeed));
    }

    SECTION("A knockback owns the velocity over everything else")
    {
        InputIntentions dashing;
        dashing.direction.x = 1;
        dashing.dashRequested = true;
        observed.contacts.onGround = true;
        simulateMovement(abilitySystem, 0.01f, dashing, observed, decided);
        REQUIRE(decided.dash.active);

        observed.hits.push_back(Hit{1, glm::vec2(-1.0f, 0.0f), false});
        simulateMovement(abilitySystem, 0.01f, dashing, observed, decided);

        REQUIRE(decided.knockback.active);
        REQUIRE(observed.velocity == decided.knockback.velocity);
        REQUIRE(observed.velocity.x < 0.0f);
    }

    SECTION("Gravity is not applied when dashing")
    {
        inputIntentions.dashRequested = true;
        inputIntentions.direction.x = 1.0f;
        simulateMovement(abilitySystem, 0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.targetVelocity.y == Approx(0.0f));
        simulateMovement(abilitySystem, 0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.targetVelocity.y == Approx(0.0f));
    }

    SECTION("Gravity is applied when not dashing")
    {
        inputIntentions.dashRequested = true;
        inputIntentions.direction.x = 1.0f;
        simulateMovement(
            abilitySystem,
            motionData.dashAbilityData->dashDuration + 0.01f,
            inputIntentions,
            observed,
            decided);
        REQUIRE(
            decided.targetVelocity.y == Approx(
                                            motionData.gravityAbilityData->gravity *
                                            (motionData.dashAbilityData->dashDuration + 0.01f)));
        simulateMovement(abilitySystem, 0.01f, inputIntentions, observed, decided);
        REQUIRE(
            decided.targetVelocity.y == Approx(
                                            motionData.gravityAbilityData->gravity *
                                            (motionData.dashAbilityData->dashDuration + 0.02f)));
    }
}