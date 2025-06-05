#include <catch2/catch_test_macros.hpp>
#include "game/player/movement_abilities/wall_slide_ability.hpp"
#include "game/player/movement_abilities/wall_slide_ability_data.hpp"
#include "game/player/player_state.hpp"
#include "test_helpers/mock_player.hpp"

TEST_CASE("WallSlideAbility activates on wall contact while falling", "[WallSlideAbility]")
{
    PlayerState playerState;
    MockPlayer mockPlayer;
    WallSlideAbilityData wallSlideAbilityData;
    WallSlideAbility slideAbility(wallSlideAbilityData);
    
    mockPlayer.setVelocity(glm::vec2(0.0f, 300.0f));

    SECTION("Not wallSliding when not touching walls")
    {
        playerState.touchingLeftWall = false;
        playerState.touchingRightWall = false;
        slideAbility.update(mockPlayer, playerState, 0.1f);
        slideAbility.syncState(playerState);
        REQUIRE_FALSE(playerState.wallSliding);
        REQUIRE(mockPlayer.getVelocity().y == 300.0f);
    }

    SECTION("Slides when touching left wall while falling")
    {
        playerState.touchingLeftWall = true;
        slideAbility.fixedUpdate(mockPlayer, playerState, 0.1f);
        slideAbility.update(mockPlayer, playerState, 0.1f);
        slideAbility.syncState(playerState);
        REQUIRE(playerState.wallSliding);
        REQUIRE(mockPlayer.getVelocity().y < 300.0f);
    }

    SECTION("Slides when touching right wall while falling")
    {
        playerState.touchingRightWall = true;
        slideAbility.fixedUpdate(mockPlayer, playerState, 0.1f);
        slideAbility.update(mockPlayer, playerState, 0.1f);
        slideAbility.syncState(playerState);
        REQUIRE(playerState.wallSliding);
        REQUIRE(mockPlayer.getVelocity().y < 300.0f);
    }

    SECTION("No slide when on ground")
    {
        playerState.onGround = true;
        playerState.touchingRightWall = true;
        slideAbility.update(mockPlayer, playerState, 0.1f);
        slideAbility.syncState(playerState);
        REQUIRE_FALSE(playerState.wallSliding);
    }

    SECTION("No slide when exceed hang time, hang time resets when on ground")
    {
        playerState.touchingLeftWall = true;
        slideAbility.fixedUpdate(mockPlayer, playerState, slideAbility.getHangDuration() + 0.01f);
        slideAbility.update(mockPlayer, playerState, slideAbility.getHangDuration() + 0.01f);
        slideAbility.syncState(playerState);
        REQUIRE_FALSE(playerState.wallSliding);

        playerState.onGround = true;
        slideAbility.fixedUpdate(mockPlayer, playerState, 0.1f);
        slideAbility.update(mockPlayer, playerState, 0.1f);

        playerState.onGround = false;
        playerState.touchingLeftWall = true;
        slideAbility.fixedUpdate(mockPlayer, playerState, 0.1f);
        slideAbility.update(mockPlayer, playerState, 0.1f);
        slideAbility.syncState(playerState);
        REQUIRE(playerState.wallSliding);
    }
}