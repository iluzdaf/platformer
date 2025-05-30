#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/player/movement_abilities/jump_ability.hpp"
#include "game/player/movement_abilities/jump_ability_data.hpp"
#include "test_helpers/mock_player.hpp"
using Catch::Approx;

TEST_CASE("JumpAbility respects max jump count", "[JumpAbility]")
{
    MockPlayer mockPlayer;
    JumpAbilityData jumpAbilityData;
    JumpAbility jumpAbility(jumpAbilityData);

    SECTION("Player can jump twice but not more")
    {
        mockPlayer.setOnGround(true);
        jumpAbility.tryJump(mockPlayer);
        REQUIRE(mockPlayer.velocity.y == Approx(jumpAbility.getJumpSpeed()));

        mockPlayer.setOnGround(true);
        jumpAbility.tryJump(mockPlayer);
        REQUIRE(mockPlayer.velocity.y == Approx(jumpAbility.getJumpSpeed()));

        mockPlayer.velocity = {0, 0};
        jumpAbility.tryJump(mockPlayer);
        REQUIRE(mockPlayer.velocity.y == Approx(0.0f));
    }

    SECTION("Jump count resets when on ground")
    {
        mockPlayer.setOnGround(true);
        jumpAbility.update(mockPlayer, 0.016f);
        jumpAbility.tryJump(mockPlayer);
        REQUIRE(mockPlayer.velocity.y == Approx(jumpAbility.getJumpSpeed()));

        mockPlayer.setOnGround(true);
        jumpAbility.update(mockPlayer, 0.016f);
        mockPlayer.velocity = {0, 0};
        jumpAbility.tryJump(mockPlayer);
        REQUIRE(mockPlayer.velocity.y == Approx(jumpAbility.getJumpSpeed()));
    }
}