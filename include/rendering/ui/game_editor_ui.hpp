#pragma once

#include <signals.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "rendering/ui/editor_section.hpp"

struct ActorMotionState;
struct ActorState;
struct GameData;
class Camera2D;
class ImGuiManager;

class GameEditorUi
{
public:
    void draw(
        EditorSection section,
        GameData &gameData,
        const ActorMotionState &playerMotionState,
        const glm::vec2 &playerPosition,
        const ActorState &actorState,
        const Camera2D &camera);

    bool drawsPlayerAABBs() const;

    fteng::signal<void()> onPlay, onStep, onToggleZoom, onRespawn;

private:
    bool drawPlayerAABBs = false;
};