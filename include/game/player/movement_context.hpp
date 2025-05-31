#pragma once
#include <glm/gtc/matrix_transform.hpp>
class MovementAbility;
class PlayerState;

class MovementContext
{
public:
    virtual ~MovementContext() = default;
    virtual glm::vec2 getPosition() const = 0;
    virtual glm::vec2 getVelocity() const = 0;
    virtual void setVelocity(const glm::vec2 &velocity) = 0;
    virtual bool onGround() const = 0;
    virtual void setOnGround(bool isOnGround) = 0;
    virtual bool facingLeft() const = 0;
    virtual void setFacingLeft(bool isFacingLeft) = 0;
    virtual const PlayerState &getPlayerState() const = 0;
    virtual bool touchingLeftWall() const = 0;
    virtual bool touchingRightWall() const = 0;
};