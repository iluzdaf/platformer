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
        playerState.onGround = false;
        mockPlayer.setVelocity({0, 0});
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

TEST_CASE("Jump is buffered and consumed on landing", "[JumpAbility]")
{
    MockPlayer mockPlayer;
    PlayerState playerState;
    JumpAbilityData jumpAbilityData;
    jumpAbilityData.jumpBufferDuration = 0.2f;
    JumpAbility jumpAbility(jumpAbilityData);
    playerState.onGround = false;
    jumpAbility.tryJump(mockPlayer, playerState);
    playerState.onGround = true;
    jumpAbility.fixedUpdate(mockPlayer, playerState, 0.1f);
    REQUIRE(mockPlayer.getVelocity().y == Approx(jumpAbility.getJumpSpeed()));
}

TEST_CASE("First jump must be from ground", "[JumpAbility]")
{
    MockPlayer mockPlayer;
    PlayerState playerState;
    JumpAbilityData jumpAbilityData;
    JumpAbility jumpAbility(jumpAbilityData);
    playerState.onGround = false;
    jumpAbility.tryJump(mockPlayer, playerState);
    REQUIRE(mockPlayer.getVelocity().y == Approx(0.0f));
    REQUIRE(playerState.jumpCount == 0);
}

TEST_CASE("Jump is allowed during coyote time", "[JumpAbility]")
{
    MockPlayer mockPlayer;
    PlayerState playerState;
    JumpAbilityData jumpAbilityData;
    jumpAbilityData.jumpCoyoteDuration = 0.2f;
    JumpAbility jumpAbility(jumpAbilityData);
    playerState.onGround = true;
    jumpAbility.fixedUpdate(mockPlayer, playerState, 0.1f);
    jumpAbility.update(mockPlayer, playerState, 0.1f);
    playerState.onGround = false;
    jumpAbility.fixedUpdate(mockPlayer, playerState, 0.09f);
    jumpAbility.update(mockPlayer, playerState, 0.09f);
    jumpAbility.tryJump(mockPlayer, playerState);
    REQUIRE(mockPlayer.getVelocity().y == Approx(jumpAbility.getJumpSpeed()));
}