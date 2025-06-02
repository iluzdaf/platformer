#pragma once
class MovementContext;
class PlayerState;

class MovementAbility
{
public:
    virtual ~MovementAbility() = default;
    virtual void fixedUpdate(
        MovementContext &movementContext,
        const PlayerState &playerState,
        float deltaTime) = 0;
    virtual void update(
        MovementContext &movementContext,
        const PlayerState &playerState,
        float deltaTime) = 0;
    virtual void tryJump(
        MovementContext &movementContext,
        const PlayerState &playerState) {}
    virtual void tryDash(
        MovementContext &movementContext,
        const PlayerState &playerState) {}
    virtual void tryMoveLeft(
        MovementContext &movementContext,
        const PlayerState &playerState) {}
    virtual void tryMoveRight(
        MovementContext &movementContext,
        const PlayerState &playerState) {}
    virtual void syncState(PlayerState &playerState) const {}
};