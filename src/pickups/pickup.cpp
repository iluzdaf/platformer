#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>
#include "pickups/pickup.hpp"
#include "pickups/pickup_data.hpp"
#include "physics/aabb.hpp"

Pickup::Pickup(const PickupData &pickupData, glm::vec2 position)
    : sheet(pickupData.sheet), animation(pickupData.animationData), position(position),
      size(pickupData.size), colliderSize(pickupData.colliderSize.value_or(pickupData.size)),
      colliderOffset(pickupData.colliderOffset), scoreDelta(pickupData.scoreDelta)
{
    if (colliderSize.x <= 0.0f || colliderSize.y <= 0.0f)
        throw std::runtime_error("A pickup nothing can reach is one nobody can take");
}

void Pickup::update(float deltaTime)
{
    animation.update(deltaTime);
}

const SheetData &Pickup::getSheet() const
{
    return sheet;
}

int Pickup::frame() const
{
    return animation.frame();
}

const glm::vec2 &Pickup::getPosition() const
{
    return position;
}

const glm::vec2 &Pickup::getSize() const
{
    return size;
}

int Pickup::getScoreDelta() const
{
    return scoreDelta;
}

AABB Pickup::getAABB() const
{
    return AABB{position + colliderOffset, colliderSize};
}
