#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <vector>

struct ActorMotionData;

std::vector<glm::vec2> simulateJumpArc(
    const ActorMotionData &motionData,
    float holdFraction = 1.0f);

std::vector<std::vector<glm::vec2>> simulateJumpArcs(const ActorMotionData &motionData);
