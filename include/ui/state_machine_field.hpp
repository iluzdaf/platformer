#pragma once

#include "actor/behaviors/state_machine_behavior_data.hpp"
#include <optional>
#include <set>
#include <string>
#include "ui/inspector_edited.hpp"
#include "ui/state_machine_shown.hpp"

inspector::Edited drawStateMachineEditor(
    std::optional<StateMachineBehaviorData> &machine,
    const std::set<std::string> &litStates,
    MachineShown &shown);
