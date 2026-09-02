#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "animations/frame_animation.hpp"
#include "assets/sheet.hpp"

struct PickupData;

class Pickup
{
public:
    Pickup(const PickupData &pickupData, glm::vec2 position);

    void update(float deltaTime);

    const Sheet &getSheet() const;
    int getCurrentFrame() const;
    const glm::vec2 &getPosition() const;
    const glm::vec2 &getSize() const;

private:
    Sheet sheet;
    FrameAnimation animation;
    glm::vec2 position, size;
};
