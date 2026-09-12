#pragma once

#include <glm/gtc/matrix_transform.hpp>

enum class SwingPhase
{
    Idle,
    Windup,
    Active,
    Recovery
};

struct SwingAbilityState
{
    SwingPhase phase = SwingPhase::Idle;
    float direction = 1;
    float elapsed = 0.0f;
    bool emit = false;
    glm::vec2 reach = glm::vec2(0.0f);
    int damage = 0;

    bool swinging() const
    {
        return phase != SwingPhase::Idle;
    }

    bool striking() const
    {
        return phase == SwingPhase::Active;
    }
};
