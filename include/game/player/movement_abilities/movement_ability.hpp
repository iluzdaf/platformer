#pragma once
class MovementContext;
class PlayerState;

class MovementAbility
{
public:
    virtual ~MovementAbility() = default;
    virtual void fixedUpdate(
        MovementContext &movementContext,
        PlayerState &playerState,
        float deltaTime) = 0;
};