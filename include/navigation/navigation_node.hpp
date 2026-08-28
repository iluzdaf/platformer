#pragma once
#include <glm/gtc/matrix_transform.hpp>

enum class NodeKind
{
    // Where a surface ends, or where a jump up is worth making from.
    OnFoot,

    // Where a fall puts an actor down. Somewhere to arrive, not somewhere to
    // leave from: a drop lands where it lands, which is no place to aim a jump.
    Landing
};

struct NavigationNode
{
    int id = 0;
    glm::vec2 position = glm::vec2(0.0f);
    NodeKind kind = NodeKind::OnFoot;
};
