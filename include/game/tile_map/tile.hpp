#pragma once
#include <optional>
#include <glm/gtc/matrix_transform.hpp>
#include "game/tile_map/tile_kind.hpp"
#include "game/tile_map/tile_data.hpp"
#include "animations/tile_animation.hpp"
#include "physics/aabb.hpp"

class Tile
{
public:
    Tile(int tileIndex, const TileData &tileData);
    void update(float deltaTime);
    int getCurrentFrame() const;
    bool isSolid() const;
    bool isAnimated() const;
    TileKind getKind() const;
    bool isPickup() const;
    std::optional<int> getPickupReplaceIndex() const;
    bool isSpikes() const;
    glm::vec2 getColliderOffset() const;
    glm::vec2 getColliderSize() const;
    AABB getAABBAt(glm::vec2 worldPosition) const;
    TileData toTileData() const;
    bool isPortal() const;
    bool isEmpty() const;
    std::optional<int> getPickupScoreDelta() const;

private:
    TileKind kind = TileKind::Empty;
    std::optional<TileAnimation> animation;
    std::optional<int> pickupReplaceIndex, pickupScoreDelta;
    glm::vec2 colliderOffset = glm::vec2(0, 0),
              colliderSize = glm::vec2(16, 16);
    int tileIndex;
};