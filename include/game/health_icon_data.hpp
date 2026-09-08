#pragma once

#include "assets/sheet_data.hpp"

struct HealthIconData
{
    SheetData sheet;
    int full = 0;
    int spent = 1;
};
