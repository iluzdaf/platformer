#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/abilities/jump_ability_data.hpp"
#include "actor/abilities/grace_period.hpp"

struct InputIntentions;
struct Observed;
struct Decided;

class JumpAbility : public Ability
{
public:
    explicit JumpAbility(const JumpAbilityData &data);
    void decide(
        float deltaTime,
        const InputIntentions &inputIntentions,
        const Observed &observed,
        Decided &decided) override;

private:
    JumpAbilityData data;
    GracePeriod jumpBuffer;
    GracePeriod coyoteTime;
};