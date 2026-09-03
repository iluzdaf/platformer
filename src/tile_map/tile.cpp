#include <optional>
#include <stdexcept>
#include "tile_map/tile.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_collider_data.hpp"
#include "animations/frame_animation.hpp"
#include "physics/aabb.hpp"

Tile::Tile(const TileData &tileData, glm::vec2 cellSize)
    : solid(tileData.solid), deadly(tileData.deadly), portal(tileData.portal),
      grippable(tileData.grippable),
      collider(tileData.collider.value_or(TileColliderData{glm::vec2(0.0f), cellSize}))
{
    if (tileData.collider)
    {
        if (collider.size.x <= 0.0f || collider.size.y <= 0.0f)
            throw std::runtime_error("A collider of no size is not one anything can touch");

        glm::vec2 far = collider.offset + collider.size;
        if (collider.offset.x < 0.0f || collider.offset.y < 0.0f || far.x > cellSize.x ||
            far.y > cellSize.y)
            throw std::runtime_error(
                "A collider reaching outside its cell is never looked for out there");
    }

    if (solid && (deadly || portal))
        throw std::runtime_error(
            "A solid tile is never touched, so it cannot be deadly or a portal");

    if (deadly && portal)
        throw std::runtime_error("A deadly tile kills on touch, so it cannot also be a portal");

    if (tileData.animationData.has_value())
        animation = FrameAnimation(tileData.animationData.value());
}

void Tile::update(float deltaTime)
{
    if (animation)
        animation->update(deltaTime);
}

std::optional<int> Tile::animatingTo() const
{
    if (!animation)
        return std::nullopt;

    return animation->getCurrentFrame();
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

bool Tile::isEmpty() const
{
    return !solid && !deadly && !portal;
}

std::optional<AABB> Tile::getAABBAt(glm::vec2 worldPosition) const
{
    if (isEmpty())
        return std::nullopt;

    return AABB(worldPosition + collider.offset, collider.size);
}
