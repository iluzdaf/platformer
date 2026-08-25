#pragma once

#include "game/actor/abilities/ability.hpp"
#include "game/actor/abilities/wall_jump_ability_data.hpp"
#include "game/actor/abilities/action_buffer.hpp"
#include "game/actor/abilities/coyote_time.hpp"
#include "game/actor/abilities/direction_buffer.hpp"

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

    void startWallJump(
        ActorMotionState &state,
        int direction);
};