#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "navigation/jump_arc.hpp"

struct ActorMotionData;
struct PhysicsBodyData;
class TileMap;

struct JumpAttempt
{
    std::vector<glm::vec2> path;
    bool landed = false;
};

JumpArc simulateJumpArc(const ActorMotionData &motionData, float holdFraction = 1.0f);

std::vector<JumpArc> simulateJumpArcs(const ActorMotionData &motionData);

JumpAttempt simulateJumpAgainst(
    const TileMap &tileMap,
    const ActorMotionData &motionData,
    const PhysicsBodyData &physicsBodyData,
    glm::vec2 takeOffFeet,
    float direction,
    float holdFraction);
