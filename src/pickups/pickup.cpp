#include <glm/gtc/matrix_transform.hpp>
#include "pickups/pickup.hpp"
#include "pickups/pickup_data.hpp"
#include "physics/aabb.hpp"

Pickup::Pickup(const PickupData &pickupData, glm::vec2 position)
    : sheet(pickupData.sheet), animation(pickupData.animationData), position(position),
      size(pickupData.size), scoreDelta(pickupData.scoreDelta)
{
}

void Pickup::update(float deltaTime)
{
    animation.update(deltaTime);
}

const Sheet &Pickup::getSheet() const
{
    return sheet;
}

int Pickup::getCurrentFrame() const
{
    return animation.getCurrentFrame();
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
    return AABB{position, size};
}
