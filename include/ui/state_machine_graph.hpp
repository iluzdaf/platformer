#pragma once

#include "actor/behaviors/state_machine_behavior_data.hpp"
#include <set>
#include <string>
#include "ui/state_machine_shown.hpp"

MachineShown drawStateMachineGraph(
    const StateMachineBehaviorData &machine,
    const std::set<std::string> &litStates,
    MachineShown selected);
