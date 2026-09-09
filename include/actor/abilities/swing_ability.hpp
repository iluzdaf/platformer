#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/abilities/swing_ability_data.hpp"

struct InputIntentions;
struct Observed;
struct Decided;
struct SwingAbilityState;

class SwingAbility : public Ability
{
public:
    explicit SwingAbility(const SwingAbilityData &data);
    void decide(
        float deltaTime,
        const InputIntentions &inputIntentions,
        const Observed &observed,
        Decided &decided) override;

private:
    SwingAbilityData data;
    void followTheClip(const Observed &observed, SwingAbilityState &swing);
};
