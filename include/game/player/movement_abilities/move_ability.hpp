#pragma once
#include "game/player/movement_abilities/movement_ability.hpp"
#include "game/player/movement_abilities/move_ability_data.hpp"

struct MovementContext;
struct PlayerState;

class MoveAbility : public MovementAbility
{
public:
    explicit MoveAbility(MoveAbilityData moveAbilityData);
    void fixedUpdate(
        MovementContext &movementContext,
        PlayerState &playerState,
        float deltaTime) override;

private:
    MoveAbilityData moveAbilityData;
};