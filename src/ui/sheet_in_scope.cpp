#include "ui/sheet_in_scope.hpp"

namespace
{
    const SheetInScope *showing = nullptr;
}

const SheetInScope *sheetInScope()
{
    return showing;
}

ShowingSheet::ShowingSheet(const SheetInScope &scope) : before(showing)
{
    showing = &scope;
}

ShowingSheet::~ShowingSheet()
{
    showing = before;
}
