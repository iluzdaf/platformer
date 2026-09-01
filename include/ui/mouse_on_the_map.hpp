#pragma once

#include <glm/gtc/matrix_transform.hpp>

struct MouseOnTheMap
{
    bool overTheUi = false;
    glm::vec2 worldPosition = glm::vec2(0.0f);
    bool heldDown = false;
    bool justClicked = false;
};
