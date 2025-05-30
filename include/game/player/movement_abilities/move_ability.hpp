#pragma once
#include "game/player/movement_abilities/movement_ability.hpp"

class MoveAbility : public MovementAbility
{
public:
    MoveAbility(float moveSpeed);
    void update(MovementContext &context, float deltaTime) override;
    void tryMoveLeft(MovementContext &context) override;
    void tryMoveRight(MovementContext &context) override;
    float getMoveSpeed() const;

private:
    float moveSpeed = 160.0f;
    bool moveLeftRequested = false;
    bool moveRightRequested = false;
};