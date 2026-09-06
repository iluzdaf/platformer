#include "navigation/navigation_build_report.hpp"
#include "navigation/jump_simulation.hpp"

void NavigationBuildReport::noting(const JumpAttempt &attempt)
{
    stepsSimulated += attempt.steps;
    if (attempt.capped)
        ++attemptsCapped;
}
