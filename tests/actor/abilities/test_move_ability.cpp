#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "actor/abilities/move_ability.hpp"
#include "actor/abilities/move_ability_data.hpp"
#include "actor/ability_states.hpp"
#include "helpers/abilities.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

TEST_CASE("A move goes at its speed the way it is pressed", "[MoveAbility]")
{
    MoveAbilityData data;
    MoveAbility move(data);
    AbilityStates states;

    tick(move, pressing(1.0f), onTheGround(), states);
    REQUIRE(states.move.velocity.x == Approx(data.moveSpeed));

    tick(move, pressing(-1.0f), onTheGround(), states);
    REQUIRE(states.move.velocity.x == Approx(-data.moveSpeed));
}

TEST_CASE("However little it is pressed, a move goes at its full speed", "[MoveAbility]")
{
    MoveAbilityData data;
    MoveAbility move(data);
    AbilityStates states;

    tick(move, pressing(0.3f), onTheGround(), states);
    REQUIRE(states.move.velocity.x == Approx(data.moveSpeed));

    tick(move, pressing(-0.3f), onTheGround(), states);
    REQUIRE(states.move.velocity.x == Approx(-data.moveSpeed));
}

TEST_CASE("Letting go stops a move at once", "[MoveAbility]")
{
    MoveAbility move(MoveAbilityData{});
    AbilityStates states;
    tick(move, pressing(1.0f), onTheGround(), states);

    tick(move, InputIntentions{}, onTheGround(), states);

    REQUIRE(states.move.velocity == glm::vec2(0.0f));
}

TEST_CASE("A move that goes nowhere is refused", "[MoveAbility]")
{
    MoveAbilityData noSpeed;
    noSpeed.moveSpeed = 0.0f;

    REQUIRE_THROWS_WITH(
        MoveAbility(noSpeed), Catch::Matchers::ContainsSubstring("A move needs a speed above 0"));
}
