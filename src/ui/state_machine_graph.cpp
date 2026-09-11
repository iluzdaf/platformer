#include <set>
#include <string>
#include "ui/state_machine_graph.hpp"
#include "ui/graph_shown.hpp"
#include "ui/graph_view.hpp"
#include "ui/state_machine_shown.hpp"

MachineShown drawStateMachineGraph(
    const StateMachineBehaviorData &machine,
    const std::set<std::string> &litStates,
    MachineShown selected)
{
    return drawGraph("##machineGraph", graphOf(machine), litStates, selected);
}
