#pragma once

#include "assets/sheet_data.hpp"
#include "ui/in_scope.hpp"

class Texture2D;

struct SheetInScope
{
    const Texture2D *texture = nullptr;
    SheetData sheet;
};

inline const SheetInScope *sheetInScope()
{
    return inScope<SheetInScope>();
}
