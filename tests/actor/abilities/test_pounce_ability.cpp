#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/pounce_ability.hpp"
#include "actor/abilities/pounce_ability_data.hpp"
#include "actor/abilities/pounce_ability_state.hpp"
#include "actor/ability_states.hpp"
#include "actor/observed.hpp"
#include "input/input_intentions.hpp"

namespace
{
    constexpr float Step = 0.01f;

    PounceAbilityData aLeapOf(float forward, float upward, int damage = 1)
    {
        PounceAbilityData data;
        data.leap = glm::vec2(forward, upward);
        data.damage = damage;
        return data;
    }

    InputIntentions askingToPounce(float x = 0.0f)
    {
        InputIntentions intentions;
        intentions.attack = std::string(PounceAttack);
        intentions.direction.x = x;
        return intentions;
    }

    Observed onTheGround()
    {
        Observed observed;
        observed.contacts.onGround = true;
        return observed;
    }

    Observed inTheAir()
    {
        return Observed{};
    }
}

TEST_CASE("A pounce launches from the ground when asked, forward and up", "[PounceAbility]")
{
    PounceAbility ability(aLeapOf(150.0f, -200.0f, 2));
    AbilityStates states;

    ability.decide(Step, askingToPounce(1.0f), onTheGround(), states);

    REQUIRE(states.pounce.active);
    REQUIRE(states.pounce.emit);
    REQUIRE(states.pounce.velocity == glm::vec2(150.0f, -200.0f));
    REQUIRE(states.pounce.damage == 2);
}

TEST_CASE("A pounce cannot launch from the air", "[PounceAbility]")
{
    PounceAbility ability(aLeapOf(150.0f, -200.0f));
    AbilityStates states;

    ability.decide(Step, askingToPounce(1.0f), inTheAir(), states);

    REQUIRE_FALSE(states.pounce.active);
}

TEST_CASE("A pounce goes where it was asked, else where the actor faces", "[PounceAbility]")
{
    PounceAbility ability(aLeapOf(150.0f, -200.0f));

    AbilityStates asked;
    ability.decide(Step, askingToPounce(-1.0f), onTheGround(), asked);
    REQUIRE(asked.pounce.velocity.x == -150.0f);

    Observed facingLeft = onTheGround();
    facingLeft.facingLeft = true;
    AbilityStates facing;
    ability.decide(Step, askingToPounce(), facingLeft, facing);
    REQUIRE(facing.pounce.velocity.x == -150.0f);
}

TEST_CASE("A pounce in the air keeps its forward speed and falls with gravity", "[PounceAbility]")
{
    PounceAbility ability(aLeapOf(150.0f, -200.0f));
    AbilityStates states;
    ability.decide(Step, askingToPounce(1.0f), onTheGround(), states);

    states.gravity.velocity.y = 80.0f;
    ability.decide(Step, InputIntentions{}, inTheAir(), states);

    REQUIRE(states.pounce.active);
    REQUIRE(states.pounce.velocity == glm::vec2(150.0f, -120.0f));
}

TEST_CASE("A pounce ends on landing, and not on the ground it launched from", "[PounceAbility]")
{
    PounceAbility ability(aLeapOf(150.0f, -200.0f));
    AbilityStates states;
    ability.decide(Step, askingToPounce(1.0f), onTheGround(), states);

    ability.decide(Step, InputIntentions{}, onTheGround(), states);
    REQUIRE(states.pounce.active);

    ability.decide(Step, InputIntentions{}, inTheAir(), states);
    ability.decide(Step, InputIntentions{}, onTheGround(), states);

    REQUIRE_FALSE(states.pounce.active);
}

TEST_CASE("A pounce says so once, and asking again mid-air starts nothing", "[PounceAbility]")
{
    PounceAbility ability(aLeapOf(150.0f, -200.0f));
    AbilityStates states;
    ability.decide(Step, askingToPounce(1.0f), onTheGround(), states);
    REQUIRE(states.pounce.emit);

    ability.decide(Step, askingToPounce(-1.0f), inTheAir(), states);

    REQUIRE_FALSE(states.pounce.emit);
    REQUIRE(states.pounce.velocity.x == 150.0f);
}

TEST_CASE("A knockback cuts a pounce short", "[PounceAbility]")
{
    PounceAbility ability(aLeapOf(150.0f, -200.0f));
    AbilityStates states;
    ability.decide(Step, askingToPounce(1.0f), onTheGround(), states);

    states.knockback.active = true;
    ability.decide(Step, InputIntentions{}, inTheAir(), states);

    REQUIRE_FALSE(states.pounce.active);
}

TEST_CASE("A pounce with nothing to it is refused", "[PounceAbility]")
{
    REQUIRE_THROWS_WITH(
        PounceAbility(aLeapOf(0.0f, -200.0f)), Catch::Matchers::ContainsSubstring("leap"));
    REQUIRE_THROWS_WITH(
        PounceAbility(aLeapOf(150.0f, 10.0f)), Catch::Matchers::ContainsSubstring("leap"));
    REQUIRE_THROWS_WITH(
        PounceAbility(aLeapOf(150.0f, -200.0f, 0)), Catch::Matchers::ContainsSubstring("damage"));
}

TEST_CASE("A pounce is not started by an attack that is not a pounce", "[PounceAbility]")
{
    PounceAbility ability(aLeapOf(150.0f, -200.0f));
    AbilityStates states;
    InputIntentions swinging;
    swinging.attack = "swing";

    ability.decide(Step, swinging, onTheGround(), states);

    REQUIRE_FALSE(states.pounce.active);
}
