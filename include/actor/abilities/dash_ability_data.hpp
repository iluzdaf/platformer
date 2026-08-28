#pragma once

struct DashAbilityData
{
    float dashSpeed = 500.0f;
    float dashDuration = 0.2f;

    // How much of that duration a dash begun in the air gets. A dash ignores
    // gravity, so in mid air it hangs the actor up and its reach adds to the
    // jump's rather than replacing part of it. Giving a standing dash more than
    // an airborne one is what keeps a jump and a dash together from being worth
    // the sum of both.
    float airborneFraction = 1.0f;

    bool operator==(const DashAbilityData &) const = default;
};
