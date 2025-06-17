#include <cassert>
#include <stdexcept>
#include "game/tile_map/tile.hpp"

Tile::Tile(int tileIndex, const TileData &tileData)
    : kind(tileData.kind),
      pickupReplaceIndex(tileData.pickupReplaceIndex),
      colliderOffset(tileData.colliderOffset),
      colliderSize(tileData.colliderSize),
      pickupScoreDelta(tileData.pickupScoreDelta),
      tileIndex(tileIndex)
{
    if (kind == TileKind::Pickup && !pickupReplaceIndex.has_value())
        throw std::invalid_argument("Pickup tile must define a pickupReplaceIndex");

    if (tileIndex < 0)
        throw std::invalid_argument("TileIndex must be 0 or more");

    if (tileData.animationData.has_value())
        animation = TileAnimation(tileData.animationData.value());
}

void Tile::update(float deltaTime)
{
    if (animation)
    {
        animation->update(deltaTime);
    }
}

int Tile::getCurrentFrame() const
{
    if (animation)
    {
        return animation->getCurrentFrame();
    }
    return tileIndex;
}

bool Tile::isSolid() const
{
    return kind == TileKind::Solid;
}

bool Tile::isAnimated() const
{
    return animation.has_value();
}

TileKind Tile::getKind() const
{
    return kind;
}

bool Tile::isPickup() const
{
    return kind == TileKind::Pickup;
}

std::optional<int> Tile::getPickupReplaceIndex() const
{
    return pickupReplaceIndex;
}

bool Tile::isSpikes() const
{
    return kind == TileKind::Spikes;
}

glm::vec2 Tile::getColliderOffset() const
{
    return colliderOffset;
}

glm::vec2 Tile::getColliderSize() const
{
    return colliderSize;
}

AABB Tile::getAABBAt(glm::vec2 worldPosition) const
{
    return AABB(worldPosition + colliderOffset, colliderSize);
}

TileData Tile::toTileData() const
{
    TileData data;
    data.kind = kind;
    if (animation.has_value())
        data.animationData = animation.value().toTileAnimationData();
    data.pickupReplaceIndex = pickupReplaceIndex;
    data.colliderOffset = colliderOffset;
    data.colliderSize = colliderSize;
    return data;
}

bool Tile::isPortal() const
{
    return kind == TileKind::Portal;
}

bool Tile::isEmpty() const
{
    return kind == TileKind::Empty;
}

std::optional<int> Tile::getPickupScoreDelta() const
{
    return pickupScoreDelta;
}