#include "game/tile.hpp"

Tile::Tile(TileKind kind)
    : kind(kind)
{
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