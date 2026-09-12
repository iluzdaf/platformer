#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/abilities/swing_ability_data.hpp"
#include "actor/abilities/swing_ability_state.hpp"

struct InputIntentions;
struct Observed;
struct AbilityStates;

class SwingAbility : public Ability
{
public:
    explicit SwingAbility(const SwingAbilityData &data);
    void decide(
        float deltaTime,
        const InputIntentions &inputIntentions,
        const Observed &observed,
        AbilityStates &states) override;

private:
    SwingAbilityData data;
    float lengthOf(SwingPhase phase) const;
    void keepTime(float deltaTime, SwingAbilityState &swing) const;
};
