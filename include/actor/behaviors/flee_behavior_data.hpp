#pragma once

struct FleeBehaviorData
{
    float arrivalThreshold = 2.0f;
    float breakPastWithin = 8.0f;

    bool operator==(const FleeBehaviorData &) const = default;
};
