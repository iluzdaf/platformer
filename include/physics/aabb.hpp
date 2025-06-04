#pragma once
#include <glm/gtc/matrix_transform.hpp>

struct AABB
{
    glm::vec2 position;
    glm::vec2 size;

    float left() const { return position.x; }
    float right() const { return position.x + size.x; }
    float top() const { return position.y; }
    float bottom() const { return position.y + size.y; }

    bool intersects(const AABB &other) const
    {
        return !(right() <= other.left() ||
                 left() >= other.right() ||
                 bottom() <= other.top() ||
                 top() >= other.bottom());
    }

    glm::vec2 center() const
    {
        return position + size * 0.5f;
    }
};