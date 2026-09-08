#pragma once

#include <vector>
#include <glm/gtc/matrix_transform.hpp>

class Actor;

enum class MeleePhase
{
    Idle,
    Windup,
    Active,
    Recovery
};

struct MeleeAbilityState
{
    MeleePhase phase = MeleePhase::Idle;
    float timeLeft = 0, direction = 1;
    bool emit = false;
    glm::vec2 reach = glm::vec2(0.0f);
    int damage = 0;
    std::vector<const Actor *> struck;

    bool swinging() const
    {
        return phase != MeleePhase::Idle;
    }

    bool striking() const
    {
        return phase == MeleePhase::Active;
    }
};
