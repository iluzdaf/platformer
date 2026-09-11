#pragma once

#include "ui/in_scope.hpp"
#include "ui/state_machine_shown.hpp"

inline const MachineShown *selectionInScope()
{
    return inScope<MachineShown>();
}
