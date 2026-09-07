#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/abilities/knockback_ability_data.hpp"

struct InputIntentions;
struct ActorMotionState;

class KnockbackAbility : public Ability
{
public:
    explicit KnockbackAbility(const KnockbackAbilityData &data);
    void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        ActorMotionState &state) override;

private:
    KnockbackAbilityData data;
};
