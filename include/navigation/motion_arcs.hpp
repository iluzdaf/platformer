#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <vector>

struct ActorMotionData;

struct JumpArc
{
    float holdDuration = 0.0f;
    float holdFraction = 1.0f;
    std::vector<glm::vec2> offsets;

    bool operator==(const JumpArc &) const = default;
};

JumpArc simulateJumpArc(const ActorMotionData &motionData, float holdFraction = 1.0f);

std::vector<JumpArc> simulateJumpArcs(const ActorMotionData &motionData);

class TileMap;
struct PhysicsBodyData;

struct JumpAttempt
{
    std::vector<glm::vec2> path;
    bool landed = false;
};

JumpAttempt simulateJumpAgainst(
    const TileMap &tileMap,
    const ActorMotionData &motionData,
    const PhysicsBodyData &physicsBodyData,
    glm::vec2 takeOffFeet,
    float direction,
    float holdFraction);
