#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/player/movement_abilities/move_ability.hpp"
#include "game/player/movement_abilities/move_ability_data.hpp"
#include "game/player/player_state.hpp"
#include "test_helpers/mock_player.hpp"
using Catch::Approx;

TEST_CASE("MoveAbility basic movement behavior", "[MoveAbility]")
{
    PlayerState playerState;
    MockPlayer movementContext;
    MoveAbilityData moveAbilityData;
    MoveAbility moveAbility(moveAbilityData);

    SECTION("Move left sets negative x velocity and facing left")
    {
        moveAbility.tryMoveLeft(movementContext, playerState);
        moveAbility.fixedUpdate(movementContext, playerState, 0.1f);
        moveAbility.syncState(playerState);
        moveAbility.update(movementContext, playerState, 0.1f);
        REQUIRE(movementContext.getVelocity().x == Approx(-moveAbility.getMoveSpeed()));
        REQUIRE(movementContext.facingLeft);
    }

    SECTION("Move right sets positive x velocity and facing right")
    {
        moveAbility.tryMoveRight(movementContext, playerState);
        moveAbility.fixedUpdate(movementContext, playerState, 0.1f);
        moveAbility.update(movementContext, playerState, 0.1f);
        REQUIRE(movementContext.getVelocity().x == Approx(moveAbility.getMoveSpeed()));
        REQUIRE_FALSE(movementContext.facingLeft);
    }

    SECTION("If both directions requested, no movement applied")
    {
        moveAbility.tryMoveLeft(movementContext, playerState);
        moveAbility.tryMoveRight(movementContext, playerState);
        moveAbility.fixedUpdate(movementContext, playerState, 0.1f);
        moveAbility.update(movementContext, playerState, 0.1f);
        REQUIRE(movementContext.getVelocity().x == Approx(0.0f));
    }

    SECTION("If no direction requested, no movement applied")
    {
        moveAbility.fixedUpdate(movementContext, playerState, 0.1f);
        moveAbility.update(movementContext, playerState, 0.1f);
        REQUIRE(movementContext.getVelocity().x == Approx(0.0f));
    }

    SECTION("Movement requests are cleared after update")
    {
        moveAbility.tryMoveLeft(movementContext, playerState);
        moveAbility.fixedUpdate(movementContext, playerState, 0.1f);
        moveAbility.update(movementContext, playerState, 0.1f);
        movementContext.setVelocity({0, 0});
        moveAbility.fixedUpdate(movementContext, playerState, 0.1f);
        moveAbility.update(movementContext, playerState, 0.1f);
        REQUIRE(movementContext.getVelocity().x == Approx(0.0f));
    }
}