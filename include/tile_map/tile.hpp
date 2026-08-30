#pragma once
#include <optional>
#include <glm/gtc/matrix_transform.hpp>
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_collider_data.hpp"
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
    std::optional<AABB> getAABBAt(glm::vec2 worldPosition) const;

private:
    bool solid = false, deadly = false, portal = false;
    std::optional<TileAnimation> animation;
    std::optional<TilePickupData> pickup;
    TileColliderData collider;
    int tileIndex;
};
