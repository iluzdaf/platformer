#pragma once

#include <signals.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct ActorMotionState;
struct DebugData;
struct ActorState;
class Camera2D;
class ImGuiManager;

class DebugUi
{
public:
    void draw(
        const ImGuiManager &imGuiManager,
        const ActorMotionState &playerMotionState,
        const glm::vec2 &playerPosition,
        const ActorState &actorState,
        const Camera2D &camera,
        DebugData &debug);

    fteng::signal<void()> onPlay,
        onStep,
        onToggleZoom;
};