#include <algorithm>
#include <stdexcept>
#include <string_view>
#include "actor/abilities/swing_ability.hpp"
#include "actor/abilities/swing_ability_data.hpp"
#include "actor/abilities/swing_ability_state.hpp"
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

SwingAbility::SwingAbility(const SwingAbilityData &data) : data(data)
{
    if (data.reach.x <= 0.0f || data.reach.y <= 0.0f)
        throw std::runtime_error("A swing needs a reach above 0 each way");

    if (data.damage <= 0)
        throw std::runtime_error("A swing needs damage above 0");
}

void SwingAbility::followTheClip(const Observed &observed, SwingAbilityState &swing)
{
    if (observed.animationFinished)
    {
        swing.phase = SwingPhase::Idle;
        return;
    }

    if (swing.phase == SwingPhase::Windup && said(observed, StrikeCue))
        swing.phase = SwingPhase::Active;
    else if (swing.phase == SwingPhase::Active && said(observed, RecoverCue))
        swing.phase = SwingPhase::Recovery;
}

void SwingAbility::decide(
    float,
    const InputIntentions &inputIntentions,
    const Observed &observed,
    Decided &decided)
{
    SwingAbilityState &swing = decided.swing;
    swing.emit = false;

    if (decided.knockback.active)
    {
        swing.phase = SwingPhase::Idle;
        return;
    }

    if (swing.phase != SwingPhase::Idle)
    {
        followTheClip(observed, swing);
        return;
    }

    if (inputIntentions.attack != SwingAttack || decided.dash.active)
        return;

    swing.phase = SwingPhase::Windup;
    swing.emit = true;
    if (inputIntentions.direction.x != 0.0f)
        swing.direction = inputIntentions.direction.x < 0.0f ? -1.0f : 1.0f;
    else
        swing.direction = observed.facingLeft ? -1.0f : 1.0f;
    swing.reach = data.reach;
    swing.damage = data.damage;
    swing.struck.clear();
}
