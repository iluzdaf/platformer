#pragma once

#include <string>
#include <string_view>
#include "ui/inspector_edited.hpp"

inspector::Edited drawFileChooser(
    std::string_view label,
    std::string &path,
    std::string_view folder,
    std::string_view extension);
