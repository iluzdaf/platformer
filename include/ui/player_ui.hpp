#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "ui/fading_aabbs.hpp"
#include "ui/saveable.hpp"

struct ActorMotionState;
struct ActorState;
struct EditorCommands;
struct GameData;
class Camera2D;
class ImGuiManager;
class Player;

class PlayerUi
{
public:
    void draw(
        GameData &gameData,
        const ActorMotionState &playerMotionState,
        const glm::vec2 &playerPosition,
        const ActorState &actorState,
        EditorCommands &commands);

    void update(float deltaTime);
    void drawOverlay(
        const ImGuiManager &imGuiManager,
        const Camera2D &camera,
        const Player &player);
    bool hasUnsavedChanges(const GameData &gameData) const;
    void valuesReplaced();

private:
    Saveable saveable;
    FadingAABBs fadingAABBs;
    bool drawPlayerCollider = false;
    bool drawPlayerCollisions = false;
    bool drawContactProbes = false;
};
