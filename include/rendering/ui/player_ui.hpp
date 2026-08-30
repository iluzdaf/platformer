#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "rendering/ui/saveable.hpp"

struct ActorMotionState;
struct ActorState;
struct EditorCommands;
struct GameData;

class PlayerUi
{
public:
    void draw(
        GameData &gameData,
        const ActorMotionState &playerMotionState,
        const glm::vec2 &playerPosition,
        const ActorState &actorState,
        EditorCommands &commands);

    bool drawsPlayerCollider() const;
    bool drawsPlayerCollisions() const;
    bool drawsContactProbes() const;
    void valuesReplaced();

private:
    Saveable saveable;
    bool drawPlayerCollider = false;
    bool drawPlayerCollisions = false;
    bool drawContactProbes = false;
};
