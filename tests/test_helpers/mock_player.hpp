#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "game/player/movement_context.hpp"

class MockPlayer : public MovementContext
{
public:
    glm::vec2 position = {};
    glm::vec2 velocity = {};
    bool isOnGround = false;
    bool isFacingLeft = false;

    float getJumpSpeed() const { return -500.0f; }

    glm::vec2 getPosition() const override { return position; }
    glm::vec2 getVelocity() const override { return velocity; }
    void setVelocity(const glm::vec2 &v) override { velocity = v; }
    bool onGround() const override { return isOnGround; }
    void setOnGround(bool isOnGround) override { this->isOnGround = isOnGround; };
    bool facingLeft() const override { return isFacingLeft; }
    MovementAbility *getAbilityByType(const std::type_info &type) override { return nullptr; }
    void setFacingLeft(bool isFacingLeft) override { this->isFacingLeft = isFacingLeft; }
};