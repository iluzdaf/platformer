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
    return !(
        right() <= other.left() || left() >= other.right() || bottom() <= other.top() ||
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
