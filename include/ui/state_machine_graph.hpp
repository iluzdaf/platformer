#pragma once

#include <set>
#include <string>

struct StateMachineBehaviorData;

void drawStateMachineGraph(
    const StateMachineBehaviorData &machine,
    const std::set<std::string> &litStates);
