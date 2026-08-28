#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/abilities/wall_jump_ability_data.hpp"
#include "actor/abilities/action_buffer.hpp"
#include "actor/abilities/coyote_time.hpp"
#include "actor/abilities/direction_buffer.hpp"

struct InputIntentions;
struct ActorMotionState;

class WallJumpAbility : public Ability
{
public:
    explicit WallJumpAbility(const WallJumpAbilityData &data);
    void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        ActorMotionState &state) override;

private:
    WallJumpAbilityData data;
    ActionBuffer wallJumpBuffer;
    DirectionBuffer wallJumpDirectionBuffer;
    CoyoteTime wallJumpCoyote;

    void startWallJump(ActorMotionState &state, int direction);
};