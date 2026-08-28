#pragma once
#include <glm/gtc/matrix_transform.hpp>

enum class NodeKind
{
    OnFoot,

    Landing
};

struct NavigationNode
{
    int id = 0;
    glm::vec2 position = glm::vec2(0.0f);
    NodeKind kind = NodeKind::OnFoot;
};
