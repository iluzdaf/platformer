#include <catch2/catch_test_macros.hpp>
#include "game/player/movement_abilities/dash_ability.hpp"
#include "game/player/movement_abilities/dash_ability_data.hpp"
#include "glm/vec2.hpp"
#include "test_helpers/mock_player.hpp"

TEST_CASE("DashAbility basic behavior", "[DashAbility]")
{
    MockPlayer mockPlayer;
    DashAbilityData dashAbilityData;
    DashAbility dashAbility(dashAbilityData);

    SECTION("Can dash when not cooling down")
    {
        dashAbility.tryDash(mockPlayer);
        REQUIRE(dashAbility.dashing());
    }

    SECTION("Dash applies speed to the left")
    {
        mockPlayer.setFacingLeft(false);
        dashAbility.tryDash(mockPlayer);
        dashAbility.update(mockPlayer, 0.1f);
        REQUIRE(mockPlayer.getVelocity().x > 0);
    }

    SECTION("Dash applies speed to the right")
    {
        mockPlayer.setFacingLeft(true);
        dashAbility.tryDash(mockPlayer);
        dashAbility.update(mockPlayer, 0.1f);
        REQUIRE(mockPlayer.getVelocity().x < 0);
    }

    SECTION("Dash ends after duration")
    {
        mockPlayer.setFacingLeft(false);
        dashAbility.tryDash(mockPlayer);

        float time = 0.0f;
        while (time < dashAbility.getDashDuration() + 0.1f)
        {
            dashAbility.update(mockPlayer, 0.1f);
            time += 0.1f;
        }

        REQUIRE_FALSE(dashAbility.dashing());
    }

    SECTION("Cannot dash again immediately")
    {
        dashAbility.tryDash(mockPlayer);
        dashAbility.update(mockPlayer, dashAbility.getDashDuration());

        dashAbility.tryDash(mockPlayer);
        REQUIRE_FALSE(dashAbility.dashing());
    }

    SECTION("Can dash again after cooldown")
    {
        dashAbility.tryDash(mockPlayer);
        float totalTime = dashAbility.getDashDuration() + dashAbility.getDashCooldown();
        float time = 0.0f;

        while (time < totalTime)
        {
            dashAbility.update(mockPlayer, 0.1f);
            time += 0.1f;
        }

        dashAbility.tryDash(mockPlayer);
        REQUIRE(dashAbility.dashing());
    }
}