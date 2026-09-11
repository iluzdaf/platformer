#pragma once

#include <string>
#include <string_view>
#include "ui/inspector_edited.hpp"

#include <vector>

inspector::Edited drawFileChooser(
    std::string_view label,
    std::string &path,
    std::string_view folder,
    std::string_view extension);

inspector::Edited drawFileChooser(
    std::string_view label,
    std::string &path,
    std::string_view folder,
    const std::vector<std::string> &offered);
