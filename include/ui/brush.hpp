#pragma once

#include <string>

struct Brush
{
    enum class Kind
    {
        Tile,
        PlayerStart,
        Npc
    };

    Kind kind = Kind::Tile;
    int tileIndex = 0;
    std::string npcType;

    bool operator==(const Brush &) const = default;
};
