#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "animations/frame_animation.hpp"
#include "assets/sheet_data.hpp"
#include "physics/aabb.hpp"

struct PickupData;

class Pickup
{
public:
    Pickup(const PickupData &pickupData, glm::vec2 position);

    void update(float deltaTime);

    const SheetData &getSheet() const;
    int getCurrentFrame() const;
    const glm::vec2 &getPosition() const;
    const glm::vec2 &getSize() const;
    int getScoreDelta() const;
    AABB getAABB() const;

private:
    SheetData sheet;
    FrameAnimation animation;
    glm::vec2 position, size;
    int scoreDelta = 0;
};
