#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "agent/agent_data.hpp"
#include "agent/agent_state.hpp"
#include "agent/movement_abilities/movement_system.hpp"
#include "agent/movement_abilities/move_ability.hpp"
#include "agent/movement_abilities/jump_ability.hpp"
#include "agent/movement_abilities/dash_ability.hpp"
#include "agent/movement_abilities/wall_slide_ability.hpp"
#include "agent/movement_abilities/wall_jump_ability.hpp"
#include "agent/movement_abilities/climb_ability.hpp"
#include "agent/movement_abilities/climb_move_ability.hpp"
#include "agent/movement_abilities/gravity_ability.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

void simulateMovement(
    MovementSystem &movementSystem,
    float deltaTime,
    const InputIntentions &inputIntentions,
    AgentState &state)
{
    movementSystem.applyMovement(deltaTime, inputIntentions, state);

    state.velocity = state.targetVelocity;
}

TEST_CASE("MovementSystem basic functionality", "[MovementSystem]")
{
    AgentState state;
    AgentData agentData;
    agentData.moveAbilityData = MoveAbilityData();
    agentData.jumpAbilityData = JumpAbilityData();
    agentData.dashAbilityData = DashAbilityData();
    agentData.wallSlideAbilityData = WallSlideAbilityData();
    agentData.wallJumpAbilityData = WallJumpAbilityData();
    agentData.climbAbilityData = ClimbAbilityData();
    agentData.climbMoveAbilityData = ClimbMoveAbilityData();
    agentData.gravityAbilityData = GravityAbilityData();
    MovementSystem movementSystem(agentData);
    InputIntentions inputIntentions;

    SECTION("Can move and jump")
    {
        state.onGround = true;
        inputIntentions.direction = glm::vec2(1.0f, 0.0f);
        inputIntentions.jumpRequested = true;
        simulateMovement(movementSystem, 0.01f, inputIntentions, state);
        REQUIRE(state.targetVelocity.x == Approx(agentData.moveAbilityData->moveSpeed));
        REQUIRE(state.targetVelocity.y == Approx(agentData.jumpAbilityData->jumpSpeed));
        REQUIRE(state.jumping == true);
    }

    SECTION("Can jump, wall slide then wall jump")
    {
        state.onGround = true;
        state.touchingLeftWall = true;
        inputIntentions.jumpRequested = true;
        simulateMovement(movementSystem, 0.01f, inputIntentions, state);
        state.onGround = false;
        REQUIRE(state.jumping);
        REQUIRE_FALSE(state.wallJumping);
        REQUIRE(state.targetVelocity.y == Approx(agentData.jumpAbilityData->jumpSpeed));
        REQUIRE(state.targetVelocity.x == Approx(0.0f));
        inputIntentions = InputIntentions();
        simulateMovement(movementSystem, agentData.jumpAbilityData->jumpDuration, inputIntentions, state);
        REQUIRE_FALSE(state.jumping);
        simulateMovement(movementSystem, agentData.jumpAbilityData->jumpDuration, inputIntentions, state);
        REQUIRE(state.wallSliding);
        REQUIRE(state.targetVelocity.y == Approx(agentData.wallSlideAbilityData->slideSpeed));
        REQUIRE(state.targetVelocity.x == Approx(0.0f));
        inputIntentions.jumpHeld = true;
        inputIntentions.direction = glm::vec2(1.0f, 0.0f);
        simulateMovement(movementSystem, 0.01f, inputIntentions, state);
        REQUIRE(state.wallJumping);
        REQUIRE(state.targetVelocity.y == Approx(agentData.wallJumpAbilityData->wallJumpSpeed));
        REQUIRE(state.targetVelocity.x == Approx(state.wallJumpDirection * agentData.wallJumpAbilityData->wallJumpHorizontalSpeed));
    }

    SECTION("Can dash into wall then wall jump")
    {
        state.onGround = false;
        state.touchingLeftWall = false;
        inputIntentions.dashRequested = true;
        simulateMovement(movementSystem, 0.01f, inputIntentions, state);
        REQUIRE(state.dashing);
        REQUIRE(state.targetVelocity.x == Approx(agentData.dashAbilityData->dashSpeed));
        state.touchingLeftWall = true;
        inputIntentions = InputIntentions();
        simulateMovement(movementSystem, 0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.dashing);
        REQUIRE(state.targetVelocity.x == Approx(0.0f));
        simulateMovement(movementSystem, 0.01f, inputIntentions, state);
        REQUIRE(state.wallSliding);
        REQUIRE(state.targetVelocity.y == Approx(agentData.wallSlideAbilityData->slideSpeed));
        inputIntentions.jumpHeld = true;
        inputIntentions.direction = glm::vec2(1.0f, 0.0f);
        simulateMovement(movementSystem, 0.01f, inputIntentions, state);
        REQUIRE(state.wallJumping);
        REQUIRE(state.targetVelocity.y == Approx(agentData.wallJumpAbilityData->wallJumpSpeed));
        REQUIRE(state.targetVelocity.x == Approx(state.wallJumpDirection * agentData.wallJumpAbilityData->wallJumpHorizontalSpeed));
    }

    SECTION("Cannot jump while dashing")
    {
        inputIntentions.dashRequested = true;
        simulateMovement(movementSystem, 0.01f, inputIntentions, state);
        REQUIRE(state.dashing);
        REQUIRE(state.targetVelocity.x == Approx(agentData.dashAbilityData->dashSpeed));
        inputIntentions.jumpRequested = true;
        simulateMovement(movementSystem, 0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.jumping);
        REQUIRE(state.dashing);
    }

    SECTION("Cannot move while dashing")
    {
        inputIntentions.dashRequested = true;
        simulateMovement(movementSystem, 0.01f, inputIntentions, state);
        REQUIRE(state.dashing);
        REQUIRE(state.targetVelocity.x == Approx(agentData.dashAbilityData->dashSpeed));
        inputIntentions.direction = glm::vec2(1.0f, 0.0f);
        simulateMovement(movementSystem, 0.01f, inputIntentions, state);
        REQUIRE(state.targetVelocity.x == Approx(agentData.dashAbilityData->dashSpeed));
    }

    SECTION("Can jump and dash")
    {
        state.onGround = true;
        inputIntentions.jumpRequested = true;
        simulateMovement(movementSystem, 0.01f, inputIntentions, state);
        REQUIRE(state.jumping);
        REQUIRE(state.targetVelocity.y == Approx(agentData.jumpAbilityData->jumpSpeed));
        inputIntentions = InputIntentions();
        inputIntentions.dashRequested = true;
        simulateMovement(movementSystem, 0.01f, inputIntentions, state);
        REQUIRE(state.dashing);
        REQUIRE(state.targetVelocity.x == Approx(agentData.dashAbilityData->dashSpeed));
        REQUIRE(state.targetVelocity.y == Approx(0.0f));
    }

    SECTION("Can move right and dash")
    {
        inputIntentions.direction = glm::vec2(1.0f, 0.0f);
        simulateMovement(movementSystem, 0.01f, inputIntentions, state);
        REQUIRE(state.targetVelocity.x == Approx(agentData.moveAbilityData->moveSpeed));
        inputIntentions.dashRequested = true;
        simulateMovement(movementSystem, 0.01f, inputIntentions, state);
        REQUIRE(state.dashing);
        REQUIRE(state.targetVelocity.x == Approx(agentData.dashAbilityData->dashSpeed));
    }

    SECTION("Can move left and dash")
    {
        inputIntentions.direction = glm::vec2(-1.0f, 0.0f);
        simulateMovement(movementSystem, 0.01f, inputIntentions, state);
        REQUIRE(state.targetVelocity.x == Approx(-agentData.moveAbilityData->moveSpeed));
        inputIntentions.dashRequested = true;
        simulateMovement(movementSystem, 0.01f, inputIntentions, state);
        REQUIRE(state.dashing);
        REQUIRE(state.targetVelocity.x == Approx(-agentData.dashAbilityData->dashSpeed));
    }

    SECTION("Gravity is not applied when dashing")
    {
        inputIntentions.dashRequested = true;
        simulateMovement(movementSystem, 0.01f, inputIntentions, state);
        REQUIRE(state.targetVelocity.y == Approx(0.0f));
        simulateMovement(movementSystem, 0.01f, inputIntentions, state);
        REQUIRE(state.targetVelocity.y == Approx(0.0f));
    }

    SECTION("Gravity is applied when not dashing")
    {
        inputIntentions.dashRequested = true;
        simulateMovement(movementSystem, agentData.dashAbilityData->dashDuration + 0.01f, inputIntentions, state);
        REQUIRE(state.targetVelocity.y == Approx(980.0f * (agentData.dashAbilityData->dashDuration + 0.01f)));
        simulateMovement(movementSystem, 0.01f, inputIntentions, state);
        REQUIRE(state.targetVelocity.y == Approx(980.0f * 0.01f));
    }
}