#pragma once

#include "game/actor/abilities/ability.hpp"
#include "game/actor/abilities/jump_ability_data.hpp"
#include "game/actor/abilities/action_buffer.hpp"
#include "game/actor/abilities/coyote_time.hpp"

struct InputIntentions;
struct ActorMotionState;

class JumpAbility : public Ability
{
public:
    explicit JumpAbility(const JumpAbilityData &data);
    void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        ActorMotionState &state) override;

private:
    JumpAbilityData data;
    ActionBuffer jumpBuffer;
    CoyoteTime coyoteTime;
};