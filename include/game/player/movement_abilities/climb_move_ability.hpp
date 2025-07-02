#pragma once
#include "game/player/movement_abilities/movement_ability.hpp"
#include "game/player/movement_abilities/climb_move_ability_data.hpp"

struct MovementContext;
struct PlayerState;

class ClimbMoveAbility : public MovementAbility
{
public:
    explicit ClimbMoveAbility(ClimbMoveAbilityData climbMoveAbilityData);
    void fixedUpdate(
        MovementContext &movementContext,
        PlayerState &playerState,
        float deltaTime) override;

private:
    ClimbMoveAbilityData climbMoveAbilityData;
};