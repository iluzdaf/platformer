#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/abilities/wall_jump_ability_data.hpp"
#include "actor/abilities/grace_period.hpp"

struct InputIntentions;
struct Observed;
struct AbilityStates;

class WallJumpAbility : public Ability
{
public:
    explicit WallJumpAbility(const WallJumpAbilityData &data);
    void decide(
        float deltaTime,
        const InputIntentions &inputIntentions,
        const Observed &observed,
        AbilityStates &states) override;

private:
    WallJumpAbilityData data;
    GracePeriod wallJumpBuffer;
    GracePeriod wallJumpCoyote;

    void startWallJump(AbilityStates &states, int direction);
};