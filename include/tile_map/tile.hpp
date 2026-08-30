#pragma once
#include <optional>
#include <glm/gtc/matrix_transform.hpp>
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_pickup_data.hpp"
#include "animations/tile_animation.hpp"
#include "physics/aabb.hpp"

class Tile
{
public:
    Tile(int tileIndex, const TileData &tileData);
    void update(float deltaTime);
    int getCurrentFrame() const;
    bool isSolid() const;
    bool isDeadly() const;
    bool isPortal() const;
    bool isPickup() const;
    bool isEmpty() const;
    bool isAnimated() const;
    std::optional<int> getPickupReplaceIndex() const;
    std::optional<int> getPickupScoreDelta() const;
    glm::vec2 getColliderOffset() const;
    glm::vec2 getColliderSize() const;
    AABB getAABBAt(glm::vec2 worldPosition) const;

private:
    bool solid = false, deadly = false, portal = false;
    std::optional<TileAnimation> animation;
    std::optional<TilePickupData> pickup;
    glm::vec2 colliderOffset = glm::vec2(0, 0), colliderSize = glm::vec2(16, 16);
    int tileIndex;
};
