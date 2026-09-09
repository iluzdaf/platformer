#include <algorithm>
#include <stdexcept>
#include <string_view>
#include "actor/abilities/melee_ability.hpp"
#include "actor/abilities/melee_ability_data.hpp"
#include "actor/abilities/melee_ability_state.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "input/input_intentions.hpp"

namespace
{
    bool said(const Observed &observed, std::string_view cue)
    {
        return std::ranges::find(observed.cues, cue) != observed.cues.end();
    }
}

MeleeAbility::MeleeAbility(const MeleeAbilityData &data) : data(data)
{
    if (data.reach.x <= 0.0f || data.reach.y <= 0.0f)
        throw std::runtime_error("A swing needs a reach above 0 each way");

    if (data.damage <= 0)
        throw std::runtime_error("A swing needs damage above 0");
}

void MeleeAbility::followTheClip(const Observed &observed, MeleeAbilityState &melee)
{
    if (observed.animationFinished)
    {
        melee.phase = MeleePhase::Idle;
        return;
    }

    if (melee.phase == MeleePhase::Windup && said(observed, StrikeCue))
        melee.phase = MeleePhase::Active;
    else if (melee.phase == MeleePhase::Active && said(observed, RecoverCue))
        melee.phase = MeleePhase::Recovery;
}

void MeleeAbility::decide(
    float,
    const InputIntentions &inputIntentions,
    const Observed &observed,
    Decided &decided)
{
    MeleeAbilityState &melee = decided.melee;
    melee.emit = false;

    if (decided.knockback.active)
    {
        melee.phase = MeleePhase::Idle;
        return;
    }

    if (melee.phase != MeleePhase::Idle)
    {
        followTheClip(observed, melee);
        return;
    }

    if (!inputIntentions.attackRequested || decided.dash.active)
        return;

    melee.phase = MeleePhase::Windup;
    melee.emit = true;
    if (inputIntentions.direction.x != 0.0f)
        melee.direction = inputIntentions.direction.x < 0.0f ? -1.0f : 1.0f;
    else
        melee.direction = observed.facingLeft ? -1.0f : 1.0f;
    melee.reach = data.reach;
    melee.damage = data.damage;
    melee.struck.clear();
}
