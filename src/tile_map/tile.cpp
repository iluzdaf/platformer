#include <optional>
#include <stdexcept>
#include "tile_map/tile.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_collider_data.hpp"
#include "animations/tile_animation.hpp"
#include "physics/aabb.hpp"

Tile::Tile(int tileIndex, const TileData &tileData)
    : solid(tileData.solid), deadly(tileData.deadly), portal(tileData.portal),
      grippable(tileData.grippable), pickup(tileData.pickup),
      collider(tileData.collider.value_or(TileColliderData{})), tileIndex(tileIndex)
{
    if (tileIndex < 0)
        throw std::runtime_error("TileIndex must be 0 or more");

    if (solid && (deadly || pickup || portal))
        throw std::runtime_error(
            "A solid tile is never touched, so it cannot be deadly, a pickup or a portal");

    if (deadly && (pickup || portal))
        throw std::runtime_error(
            "A deadly tile kills on touch, so it cannot also be a pickup or a portal");

    if (tileData.animationData.has_value())
        animation = TileAnimation(tileData.animationData.value());
}

void Tile::update(float deltaTime)
{
    if (animation)
        animation->update(deltaTime);
}

int Tile::getCurrentFrame() const
{
    if (animation)
        return animation->getCurrentFrame();

    return tileIndex;
}

bool Tile::isSolid() const
{
    return solid;
}

bool Tile::isGrippable() const
{
    return solid && grippable;
}

bool Tile::isDeadly() const
{
    return deadly;
}

bool Tile::isPortal() const
{
    return portal;
}

bool Tile::isPickup() const
{
    return pickup.has_value();
}

bool Tile::isEmpty() const
{
    return !solid && !deadly && !portal && !pickup;
}

bool Tile::isAnimated() const
{
    return animation.has_value();
}

std::optional<int> Tile::getPickupReplaceIndex() const
{
    if (!pickup)
        return std::nullopt;

    return pickup->replaceIndex;
}

std::optional<int> Tile::getPickupScoreDelta() const
{
    if (!pickup)
        return std::nullopt;

    return pickup->scoreDelta;
}

std::optional<AABB> Tile::getAABBAt(glm::vec2 worldPosition) const
{
    if (isEmpty())
        return std::nullopt;

    return AABB(worldPosition + collider.offset, collider.size);
}
