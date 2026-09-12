#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "navigation/jump_arc.hpp"

struct AbilitiesData;
struct PhysicsBodyData;
class TileMap;

struct JumpAttempt
{
    std::vector<glm::vec2> path;
    bool landed = false;
    int steps = 0;
    bool capped = false;
};

JumpArc simulateJumpArc(const AbilitiesData &abilitiesData, float holdFraction = 1.0f);

std::vector<JumpArc> simulateJumpArcs(const AbilitiesData &abilitiesData);

JumpAttempt simulateJumpAgainst(
    const TileMap &tileMap,
    const AbilitiesData &abilitiesData,
    const PhysicsBodyData &physicsBodyData,
    glm::vec2 takeOffFeet,
    float direction,
    float holdFraction);
