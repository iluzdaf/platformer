#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/player/movement_abilities/move_ability.hpp"
#include "game/player/movement_abilities/movement_context.hpp"
#include "game/player/player_state.hpp"

using Catch::Approx;

TEST_CASE("MoveAbility basic movement behavior", "[MoveAbility]")
{
    PlayerState playerState;
    MovementContext movementContext;
    MoveAbilityData moveAbilityData;
    MoveAbility moveAbility(moveAbilityData);

    SECTION("Can move left")
    {
        movementContext.inputIntentions.direction.x = -1;
        moveAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE(movementContext.moveVelocity.x == Approx(-moveAbilityData.moveSpeed));
    }

    SECTION("Can move right")
    {
        movementContext.inputIntentions.direction.x = 1;
        moveAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE(movementContext.moveVelocity.x == Approx(moveAbilityData.moveSpeed));
    }

    SECTION("If no direction requested, no movement applied")
    {
        moveAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE(movementContext.moveVelocity.x == Approx(0.0f));
    }
}