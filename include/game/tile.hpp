#pragma once

#include "tile_kind.hpp"
#include "animations/tile_animation.hpp"
#include <optional>

class Tile
{
public:
    explicit Tile(TileKind kind = TileKind::Empty, std::optional<int> pickupReplaceIndex = std::nullopt);
    void update(float deltaTime);
    int getCurrentFrame() const;
    bool isSolid() const;
    void setAnimation(const TileAnimation &anim);
    bool isAnimated() const;
    TileKind getKind() const;
    bool isPickup() const;
    std::optional<int> getPickupReplaceIndex() const;

private:
    TileKind kind;
    std::optional<TileAnimation> animation;
    std::optional<int> pickupReplaceIndex;
};