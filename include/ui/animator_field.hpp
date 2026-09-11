#pragma once

#include <set>
#include <string>
#include "ui/state_machine_shown.hpp"

struct AnimatorData;

MachineShown drawAnimatorGraph(
    const AnimatorData &animations,
    const std::set<std::string> &litClips,
    MachineShown selected);
