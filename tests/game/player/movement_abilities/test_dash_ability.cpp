#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "game/player/movement_abilities/dash_ability.hpp"
#include "game/player/movement_abilities/dash_ability_data.hpp"
#include "game/player/player_state.hpp"
#include "test_helpers/mock_player.hpp"

TEST_CASE("DashAbility basic behavior", "[DashAbility]")
{
    PlayerState playerState;
    MockPlayer mockPlayer;
    DashAbilityData dashAbilityData;
    DashAbility dashAbility(dashAbilityData);

    SECTION("Can dash when not cooling down")
    {
        dashAbility.tryDash(mockPlayer, playerState);
        REQUIRE(dashAbility.dashing());
    }

    SECTION("Dash applies speed to the left")
    {
        playerState.facingLeft = true;
        dashAbility.tryDash(mockPlayer, playerState);
        dashAbility.fixedUpdate(mockPlayer, playerState, 0.1f);
        dashAbility.update(mockPlayer, playerState, 0.1f);
        REQUIRE(mockPlayer.getVelocity().x < 0);
    }

    SECTION("Dash applies speed to the right")
    {
        playerState.facingLeft = false;
        dashAbility.tryDash(mockPlayer, playerState);
        dashAbility.fixedUpdate(mockPlayer, playerState, 0.1f);
        dashAbility.update(mockPlayer, playerState, 0.1f);
        REQUIRE(mockPlayer.getVelocity().x > 0);
    }

    SECTION("Dash ends after duration")
    {
        dashAbility.tryDash(mockPlayer, playerState);
        dashAbility.fixedUpdate(mockPlayer, playerState, dashAbility.getDashDuration() + 0.01f);
        dashAbility.update(mockPlayer, playerState, dashAbility.getDashDuration() + 0.01f);
        REQUIRE_FALSE(dashAbility.dashing());
    }

    SECTION("Cannot dash again before cooldown")
    {
        dashAbility.tryDash(mockPlayer, playerState);
        dashAbility.fixedUpdate(mockPlayer, playerState, dashAbility.getDashCooldown() - 0.01f);
        dashAbility.update(mockPlayer, playerState, dashAbility.getDashCooldown() - 0.01f);
        dashAbility.tryDash(mockPlayer, playerState);
        REQUIRE_FALSE(dashAbility.dashing());
    }

    SECTION("Can dash again after cooldown")
    {
        dashAbility.tryDash(mockPlayer, playerState);
        dashAbility.fixedUpdate(mockPlayer, playerState, dashAbility.getDashCooldown() + 0.01f);
        dashAbility.update(mockPlayer, playerState, dashAbility.getDashCooldown() + 0.01f);
        dashAbility.tryDash(mockPlayer, playerState);
        REQUIRE(dashAbility.dashing());
    }
}