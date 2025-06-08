#pragma once
#include <glm/gtc/matrix_transform.hpp>
class MovementAbility;
class PlayerState;

class MovementContext
{
public:
    virtual ~MovementContext() = default;
    virtual glm::vec2 getVelocity() const = 0;
    virtual void setVelocity(const glm::vec2 &velocity) = 0;
    virtual void setFacingLeft(bool isFacingLeft) = 0;
    virtual void emitWallJump() {}
    virtual void emitDoubleJump() {}
    virtual void emitDash() {}
};