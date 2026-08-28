#pragma once

struct DashAbilityData
{
    float dashSpeed = 500.0f;
    float dashDuration = 0.2f;

    float airborneFraction = 1.0f;

    bool operator==(const DashAbilityData &) const = default;
};
