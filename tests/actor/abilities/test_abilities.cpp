#include <string>
#include <string_view>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/abilities.hpp"
#include "actor/abilities/charge_ability_data.hpp"
#include "actor/abilities/dash_ability_data.hpp"
#include "actor/abilities/gravity_ability_data.hpp"
#include "actor/abilities/jump_ability_data.hpp"
#include "actor/abilities/knockback_ability_data.hpp"
#include "actor/abilities/mantle_ability_data.hpp"
#include "actor/abilities/move_ability_data.hpp"
#include "actor/abilities/pounce_ability_data.hpp"
#include "actor/abilities/swing_ability_data.hpp"
#include "actor/abilities/wall_climb_ability_data.hpp"
#include "actor/abilities/wall_hang_ability_data.hpp"
#include "actor/abilities/wall_jump_ability_data.hpp"
#include "actor/abilities/wall_slide_ability_data.hpp"
#include "actor/abilities/ability_states.hpp"
#include "actor/abilities/abilities_data.hpp"
#include "combat/hit.hpp"
#include "actor/observed.hpp"
#include "helpers/abilities.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

namespace
{
    AbilitiesData everyAbility()
    {
        AbilitiesData data;
        data.move = MoveAbilityData{};
        data.jump = JumpAbilityData{};
        data.dash = DashAbilityData{};
        data.wallSlide = WallSlideAbilityData{};
        data.wallJump = WallJumpAbilityData{};
        data.wallHang = WallHangAbilityData{};
        data.wallClimb = WallClimbAbilityData{};
        data.mantle = MantleAbilityData{};
        data.gravity = GravityAbilityData{};
        data.pounce = PounceAbilityData{};
        data.charge = ChargeAbilityData{};
        data.knockback = KnockbackAbilityData{};
        data.swing = SwingAbilityData{};
        return data;
    }

    InputIntentions askingToClimb(InputIntentions intentions = InputIntentions{})
    {
        intentions.climbRequested = true;
        return intentions;
    }

    InputIntentions askingTo(std::string_view attack, float x)
    {
        InputIntentions intentions = pressing(x);
        intentions.attack = std::string(attack);
        return intentions;
    }

    void settleUntilSliding(Abilities &abilities, Observed &observed, AbilityStates &states)
    {
        for (int tick = 0; tick < 100 && !states.wallSlide.active; ++tick)
            settle(abilities, InputIntentions{}, observed, states);
    }
}

TEST_CASE("Walking and jumping at once goes along at the walk and up at the jump", "[Abilities]")
{
    AbilitiesData data = everyAbility();
    Abilities abilities(data);
    AbilityStates states;
    Observed observed = onTheGround();

    glm::vec2 velocity = settle(abilities, pressingJump(1.0f), observed, states);

    REQUIRE(velocity.x == Approx(data.move->moveSpeed));
    REQUIRE(velocity.y == Approx(data.jump->jumpSpeed));
}

TEST_CASE(
    "In the air with nothing else going on, gravity says how fast it falls and the walk how fast "
    "it goes along",
    "[Abilities]")
{
    AbilitiesData data = everyAbility();
    Abilities abilities(data);
    AbilityStates states;
    Observed observed = inTheAir();

    glm::vec2 velocity = settle(abilities, pressing(1.0f), observed, states, 3);

    REQUIRE(velocity.x == Approx(data.move->moveSpeed));
    REQUIRE(velocity.y == Approx(data.gravity->gravity * Step * 3.0f));
}

TEST_CASE("A dash owns the velocity over walking, jumping and gravity", "[Abilities]")
{
    AbilitiesData data = everyAbility();
    Abilities abilities(data);
    AbilityStates states;
    Observed observed = onTheGround();
    settle(abilities, pressingDash(1.0f), observed, states);
    observed.contacts = inTheAir().contacts;

    glm::vec2 velocity = settle(abilities, pressingJump(-1.0f), observed, states);

    REQUIRE(states.jump.active);
    REQUIRE(velocity == glm::vec2(data.dash->dashSpeed, 0.0f));
}

TEST_CASE(
    "Gravity gathers through a dash in the air, and the fall comes when the dash ends",
    "[Abilities]")
{
    AbilitiesData data = everyAbility();
    Abilities abilities(data);
    AbilityStates states;
    Observed observed = inTheAir();
    settle(abilities, pressingDash(1.0f), observed, states);

    int ticks = 1;
    while (states.dash.active && ticks < 1000)
    {
        settle(abilities, InputIntentions{}, observed, states);
        ++ticks;
    }

    REQUIRE_FALSE(states.dash.active);
    REQUIRE(
        observed.velocity.y == Approx(data.gravity->gravity * Step * static_cast<float>(ticks)));
}

TEST_CASE("A knockback owns the velocity over everything, a dash included", "[Abilities]")
{
    Abilities abilities(everyAbility());
    AbilityStates states;
    Observed observed = onTheGround();
    settle(abilities, pressingDash(1.0f), observed, states);
    observed.hits.push_back(Hit{1, glm::vec2(-1.0f, 0.0f), false});

    glm::vec2 velocity = settle(abilities, pressingDash(1.0f), observed, states);

    REQUIRE(states.knockback.active);
    REQUIRE(velocity == states.knockback.velocity);
    REQUIRE(velocity.x < 0.0f);
}

TEST_CASE("A mantle owns the velocity over walking, climbing and gravity", "[Abilities]")
{
    AbilitiesData data = everyAbility();
    Abilities abilities(data);
    AbilityStates states;
    Observed observed = atALedge(WallSide::Right);

    glm::vec2 velocity = settle(abilities, askingToClimb(pressing(-1.0f, -1.0f)), observed, states);

    REQUIRE(states.mantle.active);
    REQUIRE(velocity == glm::vec2(0.0f, -data.mantle->mantleSpeed));
}

TEST_CASE("A pounce owns the velocity over walking, and gravity bends it", "[Abilities]")
{
    AbilitiesData data = everyAbility();
    Abilities abilities(data);
    AbilityStates states;
    Observed observed = onTheGround();
    glm::vec2 leap = data.pounce->leap;

    glm::vec2 velocity = settle(abilities, askingTo(PounceAttack, 1.0f), observed, states);
    REQUIRE(velocity == leap);

    observed.contacts = inTheAir().contacts;
    velocity = settle(abilities, pressing(-1.0f), observed, states);
    REQUIRE(velocity.x == Approx(leap.x));
    REQUIRE(velocity.y == Approx(leap.y + data.gravity->gravity * Step));
}

TEST_CASE("A wall jump owns both ways the actor goes, over walking", "[Abilities]")
{
    AbilitiesData data = everyAbility();
    data.wallJump->wallJumpHorizontalSpeed = 400.0f;
    Abilities abilities(data);
    AbilityStates states;
    Observed observed = onAWall(WallSide::Left);

    glm::vec2 velocity = settle(abilities, holdingJump(1.0f), observed, states);

    REQUIRE(velocity == glm::vec2(400.0f, data.wallJump->wallJumpSpeed));
}

TEST_CASE(
    "Hanging on a wall it would slide down, the climb says how fast it goes, not the slide",
    "[Abilities]")
{
    AbilitiesData data = everyAbility();
    Abilities abilities(data);
    AbilityStates states;
    Observed observed = onAWall(WallSide::Left);
    observed.velocity.y = 100.0f;

    glm::vec2 velocity = settle(abilities, askingToClimb(), observed, states);
    REQUIRE(states.wallSlide.active);
    REQUIRE(velocity.y == 0.0f);

    velocity = settle(abilities, askingToClimb(pressingUp()), observed, states);
    REQUIRE(velocity.y == Approx(-data.wallClimb->climbSpeed));
}

TEST_CASE("Sliding down a wall, the slide says how fast it falls, not gravity", "[Abilities]")
{
    AbilitiesData data = everyAbility();
    Abilities abilities(data);
    AbilityStates states;
    Observed observed = onAWall(WallSide::Left);
    settle(abilities, InputIntentions{}, observed, states);

    glm::vec2 velocity = settle(abilities, InputIntentions{}, observed, states);

    REQUIRE(states.wallSlide.active);
    REQUIRE(velocity.y == Approx(data.wallSlide->slideSpeed));
}

TEST_CASE("A dash and a swing pressed together dash, and do not swing", "[Abilities]")
{
    Abilities abilities(everyAbility());
    AbilityStates states;
    Observed observed = onTheGround();
    InputIntentions both = pressingDash(1.0f);
    both.attack = std::string(SwingAttack);

    settle(abilities, both, observed, states);

    REQUIRE(states.dash.active);
    REQUIRE_FALSE(states.swing.swinging());
}

TEST_CASE(
    "A jump beside a wall comes down it as a slide, and jumping away from it there wall jumps",
    "[Abilities]")
{
    AbilitiesData data = everyAbility();
    Abilities abilities(data);
    AbilityStates states;
    Observed observed = onAWall(WallSide::Left);
    observed.contacts.onGround = true;

    glm::vec2 velocity = settle(abilities, pressingJump(), observed, states);
    REQUIRE(velocity == glm::vec2(0.0f, data.jump->jumpSpeed));

    observed.contacts = onAWall(WallSide::Left).contacts;
    settleUntilSliding(abilities, observed, states);
    REQUIRE(states.wallSlide.active);
    REQUIRE(observed.velocity.y == Approx(data.wallSlide->slideSpeed));

    velocity = settle(abilities, holdingJump(1.0f), observed, states);
    REQUIRE(
        velocity ==
        glm::vec2(data.wallJump->wallJumpHorizontalSpeed, data.wallJump->wallJumpSpeed));
}

TEST_CASE(
    "A dash into a wall stops there, slides down it, and can jump away from it",
    "[Abilities]")
{
    AbilitiesData data = everyAbility();
    Abilities abilities(data);
    AbilityStates states;
    Observed observed = inTheAir();
    settle(abilities, pressingDash(-1.0f), observed, states);
    REQUIRE(states.dash.active);

    observed.contacts = onAWall(WallSide::Left).contacts;
    glm::vec2 velocity = settle(abilities, InputIntentions{}, observed, states);
    REQUIRE_FALSE(states.dash.active);
    REQUIRE(velocity.x == 0.0f);

    settleUntilSliding(abilities, observed, states);
    REQUIRE(states.wallSlide.active);

    velocity = settle(abilities, holdingJump(1.0f), observed, states);
    REQUIRE(states.wallJump.active);
    REQUIRE(velocity.x == Approx(data.wallJump->wallJumpHorizontalSpeed));
}
