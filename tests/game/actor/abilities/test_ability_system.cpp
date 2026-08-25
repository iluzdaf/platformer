#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/actor/actor_motion_data.hpp"
#include "game/actor/actor_motion_state.hpp"
#include "game/actor/abilities/ability_system.hpp"
#include "game/actor/abilities/move_ability.hpp"
#include "game/actor/abilities/jump_ability.hpp"
#include "game/actor/abilities/dash_ability.hpp"
#include "game/actor/abilities/wall_slide_ability.hpp"
#include "game/actor/abilities/wall_jump_ability.hpp"
#include "game/actor/abilities/climb_ability.hpp"
#include "game/actor/abilities/climb_move_ability.hpp"
#include "game/actor/abilities/gravity_ability.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

namespace
{
    void simulateMovement(
        AbilitySystem &abilitySystem,
        float deltaTime,
        const InputIntentions &inputIntentions,
        ActorMotionState &state)
    {
        abilitySystem.applyMovement(deltaTime, inputIntentions, state);

        state.velocity = state.targetVelocity;
    }
}

TEST_CASE("AbilitySystem basic functionality", "[AbilitySystem]")
{
    ActorMotionState state;
    ActorMotionData motionData;
    motionData.moveAbilityData = MoveAbilityData();
    motionData.jumpAbilityData = JumpAbilityData();
    motionData.dashAbilityData = DashAbilityData();
    motionData.wallSlideAbilityData = WallSlideAbilityData();
    motionData.wallJumpAbilityData = WallJumpAbilityData();
    motionData.climbAbilityData = ClimbAbilityData();
    motionData.climbMoveAbilityData = ClimbMoveAbilityData();
    motionData.gravityAbilityData = GravityAbilityData();
    AbilitySystem abilitySystem(motionData);
    InputIntentions inputIntentions;

    SECTION("Can move and jump")
    {
        state.contacts.onGround = true;
        inputIntentions.direction = glm::vec2(1.0f, 0.0f);
        inputIntentions.jumpRequested = true;
        simulateMovement(abilitySystem, 0.01f, inputIntentions, state);
        REQUIRE(state.targetVelocity.x == Approx(motionData.moveAbilityData->moveSpeed));
        REQUIRE(state.targetVelocity.y == Approx(motionData.jumpAbilityData->jumpSpeed));
        REQUIRE(state.jump.active == true);
    }

    SECTION("Can jump, wall slide then wall jump")
    {
        state.contacts.onGround = true;
        state.contacts.touchingLeftWall = true;
        inputIntentions.jumpRequested = true;
        simulateMovement(abilitySystem, 0.01f, inputIntentions, state);
        state.contacts.onGround = false;
        REQUIRE(state.jump.active);
        REQUIRE_FALSE(state.wallJump.active);
        REQUIRE(state.targetVelocity.y == Approx(motionData.jumpAbilityData->jumpSpeed));
        REQUIRE(state.targetVelocity.x == Approx(0.0f));
        inputIntentions = InputIntentions();
        simulateMovement(abilitySystem, motionData.jumpAbilityData->jumpDuration, inputIntentions, state);
        REQUIRE_FALSE(state.jump.active);
        simulateMovement(abilitySystem, motionData.jumpAbilityData->jumpDuration, inputIntentions, state);
        REQUIRE(state.wallSlide.active);
        REQUIRE(state.targetVelocity.y == Approx(motionData.wallSlideAbilityData->slideSpeed));
        REQUIRE(state.targetVelocity.x == Approx(0.0f));
        inputIntentions.jumpHeld = true;
        inputIntentions.direction = glm::vec2(1.0f, 0.0f);
        simulateMovement(abilitySystem, 0.01f, inputIntentions, state);
        REQUIRE(state.wallJump.active);
        REQUIRE(state.targetVelocity.y == Approx(motionData.wallJumpAbilityData->wallJumpSpeed));
        REQUIRE(state.targetVelocity.x == Approx(state.wallJump.direction * motionData.wallJumpAbilityData->wallJumpHorizontalSpeed));
    }

    SECTION("Can dash into wall then wall jump")
    {
        state.contacts.onGround = false;
        state.contacts.touchingLeftWall = false;
        inputIntentions.dashRequested = true;
        inputIntentions.direction.x = 1.0f;
        simulateMovement(abilitySystem, 0.01f, inputIntentions, state);
        REQUIRE(state.dash.active);
        REQUIRE(state.targetVelocity.x == Approx(motionData.dashAbilityData->dashSpeed));
        state.contacts.touchingLeftWall = true;
        inputIntentions = InputIntentions();
        simulateMovement(abilitySystem, 0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.dash.active);
        REQUIRE(state.targetVelocity.x == Approx(0.0f));
        simulateMovement(abilitySystem, 0.01f, inputIntentions, state);
        REQUIRE(state.wallSlide.active);
        REQUIRE(state.targetVelocity.y == Approx(motionData.wallSlideAbilityData->slideSpeed));
        inputIntentions.jumpHeld = true;
        inputIntentions.direction = glm::vec2(1.0f, 0.0f);
        simulateMovement(abilitySystem, 0.01f, inputIntentions, state);
        REQUIRE(state.wallJump.active);
        REQUIRE(state.targetVelocity.y == Approx(motionData.wallJumpAbilityData->wallJumpSpeed));
        REQUIRE(state.targetVelocity.x == Approx(state.wallJump.direction * motionData.wallJumpAbilityData->wallJumpHorizontalSpeed));
    }

    SECTION("Cannot jump while dashing")
    {
        inputIntentions.dashRequested = true;
        inputIntentions.direction.x = 1.0f;
        simulateMovement(abilitySystem, 0.01f, inputIntentions, state);
        REQUIRE(state.dash.active);
        REQUIRE(state.targetVelocity.x == Approx(motionData.dashAbilityData->dashSpeed));
        inputIntentions.jumpRequested = true;
        simulateMovement(abilitySystem, 0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.jump.active);
        REQUIRE(state.dash.active);
    }

    SECTION("Cannot move while dashing")
    {
        inputIntentions.dashRequested = true;
        inputIntentions.direction.x = 1.0f;
        simulateMovement(abilitySystem, 0.01f, inputIntentions, state);
        REQUIRE(state.dash.active);
        REQUIRE(state.targetVelocity.x == Approx(motionData.dashAbilityData->dashSpeed));
        inputIntentions.direction = glm::vec2(1.0f, 0.0f);
        simulateMovement(abilitySystem, 0.01f, inputIntentions, state);
        REQUIRE(state.targetVelocity.x == Approx(motionData.dashAbilityData->dashSpeed));
    }

    SECTION("Can jump and dash")
    {
        state.contacts.onGround = true;
        inputIntentions.jumpRequested = true;
        simulateMovement(abilitySystem, 0.01f, inputIntentions, state);
        REQUIRE(state.jump.active);
        REQUIRE(state.targetVelocity.y == Approx(motionData.jumpAbilityData->jumpSpeed));
        inputIntentions = InputIntentions();
        inputIntentions.dashRequested = true;
        inputIntentions.direction.x = 1.0f;
        simulateMovement(abilitySystem, 0.01f, inputIntentions, state);
        REQUIRE(state.dash.active);
        REQUIRE(state.targetVelocity.x == Approx(motionData.dashAbilityData->dashSpeed));
        REQUIRE(state.targetVelocity.y == Approx(0.0f));
    }

    SECTION("Can move right and dash")
    {
        inputIntentions.direction = glm::vec2(1.0f, 0.0f);
        simulateMovement(abilitySystem, 0.01f, inputIntentions, state);
        REQUIRE(state.targetVelocity.x == Approx(motionData.moveAbilityData->moveSpeed));
        inputIntentions.dashRequested = true;
        simulateMovement(abilitySystem, 0.01f, inputIntentions, state);
        REQUIRE(state.dash.active);
        REQUIRE(state.targetVelocity.x == Approx(motionData.dashAbilityData->dashSpeed));
    }

    SECTION("Can move left and dash")
    {
        inputIntentions.direction = glm::vec2(-1.0f, 0.0f);
        simulateMovement(abilitySystem, 0.01f, inputIntentions, state);
        REQUIRE(state.targetVelocity.x == Approx(-motionData.moveAbilityData->moveSpeed));
        inputIntentions.dashRequested = true;
        simulateMovement(abilitySystem, 0.01f, inputIntentions, state);
        REQUIRE(state.dash.active);
        REQUIRE(state.targetVelocity.x == Approx(-motionData.dashAbilityData->dashSpeed));
    }

    SECTION("Gravity is not applied when dashing")
    {
        inputIntentions.dashRequested = true;
        inputIntentions.direction.x = 1.0f;
        simulateMovement(abilitySystem, 0.01f, inputIntentions, state);
        REQUIRE(state.targetVelocity.y == Approx(0.0f));
        simulateMovement(abilitySystem, 0.01f, inputIntentions, state);
        REQUIRE(state.targetVelocity.y == Approx(0.0f));
    }

    SECTION("Gravity is applied when not dashing")
    {
        inputIntentions.dashRequested = true;
        inputIntentions.direction.x = 1.0f;
        simulateMovement(abilitySystem, motionData.dashAbilityData->dashDuration + 0.01f, inputIntentions, state);
        REQUIRE(state.targetVelocity.y == Approx(motionData.gravityAbilityData->gravity * (motionData.dashAbilityData->dashDuration + 0.01f)));
        simulateMovement(abilitySystem, 0.01f, inputIntentions, state);
        REQUIRE(state.targetVelocity.y == Approx(motionData.gravityAbilityData->gravity * (motionData.dashAbilityData->dashDuration + 0.02f)));
    }
}