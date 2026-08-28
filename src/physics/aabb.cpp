#include <functional>
#include "physics/aabb.hpp"

float AABB::left() const
{
    return position.x;
}

float AABB::right() const
{
    return position.x + size.x;
}

float AABB::top() const
{
    return position.y;
}

float AABB::bottom() const
{
    return position.y + size.y;
}

bool AABB::intersects(const AABB &other) const
{
    return !(right() <= other.left() ||
             left() >= other.right() ||
             bottom() <= other.top() ||
             top() >= other.bottom());
}

glm::vec2 AABB::center() const
{
    return position + size * 0.5f;
}

glm::vec2 AABB::bottomCenter() const
{
    return glm::vec2(position.x + size.x * 0.5f, bottom());
}

void AABB::expandToInclude(const AABB &other)
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

bool AABB::isEmpty() const
{
    return glm::all(glm::lessThan(glm::abs(size), glm::vec2(1e-5f)));
}

std::size_t AABB::hash() const
{
    glm::ivec2 pos = glm::round(position * 100.0f);
    glm::ivec2 roundedSize = glm::round(this->size * 100.0f);
    std::size_t h1 = std::hash<int>()(pos.x) ^ std::hash<int>()(pos.y << 1);
    std::size_t h2 = std::hash<int>()(roundedSize.x) ^ std::hash<int>()(roundedSize.y << 1);
    return h1 ^ (h2 << 1);
}
