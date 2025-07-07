#pragma once
struct MovementContext;
struct PlayerState;

class MovementAbility
{
public:
    virtual ~MovementAbility() = default;
    virtual void fixedUpdate(
        MovementContext &movementContext,
        PlayerState &playerState,
        float deltaTime) = 0;
};