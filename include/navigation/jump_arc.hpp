#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <vector>

struct ActorMotionData;

struct JumpArc
{
    float holdDuration = 0.0f;
    std::vector<glm::vec2> offsets;

    bool operator==(const JumpArc &) const = default;
};

JumpArc simulateJumpArc(const ActorMotionData &motionData, float holdFraction = 1.0f);

std::vector<JumpArc> simulateJumpArcs(const ActorMotionData &motionData);
