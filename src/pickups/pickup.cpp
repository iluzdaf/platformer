#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>
#include "pickups/pickup.hpp"
#include "pickups/pickup_data.hpp"
#include "pickups/pickup_spawn_data.hpp"
#include "physics/aabb.hpp"

Pickup::Pickup(const PickupSpawnData &spawn, const PickupData &pickupData)
    : spawn(spawn), data(pickupData), sheet(pickupData.sheet), animation(pickupData.animationData),
      position(spawn.feet - glm::vec2(drawnSizeOf(pickupData).x * 0.5f, drawnSizeOf(pickupData).y)),
      size(drawnSizeOf(pickupData)),
      colliderSize(pickupData.colliderSize.value_or(drawnSizeOf(pickupData))),
      colliderOffset(pickupData.colliderOffset), scoreDelta(pickupData.scoreDelta)
{
    if (size.x <= 0.0f || size.y <= 0.0f)
        throw std::runtime_error("A pickup drawn as nothing is one nobody can see");

    if (colliderSize.x <= 0.0f || colliderSize.y <= 0.0f)
        throw std::runtime_error("A pickup nothing can reach is one nobody can take");
}

const PickupSpawnData &Pickup::getSpawn() const
{
    return spawn;
}

const PickupData &Pickup::builtFrom() const
{
    return data;
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

void Pickup::taken()
{
    waiting = false;
}

bool Pickup::stillThere() const
{
    return waiting;
}
