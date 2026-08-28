#pragma once

#include <cstddef>
#include <glm/gtc/matrix_transform.hpp>

struct AABB
{
    glm::vec2 position{0, 0};
    glm::vec2 size{0, 0};

    float left() const;
    float right() const;
    float top() const;
    float bottom() const;
    bool intersects(const AABB &other) const;
    glm::vec2 center() const;
    glm::vec2 bottomCenter() const;
    void expandToInclude(const AABB &other);
    bool isEmpty() const;
    std::size_t hash() const;
};
