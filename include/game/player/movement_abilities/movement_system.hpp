#pragma once
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <signals.hpp>
#include "game/player/movement_abilities/movement_ability.hpp"
#include "input/input_intentions.hpp"

struct PlayerState;
struct MovementContext;

class MovementSystem
{
public:
    void fixedUpdate(
        PlayerState &playerState,
        InputIntentions inputIntentions,
        float deltaTime);
    void addAbility(std::unique_ptr<MovementAbility> ability);
    void clear();

    fteng::signal<void()>
        onWallJump,
        onDash,
        onWallSliding;

private:
    std::vector<std::unique_ptr<MovementAbility>> abilities;

    glm::vec2 computeTargetVelocity(const MovementContext &movementContext, const PlayerState &playerState);
    void emitSignals(const MovementContext &movementContext);
};