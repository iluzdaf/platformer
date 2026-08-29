#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "actor/abilities/mantle_ability_data.hpp"
#include "actor/actor_motion_state.hpp"
#include "actor/abilities/mantle_ability.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

namespace
{
    void hangAtALedgeOnTheRight(ActorMotionState &state, InputIntentions &inputIntentions)
    {
        state.wallHang.active = true;
        state.contacts.touchingRightWall = true;
        state.contacts.ledgeOnRight = true;
        inputIntentions.direction.y = -1.0f;
    }
}

TEST_CASE("Mantling pulls up and over onto the ledge", "[MantleAbility]")
{
    ActorMotionState state;
    InputIntentions inputIntentions;
    MantleAbilityData data;
    MantleAbility mantleAbility(data);

    SECTION("Asking to go up at a ledge starts it, and it pulls straight up first")
    {
        hangAtALedgeOnTheRight(state, inputIntentions);
        mantleAbility.applyMovement(0.01f, inputIntentions, state);

        REQUIRE(state.mantle.active);
        REQUIRE(state.mantle.velocity.y == Approx(-data.mantleSpeed));
        REQUIRE(state.mantle.velocity.x == Approx(0.0f));
    }

    SECTION("Once it is up it carries you over the wall it is pulling you onto")
    {
        hangAtALedgeOnTheRight(state, inputIntentions);
        for (float elapsed = 0.0f; elapsed <= data.mantleDuration * 0.5f; elapsed += 0.01f)
            mantleAbility.applyMovement(0.01f, inputIntentions, state);

        REQUIRE(state.mantle.active);
        REQUIRE(state.mantle.velocity.x == Approx(data.mantleSpeed));
        REQUIRE(state.mantle.velocity.y == Approx(0.0f));
    }

    SECTION("A ledge on the left pulls you left")
    {
        state.wallHang.active = true;
        state.contacts.touchingLeftWall = true;
        state.contacts.ledgeOnLeft = true;
        inputIntentions.direction.y = -1.0f;
        for (float elapsed = 0.0f; elapsed <= data.mantleDuration * 0.5f; elapsed += 0.01f)
            mantleAbility.applyMovement(0.01f, inputIntentions, state);

        REQUIRE(state.mantle.velocity.x == Approx(-data.mantleSpeed));
    }

    SECTION("Halfway up a wall there is no ledge to pull onto")
    {
        state.wallHang.active = true;
        state.contacts.touchingRightWall = true;
        inputIntentions.direction.y = -1.0f;
        mantleAbility.applyMovement(0.01f, inputIntentions, state);

        REQUIRE_FALSE(state.mantle.active);
    }

    SECTION("Not asking to go up leaves you hanging")
    {
        hangAtALedgeOnTheRight(state, inputIntentions);
        inputIntentions.direction.y = 0.0f;
        mantleAbility.applyMovement(0.01f, inputIntentions, state);

        REQUIRE_FALSE(state.mantle.active);
    }

    SECTION("Not holding the wall is not a mantle")
    {
        hangAtALedgeOnTheRight(state, inputIntentions);
        state.wallHang.active = false;
        mantleAbility.applyMovement(0.01f, inputIntentions, state);

        REQUIRE_FALSE(state.mantle.active);
    }

    SECTION("It lasts as long as it says and then lets go")
    {
        hangAtALedgeOnTheRight(state, inputIntentions);
        float elapsed = 0.0f;
        while (elapsed < data.mantleDuration * 2.0f && (elapsed == 0.0f || state.mantle.active))
        {
            mantleAbility.applyMovement(0.01f, inputIntentions, state);
            elapsed += 0.01f;
        }

        REQUIRE_FALSE(state.mantle.active);
        REQUIRE(elapsed == Approx(data.mantleDuration + 0.01f).margin(0.011f));
    }
}

TEST_CASE("A mantle that goes nowhere or takes no time is refused", "[MantleAbility]")
{
    MantleAbilityData noSpeed;
    noSpeed.mantleSpeed = 0.0f;
    REQUIRE_THROWS(MantleAbility(noSpeed));

    MantleAbilityData noTime;
    noTime.mantleDuration = 0.0f;
    REQUIRE_THROWS(MantleAbility(noTime));
}
