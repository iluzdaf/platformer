#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "animations/frame_animation.hpp"
#include "assets/sheet_data.hpp"
#include "physics/aabb.hpp"
#include "pickups/pickup_data.hpp"
#include "pickups/pickup_spawn_data.hpp"

class Pickup
{
public:
    Pickup(const PickupSpawnData &spawn, const PickupData &pickupData);

    const PickupSpawnData &getSpawn() const;
    const PickupData &builtFrom() const;

    void update(float deltaTime);

    void taken();
    bool stillThere() const;

    const SheetData &getSheet() const;
    int frame() const;
    const glm::vec2 &getPosition() const;
    const glm::vec2 &getSize() const;
    int getScoreDelta() const;
    AABB getAABB() const;

private:
    PickupSpawnData spawn;
    PickupData data;
    SheetData sheet;
    FrameAnimation animation;
    glm::vec2 position, size, colliderSize, colliderOffset;
    int scoreDelta = 0;
    bool waiting = true;
};
