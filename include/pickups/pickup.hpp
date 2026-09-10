#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "animations/frame_animation.hpp"
#include "assets/sheet_data.hpp"
#include "physics/aabb.hpp"
#include "pickups/pickup_data.hpp"
#include <string>

class Pickup
{
public:
    Pickup(const PickupData &pickupData, glm::vec2 position);
    Pickup(std::string type, const PickupData &pickupData, glm::vec2 feet);

    const std::string &type() const;
    const PickupData &builtFrom() const;
    glm::vec2 getFeet() const;

    void update(float deltaTime);

    const SheetData &getSheet() const;
    int frame() const;
    const glm::vec2 &getPosition() const;
    const glm::vec2 &getSize() const;
    int getScoreDelta() const;
    AABB getAABB() const;

private:
    std::string kind;
    PickupData data;
    glm::vec2 feet = glm::vec2(0.0f);
    SheetData sheet;
    FrameAnimation animation;
    glm::vec2 position, size, colliderSize, colliderOffset;
    int scoreDelta = 0;
};
