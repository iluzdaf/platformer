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
        if (other.isEmpty())
            return;

        if (isEmpty())
        {
            position = other.position;
            size = other.size;
            return;
        }

        glm::vec2 newMin = glm::min(position, other.position);
        glm::vec2 thisMax = position + size;
        glm::vec2 otherMax = other.position + other.size;
        glm::vec2 newMax = glm::max(thisMax, otherMax);
        position = newMin;
        size = newMax - newMin;
    }

    bool isEmpty() const
    {
        return glm::all(glm::lessThan(glm::abs(size), glm::vec2(1e-5f)));
    }

    std::size_t hash() const
    {
        glm::ivec2 pos = glm::round(position * 100.0f);
        glm::ivec2 size = glm::round(this->size * 100.0f);
        std::size_t h1 = std::hash<int>()(pos.x) ^ std::hash<int>()(pos.y << 1);
        std::size_t h2 = std::hash<int>()(size.x) ^ std::hash<int>()(size.y << 1);
        return h1 ^ (h2 << 1);
    }
};