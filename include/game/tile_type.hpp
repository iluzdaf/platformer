#pragma once

enum class TileKind
{
    Empty,
    Solid,
};

struct TileType
{
    TileKind kind;

    bool isSolid() const
    {
        return kind == TileKind::Solid;
    }
};