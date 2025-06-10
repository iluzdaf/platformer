#pragma once
#include <signals.hpp>

class DebugControlUi
{
public:
    void draw(bool shouldDrawDebugControl);

    fteng::signal<void()> onPlay,
        onStep,
        onRespawn,
        onToggleZoom,
        onToggleDrawGrid,
        onToggleDrawTileInfo,
        onToggleDrawPlayerAABBs,
        onToggleDrawTileMapAABBs,
        onGameReload;
};