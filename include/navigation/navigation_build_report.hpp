#pragma once

struct JumpAttempt;

struct NavigationBuildReport
{
    int stepsSimulated = 0;
    int attemptsCapped = 0;

    void noting(const JumpAttempt &attempt);

    bool operator==(const NavigationBuildReport &) const = default;
};
