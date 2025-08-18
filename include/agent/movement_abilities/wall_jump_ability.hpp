#pragma once

#include "agent/movement_abilities/movement_ability.hpp"
#include "agent/movement_abilities/wall_jump_ability_data.hpp"
#include "agent/movement_abilities/action_buffer.hpp"
#include "agent/movement_abilities/coyote_time.hpp"
#include "agent/movement_abilities/direction_buffer.hpp"

struct InputIntentions;
struct AgentState;

class WallJumpAbility : public MovementAbility
{
public:
    explicit WallJumpAbility(const WallJumpAbilityData &data);
    void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        AgentState &state) override;

private:
    WallJumpAbilityData data;
    ActionBuffer wallJumpBuffer;
    DirectionBuffer wallJumpDirectionBuffer;
    CoyoteTime wallJumpCoyote;

    void startWallJump(
        AgentState &state,
        int direction);
};