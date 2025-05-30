#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/player/movement_abilities/move_ability.hpp"
#include "game/player/movement_abilities/move_ability_data.hpp"
#include "glm/vec2.hpp"
#include "test_helpers/mock_player.hpp"
using Catch::Approx;

TEST_CASE("MoveAbility basic movement behavior", "[MoveAbility]")
{
    MockPlayer mockPlayer;
    MoveAbilityData moveAbilityData;
    MoveAbility moveAbility(moveAbilityData);

    SECTION("Move left sets negative x velocity and facing left")
    {
        moveAbility.tryMoveLeft(mockPlayer);
        moveAbility.update(mockPlayer, 0.1f);

        REQUIRE(mockPlayer.getVelocity().x == Approx(-moveAbility.getMoveSpeed()));
        REQUIRE(mockPlayer.facingLeft());
    }

    SECTION("Move right sets positive x velocity and facing right")
    {
        moveAbility.tryMoveRight(mockPlayer);
        moveAbility.update(mockPlayer, 0.1f);

        REQUIRE(mockPlayer.getVelocity().x == Approx(moveAbility.getMoveSpeed()));
        REQUIRE_FALSE(mockPlayer.facingLeft());
    }

    SECTION("If both directions requested, no movement applied")
    {
        moveAbility.tryMoveLeft(mockPlayer);
        moveAbility.tryMoveRight(mockPlayer);
        moveAbility.update(mockPlayer, 0.1f);

        REQUIRE(mockPlayer.getVelocity().x == Approx(0.0f));
    }

    SECTION("If no direction requested, no movement applied")
    {
        moveAbility.update(mockPlayer, 0.1f);
        REQUIRE(mockPlayer.getVelocity().x == Approx(0.0f));
    }

    SECTION("Movement requests are cleared after update")
    {
        moveAbility.tryMoveLeft(mockPlayer);
        moveAbility.update(mockPlayer, 0.1f);

        mockPlayer.setVelocity({0, 0});
        moveAbility.update(mockPlayer, 0.1f);
        REQUIRE(mockPlayer.getVelocity().x == Approx(0.0f));
    }
}