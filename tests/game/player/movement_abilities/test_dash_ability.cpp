#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "game/player/movement_abilities/dash_ability.hpp"
#include "game/player/movement_abilities/dash_ability_data.hpp"
#include "game/player/player_state.hpp"
#include "test_helpers/mock_player.hpp"
using Catch::Approx;

TEST_CASE("DashAbility basic movement behavior", "[DashAbility]")
{
    PlayerState playerState;
    MockPlayer movementContext;
    DashAbilityData dashAbilityData;
    DashAbility dashAbility(dashAbilityData);

    SECTION("Can dash when not cooling down")
    {
        dashAbility.tryDash(movementContext, playerState);
        REQUIRE(dashAbility.dashing());
    }

    SECTION("Dash applies speed to the left")
    {
        playerState.facingLeft = true;
        dashAbility.tryDash(movementContext, playerState);
        dashAbility.fixedUpdate(movementContext, playerState, 0.1f);
        dashAbility.update(movementContext, playerState, 0.1f);
        REQUIRE(movementContext.getVelocity().x < 0);
    }

    SECTION("Dash applies speed to the right")
    {
        playerState.facingLeft = false;
        dashAbility.tryDash(movementContext, playerState);
        dashAbility.fixedUpdate(movementContext, playerState, 0.1f);
        dashAbility.update(movementContext, playerState, 0.1f);
        REQUIRE(movementContext.getVelocity().x > 0);
    }

    SECTION("Dash ends after duration")
    {
        dashAbility.tryDash(movementContext, playerState);
        dashAbility.fixedUpdate(movementContext, playerState, dashAbility.getDashDuration() + 0.01f);
        dashAbility.update(movementContext, playerState, dashAbility.getDashDuration() + 0.01f);
        REQUIRE_FALSE(dashAbility.dashing());
    }

    SECTION("Cannot dash again before cooldown")
    {
        dashAbility.tryDash(movementContext, playerState);
        dashAbility.fixedUpdate(movementContext, playerState, dashAbility.getDashCooldown() - 0.01f);
        dashAbility.update(movementContext, playerState, dashAbility.getDashCooldown() - 0.01f);
        dashAbility.tryDash(movementContext, playerState);
        REQUIRE_FALSE(dashAbility.dashing());
    }

    SECTION("Can dash again after cooldown")
    {
        dashAbility.tryDash(movementContext, playerState);
        dashAbility.fixedUpdate(movementContext, playerState, dashAbility.getDashCooldown() + 0.01f);
        dashAbility.update(movementContext, playerState, dashAbility.getDashCooldown() + 0.01f);
        dashAbility.tryDash(movementContext, playerState);
        REQUIRE(dashAbility.dashing());
    }

    SECTION("Dash cancels when touching wall")
    {
        dashAbility.tryDash(movementContext, playerState);
        dashAbility.fixedUpdate(movementContext, playerState, 0.01f);
        dashAbility.update(movementContext, playerState, 0.01f);
        playerState.touchingLeftWall = true;
        dashAbility.fixedUpdate(movementContext, playerState, 0.01f);
        dashAbility.update(movementContext, playerState, 0.01f);
        REQUIRE_FALSE(dashAbility.dashing());
    }

    SECTION("Dashing again mid-dash should not change dashTimeLeft")
    {
        dashAbility.tryDash(movementContext, playerState);
        dashAbility.fixedUpdate(movementContext, playerState, 0.01f);
        dashAbility.update(movementContext, playerState, 0.01f);
        dashAbility.tryDash(movementContext, playerState);
        dashAbility.fixedUpdate(movementContext, playerState, 0.01f);
        dashAbility.update(movementContext, playerState, 0.01f);
        REQUIRE(dashAbility.getDashTimeLeft() == Approx(dashAbility.getDashDuration() - 0.02f));
    }

    SECTION("Cannot dash while touching wall")
    {
        playerState.touchingLeftWall = true;
        dashAbility.tryDash(movementContext, playerState);
        REQUIRE_FALSE(dashAbility.dashing());
    }

    SECTION("Cannot dash while wall jumping")
    {
        playerState.wallJumping = true;
        dashAbility.tryDash(movementContext, playerState);
        REQUIRE_FALSE(dashAbility.dashing());
    }
}