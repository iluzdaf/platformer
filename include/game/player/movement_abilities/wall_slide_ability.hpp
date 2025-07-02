#pragma once
#include "game/player/movement_abilities/movement_ability.hpp"
#include "game/player/movement_abilities/wall_slide_ability_data.hpp"

struct MovementContext;
struct PlayerState;

class WallSlideAbility : public MovementAbility
{
public:
    explicit WallSlideAbility(WallSlideAbilityData wallSlideAbilityData);
    void fixedUpdate(
        MovementContext &movementContext,
        PlayerState &playerState,
        float deltaTime) override;

private:
    WallSlideAbilityData wallSlideAbilityData;
};