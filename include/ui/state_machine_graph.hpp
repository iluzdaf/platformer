#pragma once

#include <set>
#include <string>
#include "ui/state_machine_shown.hpp"

struct StateMachineBehaviorData;

MachineShown drawStateMachineGraph(
    const StateMachineBehaviorData &machine,
    const std::set<std::string> &litStates,
    MachineShown selected);
