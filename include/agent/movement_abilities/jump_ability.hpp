#pragma once

#include "agent/movement_abilities/movement_ability.hpp"
#include "agent/movement_abilities/jump_ability_data.hpp"
#include "agent/movement_abilities/action_buffer.hpp"
#include "agent/movement_abilities/coyote_time.hpp"

struct InputIntentions;
struct AgentState;

class JumpAbility : public MovementAbility
{
public:
    explicit JumpAbility(const JumpAbilityData& data);
    void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        AgentState &state) override;

private:
    JumpAbilityData data;
    ActionBuffer jumpBuffer;
    CoyoteTime coyoteTime;
};