#pragma once
#include "game/player/movement_abilities/movement_ability.hpp"
#include "game/player/movement_abilities/wall_jump_ability_data.hpp"
#include "game/player/movement_abilities/action_buffer.hpp"
#include "game/player/movement_abilities/coyote_time.hpp"
#include "game/player/movement_abilities/direction_buffer.hpp"

struct MovementContext;
struct PlayerState;

class WallJumpAbility : public MovementAbility
{
public:
    explicit WallJumpAbility(WallJumpAbilityData wallJumpAbilityData);
    void fixedUpdate(
        MovementContext &movementContext,
        PlayerState &playerState,
        float deltaTime) override;

private:
    WallJumpAbilityData wallJumpAbilityData;
    ActionBuffer wallJumpBuffer;
    DirectionBuffer wallJumpDirectionBuffer;
    CoyoteTime wallJumpCoyote;

    void startWallJump(
        MovementContext &movementContext,
        PlayerState &playerState,
        int direction);
};