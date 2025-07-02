#pragma once
#include "game/player/movement_abilities/movement_ability.hpp"
#include "game/player/movement_abilities/jump_ability_data.hpp"
#include "game/player/movement_abilities/action_buffer.hpp"
#include "game/player/movement_abilities/coyote_time.hpp"

struct MovementContext;
struct PlayerState;

class JumpAbility : public MovementAbility
{
public:
    explicit JumpAbility(JumpAbilityData jumpAbilityData);
    void fixedUpdate(
        MovementContext &movementContext,
        PlayerState &playerState,
        float deltaTime) override;

private:
    JumpAbilityData jumpAbilityData;
    ActionBuffer jumpBuffer;
    CoyoteTime coyoteTime;
};