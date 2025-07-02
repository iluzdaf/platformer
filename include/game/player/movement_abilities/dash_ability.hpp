#pragma once
#include "game/player/movement_abilities/movement_ability.hpp"
#include "game/player/movement_abilities/dash_ability_data.hpp"

struct MovementContext;
struct PlayerState;

class DashAbility : public MovementAbility
{
public:
    explicit DashAbility(DashAbilityData dashAbilityData);
    void fixedUpdate(
        MovementContext &movementContext,
        PlayerState &playerState,
        float deltaTime) override;

private:
    DashAbilityData dashAbilityData;
};