#pragma once

#include "game/player/movement_context.hpp"

class MockPlayer : public MovementContext
{
public:
    glm::vec2 velocity = glm::vec2(0, 0);
    bool facingLeft = false;
    glm::vec2 getVelocity() const override { return velocity; }
    void setVelocity(const glm::vec2 &velocity) override { this->velocity = velocity; }
    void setFacingLeft(bool facingLeft) override { this->facingLeft = facingLeft; }
};