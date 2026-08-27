#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <vector>

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
    float holdDuration = 0.0f;
};
