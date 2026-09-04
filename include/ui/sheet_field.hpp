#pragma once

#include <string_view>
#include "ui/inspector_edited.hpp"

struct SheetData;

inspector::Edited drawCustomField(std::string_view name, SheetData &value);
