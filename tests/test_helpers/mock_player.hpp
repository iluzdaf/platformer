#pragma once

#include <glm/gtc/matrix_transform.hpp>

class MockPlayer {
public:
    glm::vec2 position = {};
    glm::vec2 velocity = {};
    bool onGround = false;

    bool onGroundCalled = false;

    glm::vec2 getVelocity() const { return velocity; }
    void setVelocity(glm::vec2 v) { velocity = v; }
    bool onGroundMethod() const { return onGround; }

    float getJumpSpeed() const { return -500.0f; }
};