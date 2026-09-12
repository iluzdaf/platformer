#pragma once

struct HealthData
{
    int maximum = 1;
    float invulnerableFor = 0.0f;

    bool operator==(const HealthData &) const = default;
};
