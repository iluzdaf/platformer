#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "actor/abilities/ability_states.hpp"
#include "actor/abilities/bite_ability.hpp"
#include "actor/abilities/bite_ability_data.hpp"
#include "helpers/abilities.hpp"
#include "input/input_intentions.hpp"

TEST_CASE("A bite is always ready, with its damage", "[BiteAbility]")
{
    BiteAbility bite(BiteAbilityData{2});
    AbilityStates states;

    tick(bite, InputIntentions{}, onTheGround(), states);
    REQUIRE(states.bite.active);
    REQUIRE(states.bite.damage == 2);

    tick(bite, pressing(1.0f), inTheAir(), states);
    REQUIRE(states.bite.active);
}

TEST_CASE("A bite that does no damage is refused", "[BiteAbility]")
{
    REQUIRE_THROWS_WITH(
        BiteAbility(BiteAbilityData{0}),
        Catch::Matchers::ContainsSubstring("A bite needs damage above 0"));
    REQUIRE_THROWS_WITH(
        BiteAbility(BiteAbilityData{-1}),
        Catch::Matchers::ContainsSubstring("A bite needs damage above 0"));
}
