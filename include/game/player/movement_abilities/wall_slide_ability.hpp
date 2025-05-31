#pragma once
#include "game/player/movement_abilities/movement_ability.hpp"
class WallSlideAbilityData;

class WallSlideAbility : public MovementAbility
{
public:
    WallSlideAbility(const WallSlideAbilityData& wallSlideAbilityData);
    void update(MovementContext &movementContext, float deltaTime) override;
    void syncState(PlayerState &playerState) const override;

private:
    bool wallSliding = false;
    float slideSpeed = 35.0f;
};