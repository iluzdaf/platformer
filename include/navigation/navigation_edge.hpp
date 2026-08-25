#pragma once

enum class EdgeType
{
    Walk,
    Jump,
    Fall,
    Climb
};

struct NavigationEdge
{
    int fromId, toId;
    EdgeType type;
};