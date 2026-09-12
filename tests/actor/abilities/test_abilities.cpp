#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "actor/actor_motion_data.hpp"
#include "actor/ability_states.hpp"
#include "actor/observed.hpp"
#include "actor/hit.hpp"
#include "actor/abilities/abilities.hpp"
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
        Abilities &abilities,
        float deltaTime,
        const InputIntentions &inputIntentions,
        Observed &observed,
        AbilityStates &states)
    {
        observed.velocity = abilities.decide(deltaTime, inputIntentions, observed, states);
    }
}

TEST_CASE("Abilities basic functionality", "[Abilities]")
{
    AbilityStates states;
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
    Abilities abilities(motionData);
    InputIntentions inputIntentions;

    SECTION("Can move and jump")
    {
        observed.contacts.onGround = true;
        inputIntentions.direction = glm::vec2(1.0f, 0.0f);
        inputIntentions.jumpRequested = true;
        simulateMovement(abilities, 0.01f, inputIntentions, observed, states);
        REQUIRE(observed.velocity.x == Approx(motionData.moveAbilityData->moveSpeed));
        REQUIRE(observed.velocity.y == Approx(motionData.jumpAbilityData->jumpSpeed));
        REQUIRE(states.jump.active == true);
    }

    SECTION("Can jump, wall slide then wall jump")
    {
        observed.contacts.onGround = true;
        observed.contacts.touchingLeftWall = observed.contacts.grippableLeftWall = true;
        inputIntentions.jumpRequested = true;
        simulateMovement(abilities, 0.01f, inputIntentions, observed, states);
        observed.contacts.onGround = false;
        REQUIRE(states.jump.active);
        REQUIRE_FALSE(states.wallJump.active);
        REQUIRE(observed.velocity.y == Approx(motionData.jumpAbilityData->jumpSpeed));
        REQUIRE(observed.velocity.x == Approx(0.0f));
        inputIntentions = InputIntentions();
        simulateMovement(
            abilities, motionData.jumpAbilityData->jumpDuration, inputIntentions, observed, states);
        REQUIRE_FALSE(states.jump.active);
        simulateMovement(
            abilities, motionData.jumpAbilityData->jumpDuration, inputIntentions, observed, states);
        REQUIRE(states.wallSlide.active);
        REQUIRE(observed.velocity.y == Approx(motionData.wallSlideAbilityData->slideSpeed));
        REQUIRE(observed.velocity.x == Approx(0.0f));
        inputIntentions.jumpHeld = true;
        inputIntentions.direction = glm::vec2(1.0f, 0.0f);
        simulateMovement(abilities, 0.01f, inputIntentions, observed, states);
        REQUIRE(states.wallJump.active);
        REQUIRE(observed.velocity.y == Approx(motionData.wallJumpAbilityData->wallJumpSpeed));
        REQUIRE(
            observed.velocity.x == Approx(
                                       states.wallJump.direction *
                                       motionData.wallJumpAbilityData->wallJumpHorizontalSpeed));
    }

    SECTION("Can dash into wall then wall jump")
    {
        observed.contacts.onGround = false;
        observed.contacts.touchingLeftWall = observed.contacts.grippableLeftWall = false;
        inputIntentions.dashRequested = true;
        inputIntentions.direction.x = 1.0f;
        simulateMovement(abilities, 0.01f, inputIntentions, observed, states);
        REQUIRE(states.dash.active);
        REQUIRE(observed.velocity.x == Approx(motionData.dashAbilityData->dashSpeed));
        observed.contacts.touchingLeftWall = observed.contacts.grippableLeftWall = true;
        inputIntentions = InputIntentions();
        simulateMovement(abilities, 0.01f, inputIntentions, observed, states);
        REQUIRE_FALSE(states.dash.active);
        REQUIRE(observed.velocity.x == Approx(0.0f));
        simulateMovement(abilities, 0.01f, inputIntentions, observed, states);
        REQUIRE(states.wallSlide.active);
        REQUIRE(observed.velocity.y == Approx(motionData.wallSlideAbilityData->slideSpeed));
        inputIntentions.jumpHeld = true;
        inputIntentions.direction = glm::vec2(1.0f, 0.0f);
        simulateMovement(abilities, 0.01f, inputIntentions, observed, states);
        REQUIRE(states.wallJump.active);
        REQUIRE(observed.velocity.y == Approx(motionData.wallJumpAbilityData->wallJumpSpeed));
        REQUIRE(
            observed.velocity.x == Approx(
                                       states.wallJump.direction *
                                       motionData.wallJumpAbilityData->wallJumpHorizontalSpeed));
    }

    SECTION("Cannot jump while dashing")
    {
        inputIntentions.dashRequested = true;
        inputIntentions.direction.x = 1.0f;
        simulateMovement(abilities, 0.01f, inputIntentions, observed, states);
        REQUIRE(states.dash.active);
        REQUIRE(observed.velocity.x == Approx(motionData.dashAbilityData->dashSpeed));
        inputIntentions.jumpRequested = true;
        simulateMovement(abilities, 0.01f, inputIntentions, observed, states);
        REQUIRE_FALSE(states.jump.active);
        REQUIRE(states.dash.active);
    }

    SECTION("Cannot move while dashing")
    {
        inputIntentions.dashRequested = true;
        inputIntentions.direction.x = 1.0f;
        simulateMovement(abilities, 0.01f, inputIntentions, observed, states);
        REQUIRE(states.dash.active);
        REQUIRE(observed.velocity.x == Approx(motionData.dashAbilityData->dashSpeed));
        inputIntentions.direction = glm::vec2(1.0f, 0.0f);
        simulateMovement(abilities, 0.01f, inputIntentions, observed, states);
        REQUIRE(observed.velocity.x == Approx(motionData.dashAbilityData->dashSpeed));
    }

    SECTION("Can jump and dash")
    {
        observed.contacts.onGround = true;
        inputIntentions.jumpRequested = true;
        simulateMovement(abilities, 0.01f, inputIntentions, observed, states);
        REQUIRE(states.jump.active);
        REQUIRE(observed.velocity.y == Approx(motionData.jumpAbilityData->jumpSpeed));
        inputIntentions = InputIntentions();
        inputIntentions.dashRequested = true;
        inputIntentions.direction.x = 1.0f;
        simulateMovement(abilities, 0.01f, inputIntentions, observed, states);
        REQUIRE(states.dash.active);
        REQUIRE(observed.velocity.x == Approx(motionData.dashAbilityData->dashSpeed));
        REQUIRE(observed.velocity.y == Approx(0.0f));
    }

    SECTION("Can move right and dash")
    {
        inputIntentions.direction = glm::vec2(1.0f, 0.0f);
        simulateMovement(abilities, 0.01f, inputIntentions, observed, states);
        REQUIRE(observed.velocity.x == Approx(motionData.moveAbilityData->moveSpeed));
        inputIntentions.dashRequested = true;
        simulateMovement(abilities, 0.01f, inputIntentions, observed, states);
        REQUIRE(states.dash.active);
        REQUIRE(observed.velocity.x == Approx(motionData.dashAbilityData->dashSpeed));
    }

    SECTION("Can move left and dash")
    {
        inputIntentions.direction = glm::vec2(-1.0f, 0.0f);
        simulateMovement(abilities, 0.01f, inputIntentions, observed, states);
        REQUIRE(observed.velocity.x == Approx(-motionData.moveAbilityData->moveSpeed));
        inputIntentions.dashRequested = true;
        simulateMovement(abilities, 0.01f, inputIntentions, observed, states);
        REQUIRE(states.dash.active);
        REQUIRE(observed.velocity.x == Approx(-motionData.dashAbilityData->dashSpeed));
    }

    SECTION("A knockback owns the velocity over everything else")
    {
        InputIntentions dashing;
        dashing.direction.x = 1;
        dashing.dashRequested = true;
        observed.contacts.onGround = true;
        simulateMovement(abilities, 0.01f, dashing, observed, states);
        REQUIRE(states.dash.active);

        observed.hits.push_back(Hit{1, glm::vec2(-1.0f, 0.0f), false});
        simulateMovement(abilities, 0.01f, dashing, observed, states);

        REQUIRE(states.knockback.active);
        REQUIRE(observed.velocity == states.knockback.velocity);
        REQUIRE(observed.velocity.x < 0.0f);
    }

    SECTION("Gravity is not applied when dashing")
    {
        inputIntentions.dashRequested = true;
        inputIntentions.direction.x = 1.0f;
        simulateMovement(abilities, 0.01f, inputIntentions, observed, states);
        REQUIRE(observed.velocity.y == Approx(0.0f));
        simulateMovement(abilities, 0.01f, inputIntentions, observed, states);
        REQUIRE(observed.velocity.y == Approx(0.0f));
    }

    SECTION("Gravity is applied when not dashing")
    {
        inputIntentions.dashRequested = true;
        inputIntentions.direction.x = 1.0f;
        simulateMovement(
            abilities,
            motionData.dashAbilityData->dashDuration + 0.01f,
            inputIntentions,
            observed,
            states);
        REQUIRE(
            observed.velocity.y == Approx(
                                       motionData.gravityAbilityData->gravity *
                                       (motionData.dashAbilityData->dashDuration + 0.01f)));
        simulateMovement(abilities, 0.01f, inputIntentions, observed, states);
        REQUIRE(
            observed.velocity.y == Approx(
                                       motionData.gravityAbilityData->gravity *
                                       (motionData.dashAbilityData->dashDuration + 0.02f)));
    }
}