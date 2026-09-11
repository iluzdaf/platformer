#pragma once

#include <set>
#include <string>
#include "ui/state_machine_shown.hpp"

struct GraphShown;

MachineShown drawGraph(
    const char *name,
    const GraphShown &graph,
    const std::set<std::string> &litNodes,
    MachineShown selected);
