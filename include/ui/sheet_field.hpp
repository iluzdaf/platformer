#pragma once

#include <string_view>
#include "ui/inspector_edited.hpp"

struct SheetData;

inspector::Edited drawSheetFields(SheetData &value);

inspector::Edited drawSquareSheetFields(SheetData &value);

inspector::Edited drawCustomField(std::string_view name, SheetData &value);
