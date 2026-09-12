#include <stdexcept>
#include "actor/abilities/swing_ability.hpp"
#include "actor/abilities/swing_ability_data.hpp"
#include "actor/abilities/swing_ability_state.hpp"
#include "actor/ability_states.hpp"
#include "actor/observed.hpp"
#include "input/input_intentions.hpp"

namespace
{
    SwingPhase after(SwingPhase phase)
    {
        switch (phase)
        {
        case SwingPhase::Windup:
            return SwingPhase::Active;
        case SwingPhase::Active:
            return SwingPhase::Recovery;
        case SwingPhase::Recovery:
        case SwingPhase::Idle:
            break;
        }

        return SwingPhase::Idle;
    }
}

SwingAbility::SwingAbility(const SwingAbilityData &data) : data(data)
{
    if (data.reach.x <= 0.0f || data.reach.y <= 0.0f)
        throw std::runtime_error("A swing needs a reach above 0 each way");

    if (data.damage <= 0)
        throw std::runtime_error("A swing needs damage above 0");

    if (data.strikeDuration <= 0.0f)
        throw std::runtime_error("A swing needs a strike that lasts above 0");

    if (data.windupDuration < 0.0f || data.recoveryDuration < 0.0f)
        throw std::runtime_error("A swing cannot wind up or recover for less than no time");
}

float SwingAbility::lengthOf(SwingPhase phase) const
{
    switch (phase)
    {
    case SwingPhase::Windup:
        return data.windupDuration;
    case SwingPhase::Active:
        return data.strikeDuration;
    case SwingPhase::Recovery:
        return data.recoveryDuration;
    case SwingPhase::Idle:
        break;
    }

    return 0.0f;
}

void SwingAbility::keepTime(float deltaTime, SwingAbilityState &swing) const
{
    swing.elapsed += deltaTime;
    while (swing.phase != SwingPhase::Idle && swing.elapsed >= lengthOf(swing.phase))
    {
        swing.elapsed -= lengthOf(swing.phase);
        swing.phase = after(swing.phase);
    }
}

void SwingAbility::decide(
    float deltaTime,
    const InputIntentions &inputIntentions,
    const Observed &observed,
    AbilityStates &states)
{
    SwingAbilityState &swing = states.swing;
    swing.emit = false;

    if (states.knockback.active)
    {
        swing.phase = SwingPhase::Idle;
        return;
    }

    if (swing.phase != SwingPhase::Idle)
    {
        keepTime(deltaTime, swing);
        return;
    }

    if (inputIntentions.attack != SwingAttack || states.dash.active)
        return;

    swing.phase = SwingPhase::Windup;
    swing.elapsed = 0.0f;
    swing.emit = true;
    if (inputIntentions.direction.x != 0.0f)
        swing.direction = inputIntentions.direction.x < 0.0f ? -1.0f : 1.0f;
    else
        swing.direction = observed.facingLeft ? -1.0f : 1.0f;
    swing.reach = data.reach;
    swing.damage = data.damage;
}
