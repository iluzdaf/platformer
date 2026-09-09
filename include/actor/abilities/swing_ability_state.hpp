#pragma once

#include <vector>
#include <glm/gtc/matrix_transform.hpp>

class Actor;

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
    bool emit = false;
    glm::vec2 reach = glm::vec2(0.0f);
    int damage = 0;
    std::vector<const Actor *> struck;

    bool swinging() const
    {
        return phase != SwingPhase::Idle;
    }

    bool striking() const
    {
        return phase == SwingPhase::Active;
    }
};
