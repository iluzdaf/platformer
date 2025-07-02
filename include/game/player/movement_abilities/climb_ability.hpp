#pragma once
#include "game/player/movement_abilities/movement_ability.hpp"
#include "game/player/movement_abilities/climb_ability_data.hpp"

struct MovementContext;
struct PlayerState;

class ClimbAbility : public MovementAbility
{
public:
    explicit ClimbAbility(ClimbAbilityData climbAbilityData);
    void fixedUpdate(
        MovementContext &movementContext,
        PlayerState &playerState,
        float deltaTime) override;

private:
    ClimbAbilityData climbAbilityData;
};