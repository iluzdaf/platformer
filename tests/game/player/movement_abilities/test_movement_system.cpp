#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/player/movement_abilities/movement_system.hpp"
#include "game/player/movement_abilities/move_ability.hpp"
#include "game/player/movement_abilities/jump_ability.hpp"
#include "game/player/movement_abilities/dash_ability.hpp"
#include "game/player/movement_abilities/wall_slide_ability.hpp"
#include "game/player/movement_abilities/wall_jump_ability.hpp"
#include "game/player/movement_abilities/climb_ability.hpp"
#include "game/player/movement_abilities/climb_move_ability.hpp"
#include "game/player/player_state.hpp"

using Catch::Approx;

void simulateMovement(
    PlayerState &playerState,
    InputIntentions &inputIntentions,
    MovementSystem &movementSystem,
    float deltaTime)
{
    playerState.targetVelocity.y = 980.0f * deltaTime;

    movementSystem.fixedUpdate(playerState, inputIntentions, deltaTime);

    playerState.velocity = playerState.targetVelocity;
}

TEST_CASE("MovementSystem basic functionality", "[MovementSystem]")
{
    PlayerState playerState;
    MovementSystem movementSystem;
    MoveAbilityData moveData;
    auto moveAbility = std::make_unique<MoveAbility>(moveData);
    movementSystem.addAbility(std::move(moveAbility));
    JumpAbilityData jumpData;
    auto jumpAbility = std::make_unique<JumpAbility>(jumpData);
    movementSystem.addAbility(std::move(jumpAbility));
    DashAbilityData dashData;
    auto dashAbility = std::make_unique<DashAbility>(dashData);
    movementSystem.addAbility(std::move(dashAbility));
    WallSlideAbilityData wallSlideData;
    auto wallSlideAbility = std::make_unique<WallSlideAbility>(wallSlideData);
    movementSystem.addAbility(std::move(wallSlideAbility));
    WallJumpAbilityData wallJumpData;
    auto wallJumpAbility = std::make_unique<WallJumpAbility>(wallJumpData);
    movementSystem.addAbility(std::move(wallJumpAbility));
    ClimbAbilityData climbData;
    auto climbAbility = std::make_unique<ClimbAbility>(climbData);
    movementSystem.addAbility(std::move(climbAbility));
    ClimbMoveAbilityData climbMoveData;
    auto climbMoveAbility = std::make_unique<ClimbMoveAbility>(climbMoveData);
    movementSystem.addAbility(std::move(climbMoveAbility));
    InputIntentions inputIntentions;

    SECTION("Can move and jump")
    {
        playerState.onGround = true;
        inputIntentions.direction = glm::vec2(1.0f, 0.0f);
        inputIntentions.jumpRequested = true;
        simulateMovement(playerState, inputIntentions, movementSystem, 0.01f);
        REQUIRE(playerState.targetVelocity.x == Approx(moveData.moveSpeed));
        REQUIRE(playerState.targetVelocity.y == Approx(jumpData.jumpSpeed));
        REQUIRE(playerState.jumping == true);
    }

    SECTION("Can jump, wall slide then wall jump")
    {
        playerState.onGround = true;
        playerState.touchingLeftWall = true;
        inputIntentions.jumpRequested = true;
        simulateMovement(playerState, inputIntentions, movementSystem, 0.01f);
        playerState.onGround = false;
        REQUIRE(playerState.jumping);
        REQUIRE_FALSE(playerState.wallJumping);
        REQUIRE(playerState.targetVelocity.y == Approx(jumpData.jumpSpeed));
        REQUIRE(playerState.targetVelocity.x == Approx(0.0f));
        inputIntentions = InputIntentions();
        simulateMovement(playerState, inputIntentions, movementSystem, jumpData.jumpDuration);
        REQUIRE_FALSE(playerState.jumping);
        simulateMovement(playerState, inputIntentions, movementSystem, jumpData.jumpDuration);
        REQUIRE(playerState.wallSliding);
        REQUIRE(playerState.targetVelocity.y == Approx(wallSlideData.slideSpeed));
        REQUIRE(playerState.targetVelocity.x == Approx(0.0f));
        inputIntentions.jumpHeld = true;
        inputIntentions.direction = glm::vec2(1.0f, 0.0f);
        simulateMovement(playerState, inputIntentions, movementSystem, 0.01f);
        REQUIRE(playerState.wallJumping);
        REQUIRE(playerState.targetVelocity.y == Approx(wallJumpData.wallJumpSpeed));
        REQUIRE(playerState.targetVelocity.x == Approx(playerState.wallJumpDirection * wallJumpData.wallJumpHorizontalSpeed));
    }

    SECTION("Can dash into wall then wall jump")
    {
        playerState.onGround = false;
        playerState.touchingLeftWall = false;
        inputIntentions.dashRequested = true;
        simulateMovement(playerState, inputIntentions, movementSystem, 0.01f);
        REQUIRE(playerState.dashing);
        REQUIRE(playerState.targetVelocity.x == Approx(dashData.dashSpeed));
        playerState.touchingLeftWall = true;
        inputIntentions = InputIntentions();
        simulateMovement(playerState, inputIntentions, movementSystem, 0.01f);
        REQUIRE_FALSE(playerState.dashing);
        REQUIRE(playerState.targetVelocity.x == Approx(0.0f));
        simulateMovement(playerState, inputIntentions, movementSystem, 0.01f);
        REQUIRE(playerState.wallSliding);
        REQUIRE(playerState.targetVelocity.y == Approx(wallSlideData.slideSpeed));
        inputIntentions.jumpHeld = true;
        inputIntentions.direction = glm::vec2(1.0f, 0.0f);
        simulateMovement(playerState, inputIntentions, movementSystem, 0.01f);
        REQUIRE(playerState.wallJumping);
        REQUIRE(playerState.targetVelocity.y == Approx(wallJumpData.wallJumpSpeed));
        REQUIRE(playerState.targetVelocity.x == Approx(playerState.wallJumpDirection * wallJumpData.wallJumpHorizontalSpeed));
    }

    SECTION("Cannot jump while dashing")
    {
        inputIntentions.dashRequested = true;
        simulateMovement(playerState, inputIntentions, movementSystem, 0.01f);
        REQUIRE(playerState.dashing);
        REQUIRE(playerState.targetVelocity.x == Approx(dashData.dashSpeed));
        inputIntentions.jumpRequested = true;
        simulateMovement(playerState, inputIntentions, movementSystem, 0.01f);
        REQUIRE_FALSE(playerState.jumping);
        REQUIRE(playerState.dashing);
    }

    SECTION("Cannot move while dashing")
    {
        inputIntentions.dashRequested = true;
        simulateMovement(playerState, inputIntentions, movementSystem, 0.01f);
        REQUIRE(playerState.dashing);
        REQUIRE(playerState.targetVelocity.x == Approx(dashData.dashSpeed));
        inputIntentions.direction = glm::vec2(1.0f, 0.0f);
        simulateMovement(playerState, inputIntentions, movementSystem, 0.01f);
        REQUIRE(playerState.targetVelocity.x == Approx(dashData.dashSpeed));
    }

    SECTION("Can jump and dash")
    {
        playerState.onGround = true;
        inputIntentions.jumpRequested = true;
        simulateMovement(playerState, inputIntentions, movementSystem, 0.01f);
        REQUIRE(playerState.jumping);
        REQUIRE(playerState.targetVelocity.y == Approx(jumpData.jumpSpeed));
        inputIntentions = InputIntentions();
        inputIntentions.dashRequested = true;
        simulateMovement(playerState, inputIntentions, movementSystem, 0.01f);
        REQUIRE(playerState.dashing);
        REQUIRE(playerState.targetVelocity.x == Approx(dashData.dashSpeed));
        REQUIRE(playerState.targetVelocity.y == Approx(0.0f));
    }

    SECTION("Can move right and dash")
    {
        inputIntentions.direction = glm::vec2(1.0f, 0.0f);
        simulateMovement(playerState, inputIntentions, movementSystem, 0.01f);
        REQUIRE(playerState.targetVelocity.x == Approx(moveData.moveSpeed));
        inputIntentions.dashRequested = true;
        simulateMovement(playerState, inputIntentions, movementSystem, 0.01f);
        REQUIRE(playerState.dashing);
        REQUIRE(playerState.targetVelocity.x == Approx(dashData.dashSpeed));
    }

    SECTION("Can move left and dash")
    {
        playerState.facingLeft = true;
        inputIntentions.direction = glm::vec2(-1.0f, 0.0f);
        simulateMovement(playerState, inputIntentions, movementSystem, 0.01f);
        REQUIRE(playerState.targetVelocity.x == Approx(-moveData.moveSpeed));
        inputIntentions.dashRequested = true;
        simulateMovement(playerState, inputIntentions, movementSystem, 0.01f);
        REQUIRE(playerState.dashing);
        REQUIRE(playerState.targetVelocity.x == Approx(-dashData.dashSpeed));
    }

    SECTION("Movement System event callbacks are triggered")
    {
        bool wallJumpTriggered = false;
        movementSystem.onWallJump.connect([&]
                                          { wallJumpTriggered = true; });
        bool dashTriggered = false;
        movementSystem.onDash.connect([&]
                                      { dashTriggered = true; });
        bool wallSlideTriggered = false;
        movementSystem.onWallSliding.connect([&]
                                             { wallSlideTriggered = true; });

        SECTION("onWallJump")
        {
            playerState.onGround = false;
            playerState.touchingLeftWall = true;
            inputIntentions.jumpHeld = true;
            inputIntentions.direction = glm::vec2(1.0f, 0.0f);
            simulateMovement(playerState, inputIntentions, movementSystem, 0.01f);
            REQUIRE(wallJumpTriggered);
            REQUIRE_FALSE(dashTriggered);
            REQUIRE_FALSE(wallSlideTriggered);
        }

        SECTION("onDash")
        {
            playerState.touchingLeftWall = false;
            inputIntentions.dashRequested = true;
            simulateMovement(playerState, inputIntentions, movementSystem, 0.01f);
            REQUIRE_FALSE(wallJumpTriggered);
            REQUIRE(dashTriggered);
            REQUIRE_FALSE(wallSlideTriggered);
        }

        SECTION("onWallSliding")
        {
            playerState.onGround = false;
            playerState.touchingLeftWall = true;
            simulateMovement(playerState, inputIntentions, movementSystem, 0.01f);
            simulateMovement(playerState, inputIntentions, movementSystem, 0.01f);
            REQUIRE_FALSE(wallJumpTriggered);
            REQUIRE_FALSE(dashTriggered);
            REQUIRE(wallSlideTriggered);
        }
    }

    SECTION("Gravity is not applied when dashing")
    {
        playerState.onGround = false;
        inputIntentions.dashRequested = true;
        simulateMovement(playerState, inputIntentions, movementSystem, 0.01f);
        REQUIRE(playerState.targetVelocity.y == Approx(0.0f));
        simulateMovement(playerState, inputIntentions, movementSystem, 0.01f);
        REQUIRE(playerState.targetVelocity.y == Approx(0.0f));
    }

    SECTION("Gravity is applied when not dashing")
    {
        playerState.onGround = false;
        inputIntentions.dashRequested = true;
        simulateMovement(playerState, inputIntentions, movementSystem, dashData.dashDuration + 0.01f);
        REQUIRE(playerState.targetVelocity.y == Approx(980.0f * (dashData.dashDuration + 0.01f)));
        simulateMovement(playerState, inputIntentions, movementSystem, 0.01f);
        REQUIRE(playerState.targetVelocity.y == Approx(980.0f * 0.01f));
    }
}