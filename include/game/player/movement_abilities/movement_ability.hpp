#pragma once
class MovementContext;

class MovementAbility
{
public:
    virtual ~MovementAbility() = default;
    virtual void update(MovementContext &context, float deltaTime) = 0;
    virtual void tryJump(MovementContext &context) {}
};