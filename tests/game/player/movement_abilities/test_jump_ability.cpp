#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/player/movement_abilities/jump_ability.hpp"
#include "game/player/movement_abilities/jump_ability_data.hpp"
#include "game/player/player_state.hpp"
#include "test_helpers/mock_player.hpp"
using Catch::Approx;

TEST_CASE("JumpAbility respects max jump count", "[JumpAbility]")
{
    MockPlayer mockPlayer;
    PlayerState playerState;
    JumpAbilityData jumpAbilityData;
    JumpAbility jumpAbility(jumpAbilityData);

    SECTION("Player can jump twice but not more")
    {
        playerState.onGround = true;
        jumpAbility.tryJump(mockPlayer, playerState);
        REQUIRE(mockPlayer.getVelocity().y == Approx(jumpAbility.getJumpSpeed()));
        jumpAbility.tryJump(mockPlayer, playerState);
        REQUIRE(mockPlayer.getVelocity().y == Approx(jumpAbility.getJumpSpeed()));
        mockPlayer.setVelocity({0, 0});
        jumpAbility.tryJump(mockPlayer, playerState);
        REQUIRE(mockPlayer.getVelocity().y == Approx(0.0f));
    }

    SECTION("Jump count resets when on ground")
    {
        playerState.onGround = true;
        jumpAbility.tryJump(mockPlayer, playerState);
        jumpAbility.tryJump(mockPlayer, playerState);
        playerState.onGround = true;
        jumpAbility.update(mockPlayer, playerState, 0.01f);
        mockPlayer.setVelocity({0, 0});
        jumpAbility.tryJump(mockPlayer, playerState);
        REQUIRE(mockPlayer.getVelocity().y == Approx(jumpAbility.getJumpSpeed()));
    }
}