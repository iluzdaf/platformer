#pragma once

#include <signals.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct ActorMotionState;
struct ActorState;
class Camera2D;
class ImGuiManager;

class GameEditorUi
{
public:
    void draw(
        const ImGuiManager &imGuiManager,
        const ActorMotionState &playerMotionState,
        const glm::vec2 &playerPosition,
        const ActorState &actorState,
        const Camera2D &camera,
        bool showEditors);

    bool drawsPlayerAABBs() const;

    fteng::signal<void()> onPlay,
        onStep,
        onToggleZoom;

private:
    bool drawPlayerAABBs = false;
};