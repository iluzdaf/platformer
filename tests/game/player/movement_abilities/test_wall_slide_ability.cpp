#include <catch2/catch_test_macros.hpp>
#include "game/player/movement_abilities/wall_slide_ability.hpp"
#include "test_helpers/mock_player.hpp"
#include "game/player/movement_abilities/wall_slide_ability_data.hpp"

TEST_CASE("WallSlideAbility activates on wall contact while falling", "[WallSlideAbility]")
{
    MockPlayer mockPlayer;
    WallSlideAbilityData wallSlideAbilityData;
    WallSlideAbility slideAbility(wallSlideAbilityData);
    mockPlayer.setOnGround(false);
    mockPlayer.setVelocity(glm::vec2(0.0f, 300.0f));

    SECTION("Not wallSliding when not touching walls")
    {
        mockPlayer.setTouchingLeftWall(false);
        mockPlayer.setTouchingRightWall(false);

        slideAbility.update(mockPlayer, 0.1f);

        PlayerState state;
        slideAbility.syncState(state);
        REQUIRE_FALSE(state.wallSliding);
        REQUIRE(mockPlayer.getVelocity().y == 300.0f);
    }

    SECTION("Slides when touching left wall while falling")
    {
        mockPlayer.setTouchingLeftWall(true);

        slideAbility.update(mockPlayer, 0.1f);

        PlayerState state;
        slideAbility.syncState(state);
        REQUIRE(state.wallSliding);
        REQUIRE(mockPlayer.getVelocity().y < 300.0f);
    }

    SECTION("Slides when touching right wall while falling")
    {
        mockPlayer.setTouchingRightWall(true);

        slideAbility.update(mockPlayer, 0.1f);

        PlayerState state;
        slideAbility.syncState(state);
        REQUIRE(state.wallSliding);
        REQUIRE(mockPlayer.getVelocity().y < 300.0f);
    }

    SECTION("No slide when on ground")
    {
        mockPlayer.setOnGround(true);
        mockPlayer.setTouchingRightWall(true);

        slideAbility.update(mockPlayer, 0.1f);

        PlayerState state;
        slideAbility.syncState(state);
        REQUIRE_FALSE(state.wallSliding);
    }
}
