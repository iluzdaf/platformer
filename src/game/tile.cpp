#include "game/tile.hpp"
#include <cassert>

Tile::Tile(TileKind kind, std::optional<int> pickupReplaceIndex)
    : kind(kind), pickupReplaceIndex(pickupReplaceIndex)
{
    assert((kind != TileKind::Pickup || pickupReplaceIndex.has_value()) && "Pickup tile must define a pickupReplaceIndex");
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
    return -1;
}

bool Tile::isSolid() const
{
    return kind == TileKind::Solid;
}

void Tile::setAnimation(const TileAnimation &anim)
{
    animation = anim;
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