#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "actor/abilities/abilities.hpp"
#include "actor/abilities/charge_ability.hpp"
#include "actor/abilities/charge_ability_data.hpp"
#include "actor/abilities/charge_ability_state.hpp"
#include "actor/abilities/move_ability_data.hpp"
#include "actor/actor_motion_data.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "input/input_intentions.hpp"

namespace
{
    constexpr float Step = 0.01f;

    ChargeAbilityData aChargeOf(float speed, int damage = 1)
    {
        ChargeAbilityData data;
        data.speed = speed;
        data.damage = damage;
        return data;
    }

    InputIntentions askingToCharge(float x = 0.0f)
    {
        InputIntentions intentions;
        intentions.attack = std::string(ChargeAttack);
        intentions.direction.x = x;
        return intentions;
    }

    Observed onTheGround()
    {
        Observed observed;
        observed.contacts.onGround = true;
        return observed;
    }

    Observed againstAWall(bool onTheRight)
    {
        Observed observed = onTheGround();
        observed.contacts.touchingRightWall = onTheRight;
        observed.contacts.touchingLeftWall = !onTheRight;
        return observed;
    }

    Observed inTheAir()
    {
        return Observed{};
    }
}

TEST_CASE("A charge sets off from the ground when asked, at its speed", "[ChargeAbility]")
{
    ChargeAbility ability(aChargeOf(150.0f, 2));
    Decided decided;
    decided.gravity.velocity.y = 5.0f;

    ability.decide(Step, askingToCharge(1.0f), onTheGround(), decided);

    REQUIRE(decided.charge.active);
    REQUIRE(decided.charge.emit);
    REQUIRE(decided.charge.velocity == glm::vec2(150.0f, 5.0f));
    REQUIRE(decided.charge.damage == 2);
}

TEST_CASE("A charge cannot set off from the air", "[ChargeAbility]")
{
    ChargeAbility ability(aChargeOf(150.0f));
    Decided decided;

    ability.decide(Step, askingToCharge(1.0f), inTheAir(), decided);

    REQUIRE_FALSE(decided.charge.active);
}

TEST_CASE("A charge goes where it was asked, else where the actor faces", "[ChargeAbility]")
{
    ChargeAbility ability(aChargeOf(150.0f));

    Decided asked;
    ability.decide(Step, askingToCharge(-1.0f), onTheGround(), asked);
    REQUIRE(asked.charge.velocity.x == -150.0f);

    Observed facingLeft = onTheGround();
    facingLeft.facingLeft = true;
    Decided facing;
    ability.decide(Step, askingToCharge(), facingLeft, facing);
    REQUIRE(facing.charge.velocity.x == -150.0f);
}

TEST_CASE("A charge keeps running, off a ledge too, and falls as gravity says", "[ChargeAbility]")
{
    ChargeAbility ability(aChargeOf(150.0f));
    Decided decided;
    ability.decide(Step, askingToCharge(1.0f), onTheGround(), decided);

    decided.gravity.velocity.y = 80.0f;
    ability.decide(Step, InputIntentions{}, inTheAir(), decided);

    REQUIRE(decided.charge.active);
    REQUIRE(decided.charge.velocity == glm::vec2(150.0f, 80.0f));
}

TEST_CASE("A charge ends at the wall ahead, and not at one behind", "[ChargeAbility]")
{
    ChargeAbility ability(aChargeOf(150.0f));
    Decided decided;
    ability.decide(Step, askingToCharge(1.0f), onTheGround(), decided);

    ability.decide(Step, InputIntentions{}, againstAWall(false), decided);
    REQUIRE(decided.charge.active);

    ability.decide(Step, InputIntentions{}, againstAWall(true), decided);
    REQUIRE_FALSE(decided.charge.active);
}

TEST_CASE("A charge into the wall it is already against never starts", "[ChargeAbility]")
{
    ChargeAbility ability(aChargeOf(150.0f));
    Decided decided;

    ability.decide(Step, askingToCharge(1.0f), againstAWall(true), decided);
    REQUIRE_FALSE(decided.charge.active);

    ability.decide(Step, askingToCharge(-1.0f), againstAWall(true), decided);
    REQUIRE(decided.charge.active);
}

TEST_CASE("A charge says so once, and asking again mid-charge changes nothing", "[ChargeAbility]")
{
    ChargeAbility ability(aChargeOf(150.0f));
    Decided decided;
    ability.decide(Step, askingToCharge(1.0f), onTheGround(), decided);
    REQUIRE(decided.charge.emit);

    ability.decide(Step, askingToCharge(-1.0f), onTheGround(), decided);

    REQUIRE_FALSE(decided.charge.emit);
    REQUIRE(decided.charge.velocity.x == 150.0f);
}

TEST_CASE("A knockback cuts a charge short", "[ChargeAbility]")
{
    ChargeAbility ability(aChargeOf(150.0f));
    Decided decided;
    ability.decide(Step, askingToCharge(1.0f), onTheGround(), decided);

    decided.knockback.active = true;
    ability.decide(Step, InputIntentions{}, onTheGround(), decided);

    REQUIRE_FALSE(decided.charge.active);
}

TEST_CASE("A charge with nothing to it is refused", "[ChargeAbility]")
{
    REQUIRE_THROWS_WITH(
        ChargeAbility(aChargeOf(0.0f)), Catch::Matchers::ContainsSubstring("speed"));
    REQUIRE_THROWS_WITH(
        ChargeAbility(aChargeOf(150.0f, 0)), Catch::Matchers::ContainsSubstring("damage"));
}

TEST_CASE("A charge carries the actor, whatever its legs were asked", "[ChargeAbility][Abilities]")
{
    ActorMotionData motion;
    motion.moveAbilityData = MoveAbilityData{60.0f};
    motion.chargeAbilityData = aChargeOf(150.0f);
    Abilities abilities(motion);
    Decided decided;

    abilities.decide(Step, askingToCharge(1.0f), onTheGround(), decided);
    REQUIRE(decided.targetVelocity.x == 150.0f);

    InputIntentions walkingBack;
    walkingBack.direction.x = -1.0f;
    abilities.decide(Step, walkingBack, onTheGround(), decided);
    REQUIRE(decided.targetVelocity.x == 150.0f);

    abilities.decide(Step, walkingBack, againstAWall(true), decided);
    REQUIRE(decided.targetVelocity.x < 0.0f);
}
