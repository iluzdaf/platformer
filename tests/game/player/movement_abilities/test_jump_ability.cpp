#include <catch2/catch_test_macros.hpp>
#include "game/player/movement_abilities/jump_ability.hpp"
#include "test_helpers/mock_player.hpp"

TEST_CASE("JumpAbility respects max jump count", "[JumpAbility]")
{
    MockPlayer mockPlayer;
    JumpAbility ability(2, -500);

    SECTION("Player can jump twice but not more")
    {
        mockPlayer.onGround = true;
        ability.tryJump(mockPlayer);
        REQUIRE(mockPlayer.velocity.y == Approx(mockPlayer.getJumpSpeed()));

        mockPlayer.onGround = false;
        ability.tryJump(mockPlayer);
        REQUIRE(mockPlayer.velocity.y == Approx(mockPlayer.getJumpSpeed()));

        mockPlayer.velocity = {0, 0};
        ability.tryJump(mockPlayer);
        REQUIRE(mockPlayer.velocity.y == Approx(0.0f));
    }

    SECTION("Jump count resets when on ground")
    {
        mockPlayer.onGround = true;
        ability.update(mockPlayer, 0.016f);
        ability.tryJump(mockPlayer);
        REQUIRE(mockPlayer.velocity.y == Approx(mockPlayer.getJumpSpeed()));

        mockPlayer.onGround = true;
        ability.update(mockPlayer, 0.016f);
        mockPlayer.velocity = {0, 0};
        ability.tryJump(mockPlayer);
        REQUIRE(mockPlayer.velocity.y == Approx(mockPlayer.getJumpSpeed()));
    }
}