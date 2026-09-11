#pragma once

#include "ui/state_machine_shown.hpp"

const MachineShown *selectionInScope();

class ShowingSelection
{
public:
    explicit ShowingSelection(const MachineShown &shown);
    ~ShowingSelection();

    ShowingSelection(const ShowingSelection &) = delete;
    ShowingSelection &operator=(const ShowingSelection &) = delete;
    ShowingSelection(ShowingSelection &&) = delete;
    ShowingSelection &operator=(ShowingSelection &&) = delete;

private:
    const MachineShown *before = nullptr;
};
