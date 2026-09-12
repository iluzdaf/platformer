#include <optional>
#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/ability_states.hpp"
#include "actor/abilities/swing_ability_state.hpp"
#include "actor/hurting_from.hpp"
#include "combat/hurting.hpp"
#include "physics/aabb.hpp"

namespace
{
    const AABB Body{glm::vec2(100.0f, 50.0f), glm::vec2(10.0f, 20.0f)};

    AbilityStates striking(float direction)
    {
        AbilityStates states;
        states.swing.phase = SwingPhase::Active;
        states.swing.direction = direction;
        states.swing.reach = glm::vec2(12.0f, 6.0f);
        states.swing.damage = 3;
        return states;
    }

    AbilityStates biting(int damage)
    {
        AbilityStates states;
        states.bite.active = true;
        states.bite.damage = damage;
        return states;
    }
}

TEST_CASE("An actor doing nothing that hurts threatens nothing", "[HurtingFrom]")
{
    REQUIRE_FALSE(hurtingFrom(AbilityStates{}, Body));
}

TEST_CASE("A bite threatens the whole body, with no way of its own", "[HurtingFrom]")
{
    std::optional<Hurting> hurting = hurtingFrom(biting(2), Body);

    REQUIRE(hurting.has_value());
    REQUIRE(hurting->box.position == Body.position);
    REQUIRE(hurting->box.size == Body.size);
    REQUIRE(hurting->damage == 2);
    REQUIRE(hurting->direction == 0.0f);
}

TEST_CASE("A charge or a pounce threatens the whole body, the way it goes", "[HurtingFrom]")
{
    AbilityStates charging;
    charging.charge.active = true;
    charging.charge.damage = 2;
    charging.charge.direction = -1.0f;

    AbilityStates pouncing;
    pouncing.pounce.active = true;
    pouncing.pounce.damage = 4;
    pouncing.pounce.direction = 1.0f;

    std::optional<Hurting> charge = hurtingFrom(charging, Body);
    REQUIRE(charge->box.position == Body.position);
    REQUIRE(charge->damage == 2);
    REQUIRE(charge->direction == -1.0f);

    std::optional<Hurting> pounce = hurtingFrom(pouncing, Body);
    REQUIRE(pounce->box.position == Body.position);
    REQUIRE(pounce->damage == 4);
    REQUIRE(pounce->direction == 1.0f);
}

TEST_CASE("A striking swing reaches out from the side of the body it faces", "[HurtingFrom]")
{
    std::optional<Hurting> right = hurtingFrom(striking(1.0f), Body);
    REQUIRE(right->box.left() == Body.right());
    REQUIRE(right->box.size == glm::vec2(12.0f, 6.0f));
    REQUIRE(right->box.center().y == Body.center().y);
    REQUIRE(right->damage == 3);
    REQUIRE(right->direction == 1.0f);

    std::optional<Hurting> left = hurtingFrom(striking(-1.0f), Body);
    REQUIRE(left->box.right() == Body.left());
    REQUIRE(left->direction == -1.0f);
}

TEST_CASE("A swing winding up or recovering threatens nothing", "[HurtingFrom]")
{
    AbilityStates windingUp = striking(1.0f);
    windingUp.swing.phase = SwingPhase::Windup;
    AbilityStates recovering = striking(1.0f);
    recovering.swing.phase = SwingPhase::Recovery;

    REQUIRE_FALSE(hurtingFrom(windingUp, Body));
    REQUIRE_FALSE(hurtingFrom(recovering, Body));
}

TEST_CASE("A swing beats a pounce, a pounce a charge, and a charge a bite", "[HurtingFrom]")
{
    AbilityStates all = striking(1.0f);
    all.pounce.active = true;
    all.pounce.damage = 4;
    all.charge.active = true;
    all.charge.damage = 2;
    all.bite = biting(1).bite;
    REQUIRE(hurtingFrom(all, Body)->damage == 3);

    all.swing.phase = SwingPhase::Idle;
    REQUIRE(hurtingFrom(all, Body)->damage == 4);

    all.pounce.active = false;
    REQUIRE(hurtingFrom(all, Body)->damage == 2);

    all.charge.active = false;
    REQUIRE(hurtingFrom(all, Body)->damage == 1);
}
