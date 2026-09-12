#pragma once

#include <glm/gtc/matrix_transform.hpp>

struct Hit
{
    int damage = 1;
    glm::vec2 direction = glm::vec2(0.0f);
    bool lethal = false;
};

inline Hit lethalHit()
{
    return Hit{0, glm::vec2(0.0f), true};
}
