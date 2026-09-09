#pragma once

struct ChaseBehaviorData
{
    float arrivalThreshold = 2.0f;
    float standoff = 0.0f;

    bool operator==(const ChaseBehaviorData &) const = default;
};
