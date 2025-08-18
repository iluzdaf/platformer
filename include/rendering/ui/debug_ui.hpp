#pragma once

#include <signals.hpp>

struct AgentState;
struct PlayerState;
class Camera2D;
class ImGuiManager;

class DebugUi
{
public:
    void draw(
        const ImGuiManager &imGuiManager,
        const AgentState &playerAgentState,
        const PlayerState &playerState,
        const Camera2D &camera,
        bool showDebug);

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