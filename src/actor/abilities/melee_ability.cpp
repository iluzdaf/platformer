#include <stdexcept>
#include "actor/abilities/melee_ability.hpp"
#include "actor/abilities/melee_ability_data.hpp"
#include "actor/abilities/melee_ability_state.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "input/input_intentions.hpp"

MeleeAbility::MeleeAbility(const MeleeAbilityData &data) : data(data)
{
    if (data.windup < 0.0f || data.recovery < 0.0f)
        throw std::runtime_error("A swing's windup and recovery are 0 or longer");

    if (data.active <= 0.0f)
        throw std::runtime_error("A swing needs time above 0 in which it strikes");

    if (data.reach.x <= 0.0f || data.reach.y <= 0.0f)
        throw std::runtime_error("A swing needs a reach above 0 each way");

    if (data.damage <= 0)
        throw std::runtime_error("A swing needs damage above 0");
}

void MeleeAbility::decide(
    float deltaTime,
    const InputIntentions &inputIntentions,
    const Observed &observed,
    Decided &decided)
{
    MeleeAbilityState &melee = decided.melee;
    melee.emit = false;

    if (decided.knockback.active)
    {
        melee.phase = MeleePhase::Idle;
        melee.timeLeft = 0.0f;
        return;
    }

    if (melee.phase == MeleePhase::Idle)
    {
        if (!inputIntentions.attackRequested || decided.dash.active)
            return;

        melee.phase = MeleePhase::Windup;
        melee.timeLeft = data.windup;
        melee.emit = true;
        if (inputIntentions.direction.x != 0.0f)
            melee.direction = inputIntentions.direction.x < 0.0f ? -1.0f : 1.0f;
        else
            melee.direction = observed.facingLeft ? -1.0f : 1.0f;
        melee.reach = data.reach;
        melee.damage = data.damage;
        melee.struck.clear();
        return;
    }

    melee.timeLeft -= deltaTime;
    if (melee.timeLeft > 0.0f)
        return;

    if (melee.phase == MeleePhase::Windup)
    {
        melee.phase = MeleePhase::Active;
        melee.timeLeft += data.active;
    }
    else if (melee.phase == MeleePhase::Active)
    {
        melee.phase = MeleePhase::Recovery;
        melee.timeLeft += data.recovery;
    }
    else
    {
        melee.phase = MeleePhase::Idle;
        melee.timeLeft = 0.0f;
    }
}
