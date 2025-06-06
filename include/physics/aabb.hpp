#pragma once
#include <glm/gtc/matrix_transform.hpp>

struct AABB
{
    glm::vec2 position{0, 0};
    glm::vec2 size{0, 0};

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

    void expandToInclude(const AABB &other)
    {
        glm::vec2 newMin = glm::min(position, other.position);
        glm::vec2 thisMax = position + size;
        glm::vec2 otherMax = other.position + other.size;
        glm::vec2 newMax = glm::max(thisMax, otherMax);
        position = newMin;
        size = newMax - newMin;
    }
};