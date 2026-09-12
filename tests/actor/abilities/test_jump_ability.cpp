#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "actor/abilities/jump_ability.hpp"
#include "actor/abilities/jump_ability_data.hpp"
#include "actor/abilities/ability_states.hpp"
#include "actor/observed.hpp"
#include "helpers/abilities.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

namespace
{
    JumpAbilityData timedAs(float duration, float buffer, float coyote)
    {
        JumpAbilityData data;
        data.jumpDuration = duration;
        data.jumpBufferDuration = buffer;
        data.jumpCoyoteDuration = coyote;
        return data;
    }

    bool ended(const AbilityStates &states)
    {
        return !states.jump.active;
    }

    Observed underACeiling()
    {
        Observed observed = inTheAir();
        observed.contacts.hitCeiling = true;
        return observed;
    }
}

TEST_CASE("A jump sets off from the ground when pressed, at its speed", "[JumpAbility]")
{
    JumpAbilityData data;
    JumpAbility jump(data);
    AbilityStates states;

    tick(jump, pressingJump(), onTheGround(), states);

    REQUIRE(states.jump.active);
    REQUIRE(states.jump.velocity.y == Approx(data.jumpSpeed));
}

TEST_CASE("A held jump goes up for as long as it lasts, and then ends", "[JumpAbility]")
{
    JumpAbilityData data = timedAs(0.045f, 0.1f, 0.1f);
    JumpAbility jump(data);
    AbilityStates states;
    tick(jump, pressingJump(), onTheGround(), states);

    int ticksUntilItEnds = ticksUntil(jump, holdingJump(), inTheAir(), states, ended);

    REQUIRE(ticksUntilItEnds == 4);
    REQUIRE(states.jump.velocity.y == 0.0f);
}

TEST_CASE("Letting go of jump ends it early", "[JumpAbility]")
{
    JumpAbility jump(JumpAbilityData{});
    AbilityStates states;
    tick(jump, pressingJump(), onTheGround(), states);

    tick(jump, InputIntentions{}, inTheAir(), states);

    REQUIRE_FALSE(states.jump.active);
    REQUIRE(states.jump.velocity.y == 0.0f);
}

TEST_CASE("A head against a ceiling ends a jump, and it stays ended", "[JumpAbility]")
{
    JumpAbility jump(JumpAbilityData{});
    AbilityStates states;
    tick(jump, pressingJump(), onTheGround(), states);

    tick(jump, holdingJump(), underACeiling(), states);
    REQUIRE_FALSE(states.jump.active);
    REQUIRE(states.jump.velocity.y == 0.0f);

    tick(jump, holdingJump(), inTheAir(), states);
    REQUIRE(states.jump.velocity.y == 0.0f);
}

TEST_CASE("A jump cannot set off from the air", "[JumpAbility]")
{
    JumpAbility jump(timedAs(0.2f, 0.1f, 0.1f));
    AbilityStates states;

    tick(jump, pressingJump(), inTheAir(), states);

    REQUIRE_FALSE(states.jump.active);
    REQUIRE(states.jump.velocity.y == 0.0f);
}

TEST_CASE("A jump pressed just before landing goes on landing", "[JumpAbility]")
{
    JumpAbility jump(timedAs(0.2f, 0.045f, 0.045f));
    AbilityStates states;
    tick(jump, pressingJump(), inTheAir(), states);
    tick(jump, holdingJump(), inTheAir(), states, 3);

    tick(jump, holdingJump(), onTheGround(), states);

    REQUIRE(states.jump.active);
}

TEST_CASE("A jump pressed too long before landing is forgotten", "[JumpAbility]")
{
    JumpAbility jump(timedAs(0.2f, 0.045f, 0.045f));
    AbilityStates states;
    tick(jump, pressingJump(), inTheAir(), states);
    tick(jump, holdingJump(), inTheAir(), states, 4);

    tick(jump, holdingJump(), onTheGround(), states);

    REQUIRE_FALSE(states.jump.active);
}

TEST_CASE("A jump can still set off just after walking off a ledge", "[JumpAbility]")
{
    JumpAbility jump(timedAs(0.2f, 0.045f, 0.045f));
    AbilityStates states;
    tick(jump, InputIntentions{}, onTheGround(), states);
    tick(jump, InputIntentions{}, inTheAir(), states, 3);

    tick(jump, pressingJump(), inTheAir(), states);

    REQUIRE(states.jump.active);
}

TEST_CASE("Too long after walking off a ledge, a jump does not set off", "[JumpAbility]")
{
    JumpAbility jump(timedAs(0.2f, 0.045f, 0.045f));
    AbilityStates states;
    tick(jump, InputIntentions{}, onTheGround(), states);
    tick(jump, InputIntentions{}, inTheAir(), states, 4);

    tick(jump, pressingJump(), inTheAir(), states);

    REQUIRE_FALSE(states.jump.active);
}

TEST_CASE("A jump uses up the leeway for walking off a ledge", "[JumpAbility]")
{
    JumpAbility jump(timedAs(0.2f, 0.1f, 0.1f));
    AbilityStates states;
    tick(jump, pressingJump(), onTheGround(), states);
    tick(jump, InputIntentions{}, inTheAir(), states);

    tick(jump, pressingJump(), inTheAir(), states);

    REQUIRE_FALSE(states.jump.active);
}

TEST_CASE("Pressing jump again mid-jump does not make it last longer", "[JumpAbility]")
{
    JumpAbilityData data = timedAs(0.045f, 0.1f, 0.1f);

    JumpAbility held(data);
    AbilityStates heldStates;
    tick(held, pressingJump(), onTheGround(), heldStates);
    int heldFor = ticksUntil(held, holdingJump(), inTheAir(), heldStates, ended);

    JumpAbility pressedAgain(data);
    AbilityStates pressedAgainStates;
    tick(pressedAgain, pressingJump(), onTheGround(), pressedAgainStates);
    int pressedAgainFor =
        ticksUntil(pressedAgain, pressingJump(), inTheAir(), pressedAgainStates, ended);

    REQUIRE(pressedAgainFor == heldFor);
}

TEST_CASE("Holding jump through a landing does not jump again", "[JumpAbility]")
{
    JumpAbility jump(timedAs(0.045f, 0.1f, 0.1f));
    AbilityStates states;
    tick(jump, pressingJump(), onTheGround(), states);
    ticksUntil(jump, holdingJump(), inTheAir(), states, ended);

    tick(jump, holdingJump(), onTheGround(), states);

    REQUIRE_FALSE(states.jump.active);
}

TEST_CASE("A jump that does not go up is refused", "[JumpAbility]")
{
    JumpAbilityData downwards;
    downwards.jumpSpeed = 0.0f;

    REQUIRE_THROWS_WITH(
        JumpAbility(downwards), Catch::Matchers::ContainsSubstring("A jump needs a speed upward"));
}

TEST_CASE("A jump whose press or ledge is forgiven for no time is refused", "[JumpAbility]")
{
    REQUIRE_THROWS_WITH(
        JumpAbility(timedAs(0.2f, 0.0f, 0.1f)),
        Catch::Matchers::ContainsSubstring("A jump's buffer needs a length above 0"));
    REQUIRE_THROWS_WITH(
        JumpAbility(timedAs(0.2f, 0.1f, 0.0f)),
        Catch::Matchers::ContainsSubstring("A jump's leeway off a ledge needs a length above 0"));
}
