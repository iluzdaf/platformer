#pragma once

#include <set>
#include <string>
#include "ui/state_machine_shown.hpp"

struct ActorAnimationData;

MachineShown drawAnimatorGraph(
    const ActorAnimationData &animations,
    const std::set<std::string> &litClips,
    MachineShown selected);
