#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/ability_states.hpp"
#include "actor/observed.hpp"
#include "input/input_intentions.hpp"

inline constexpr float Step = 0.01f;

enum class WallSide
{
    Left,
    Right
};

inline Observed onTheGround()
{
    Observed observed;
    observed.contacts.onGround = true;
    return observed;
}

inline Observed inTheAir()
{
    return Observed{};
}

inline Observed onASlipperyWall(WallSide side)
{
    Observed observed = inTheAir();
    observed.contacts.touchingLeftWall = side == WallSide::Left;
    observed.contacts.touchingRightWall = side == WallSide::Right;
    return observed;
}

inline Observed onAWall(WallSide side)
{
    Observed observed = onASlipperyWall(side);
    observed.contacts.grippableLeftWall = side == WallSide::Left;
    observed.contacts.grippableRightWall = side == WallSide::Right;
    observed.contacts.wasLastWallLeft = side == WallSide::Left;
    return observed;
}

inline Observed atALedge(WallSide side)
{
    Observed observed = onAWall(side);
    observed.contacts.ledgeOnLeft = side == WallSide::Left;
    observed.contacts.ledgeOnRight = side == WallSide::Right;
    return observed;
}

inline Observed justOffAWall(WallSide side)
{
    Observed observed = inTheAir();
    observed.contacts.wasLastWallLeft = side == WallSide::Left;
    return observed;
}

inline InputIntentions pressing(float x, float y = 0.0f)
{
    InputIntentions intentions;
    intentions.direction = glm::vec2(x, y);
    return intentions;
}

inline InputIntentions pressingUp()
{
    return pressing(0.0f, -1.0f);
}

inline InputIntentions pressingDown()
{
    return pressing(0.0f, 1.0f);
}

inline AbilityStates hanging()
{
    AbilityStates states;
    states.wallHang.active = true;
    return states;
}

inline InputIntentions pressingJump(float x = 0.0f)
{
    InputIntentions intentions;
    intentions.jumpRequested = true;
    intentions.jumpHeld = true;
    intentions.direction.x = x;
    return intentions;
}

inline InputIntentions holdingJump(float x = 0.0f)
{
    InputIntentions intentions;
    intentions.jumpHeld = true;
    intentions.direction.x = x;
    return intentions;
}

inline InputIntentions pressingDash(float x)
{
    InputIntentions intentions;
    intentions.dashRequested = true;
    intentions.direction.x = x;
    return intentions;
}

inline void tick(
    Ability &ability,
    const InputIntentions &asked,
    const Observed &observed,
    AbilityStates &states,
    int times = 1)
{
    for (int time = 0; time < times; ++time)
        ability.decide(Step, asked, observed, states);
}

template <class Until>
int ticksUntil(
    Ability &ability,
    const InputIntentions &asked,
    const Observed &observed,
    AbilityStates &states,
    Until until)
{
    constexpr int AtMost = 1000;
    for (int ticks = 1; ticks <= AtMost; ++ticks)
    {
        ability.decide(Step, asked, observed, states);
        if (until(states))
            return ticks;
    }

    return -1;
}
