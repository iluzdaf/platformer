#pragma once
#include <optional>
#include "game/tile_map/tile_kind.hpp"
#include "animations/tile_animation.hpp"
class TileData;

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
    bool isSpikes() const;

private:
    TileKind kind;
    std::optional<TileAnimation> animation;
    std::optional<int> pickupReplaceIndex;
};