#include "ui/state_machine_shown.hpp"
#include "ui/selection_in_scope.hpp"

namespace
{
    const MachineShown *showing = nullptr;
}

const MachineShown *selectionInScope()
{
    return showing;
}

ShowingSelection::ShowingSelection(const MachineShown &shown) : before(showing)
{
    showing = &shown;
}

ShowingSelection::~ShowingSelection()
{
    showing = before;
}
