#pragma once
#include "game/player/movement_abilities/movement_ability.hpp"
class MoveAbilityData;

class MoveAbility : public MovementAbility
{
public:
    explicit MoveAbility(const MoveAbilityData &moveAbilityData);

    void fixedUpdate(MovementContext &movementContext, const PlayerState &playerState, float deltaTime) override;
    void update(MovementContext &movementContext, const PlayerState &playerState, float deltaTime) override;
    void tryMoveLeft(MovementContext &movementContext, const PlayerState &playerState) override;
    void tryMoveRight(MovementContext &movementContext, const PlayerState &playerState) override;
    void syncState(PlayerState &playerState) const override;

    float getMoveSpeed() const;

private:
    float moveSpeed = 160.0f;
    bool moveLeftRequested = false;
    bool moveRightRequested = false;
};