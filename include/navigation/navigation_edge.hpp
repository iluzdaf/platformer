#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "navigation/input_program.hpp"

enum class EdgeType
{
    Walk,
    Jump,
    Fall,
    Climb
};

struct NavigationEdge
{
    int fromId, toId;
    EdgeType type;
    std::vector<glm::vec2> path;
    InputProgram inputs;
    float wallDirection = 0.0f;
};
