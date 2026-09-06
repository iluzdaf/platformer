#pragma once

#include <optional>

struct TileMapShown
{
    bool showing = false;
    std::optional<bool> beforeArming;

    bool operator==(const TileMapShown &) const = default;
};

inline TileMapShown whileArmed(TileMapShown tileMap, bool armed)
{
    if (armed && !tileMap.beforeArming)
        return TileMapShown{true, tileMap.showing};

    if (!armed && tileMap.beforeArming)
        return TileMapShown{tileMap.showing && *tileMap.beforeArming, std::nullopt};

    return tileMap;
}
