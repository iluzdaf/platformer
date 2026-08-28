#pragma once

struct PatrolBehaviorData
{
    float arrivalThreshold = 2.0f;

    bool operator==(const PatrolBehaviorData &) const = default;
};
