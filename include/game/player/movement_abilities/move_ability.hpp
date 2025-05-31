#pragma once
#include "game/player/movement_abilities/movement_ability.hpp"
class MoveAbilityData;

class MoveAbility : public MovementAbility
{
public:
    explicit MoveAbility(const MoveAbilityData &moveAbilityData);
    void update(MovementContext &movementContext, float deltaTime) override;
    void tryMoveLeft(MovementContext &movementContext) override;
    void tryMoveRight(MovementContext &movementContext) override;
    float getMoveSpeed() const;
    void syncState(PlayerState &playerState) const override;

private:
    float moveSpeed = 160.0f;
    bool moveLeftRequested = false;
    bool moveRightRequested = false;
};