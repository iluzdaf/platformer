#pragma once
class MovementContext;
class PlayerState;

class MovementAbility
{
public:
    virtual ~MovementAbility() = default;
    virtual void update(MovementContext &movementContext, float deltaTime) = 0;
    virtual void tryJump(MovementContext &movementContext) {}
    virtual void tryDash(MovementContext &movementContext) {}
    virtual void tryMoveLeft(MovementContext &movementContext) {}
    virtual void tryMoveRight(MovementContext &movementContext) {}
    virtual void syncState(PlayerState &playerState) const {}
};