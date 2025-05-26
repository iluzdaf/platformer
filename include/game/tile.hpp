#pragma once

#include "game/tile_kind.hpp"
#include "game/tile_data.hpp"
#include "animations/tile_animation.hpp"
#include <optional>

class Tile
{
public:
    explicit Tile(const TileData &tileData);
    void update(float deltaTime);
    int getCurrentFrame() const;
    bool isSolid() const;
    bool isAnimated() const;
    TileKind getKind() const;
    bool isPickup() const;
    std::optional<int> getPickupReplaceIndex() const;

private:
    TileKind kind;
    std::optional<TileAnimation> animation;
    std::optional<int> pickupReplaceIndex;
};